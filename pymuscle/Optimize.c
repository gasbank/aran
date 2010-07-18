/*
 * Optimize.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 */
#include <string.h>
#include <float.h>
#include <assert.h>
#include <cholmod.h>
#include <mosek.h> /* Include the MOSEK definition file. */
#include "PymStruct.h"
#include "Config.h"
#include "Biped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "StateDependents.h"
#include "PymuscleConfig.h"
#include "DebugPrintDef.h"
#include "MathUtil.h"
#include "Optimize.h"

static void MSKAPI printstr(void *handle,
                            char str[])
{
    FILE *out = (FILE *)handle;
    fprintf(out, "%s", str);
} /* printstr */

void PymInitializeMosek(MSKenv_t *env) {
    MSKrescodee  r;
    /* Create the mosek environment. */
    r = MSK_makeenv(env,NULL,NULL,NULL,NULL);
    /* Check if return code is ok. */
    if ( r==MSK_RES_OK )
    {
        /* Directs the log stream to the
           'printstr' function. */
        MSK_linkfunctoenvstream(*env,MSK_STREAM_LOG,NULL,printstr);
    }
    /* Initialize the environment. */
    if ( r==MSK_RES_OK )
        r = MSK_initenv(*env);
}

void PymCleanupMosek(MSKenv_t *env) {
    /* Delete the environment and the associated data. */
    MSK_deleteenv(env);
}

void AppendConeRange(MSKtask_t task, int x, int r1, int r2) {
    /*
     * Append a second-order cone constraint which has
     * the form of x >= norm([ r1, r1+1, ... , r2-1 ])
     */
	assert(r1>=0 && r2>=0 && r2>=r1 && x>=0);
	int csub[1 + r2 - r1];
	csub[0] = x;
	int j;
	for (j=0; j<r2 - r1; ++j) csub[j+1] = r1 + j;
	MSKrescodee r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, 1+r2-r1, csub);
	assert(r == MSK_RES_OK);
}

double PymOptimize(double *xx, /* Preallocated solution vector space (size = bod->bipMat->ncol) */
                   MSKsolstae *_solsta, /* MOSEK solution status */
                   const pym_biped_eqconst_t *bod,
                   const pym_rb_statedep_t *sd,
                   const pym_config_t *pymCfg,
                   MSKenv_t *pEnv, cholmod_common *cc) {
    const int NUMCON = bod->bipMat->nrow;   /* Number of constraints.             */
    const int NUMVAR = bod->bipMat->ncol;   /* Number of variables.               */
    const int NUMANZ = cholmod_nnz(bod->bipMat, cc);   /* Number of non-zeros in A.          */
    MSKrescodee  r;
    int i,j,k;
    MSKboundkeye bkc[NUMCON];
    double       blc[NUMCON];
    double       buc[NUMCON];
    FOR_0(i, NUMCON) bkc[i] = MSK_BK_FX;
    /* TODO: can be optimized */
    memcpy(blc, bod->bipEta, sizeof(double)*NUMCON);
    memcpy(buc, bod->bipEta, sizeof(double)*NUMCON);
    //FOR_0(i, NUMCON) printf("%lf  ", bod->bipEta[i]);

    MSKboundkeye bkx[NUMVAR];
    double       blx[NUMVAR];
    double       bux[NUMVAR];
    FOR_0(i, NUMVAR) {
        bkx[i] = MSK_BK_FR;
        blx[i] = -MSK_INFINITY;
        bux[i] = +MSK_INFINITY;
    }



    const int nb = pymCfg->nBody;
    const int nf = pymCfg->nFiber;
    int nplist[nb];
    FOR_0(i, nb) {
        nplist[i] = sd[i].nContacts_2;
    }
    PRINT_VECTOR_INT(nplist, nb);

    //int *Ari = bod->Ari;
    int *Aci = bod->Aci;

    double       c[NUMVAR];
    memset(c, 0, sizeof(double)*NUMVAR);

    double bodyRefWeight[] = {
            /* trunk  */ 1e4,
            /* thighL */ 1e3,
            /* thighR */ 1e3,
            /* calfL  */ 1e3,
            /* calfR  */ 1e3,
            /* soleL  */ 1e3,
            /* soleR  */ 1e3,
            /* toeL   */ 1e3,
            /* toeR   */ 1e3,
    };
    assert(sizeof(bodyRefWeight) == sizeof(double)*nb);

    FOR_0(i, nb) {
        FOR_0(j, nplist[i]) {
            /* 0.01 ~ 0.02 */
            c[ bod->Aci[i] + sd[i].Aci[1] + 6*j + 2 ] = 1e-2; /* minimize the contact normal force */
        }
        FOR_0(j, nplist[i]) {
            c[ bod->Aci[i] + sd[i].Aci[3] + 4*j + 2 ] = 0; /* Estimated position of z-coordinate of contact point */
        }
        for (j= bod->Aci[i] + sd[i].Aci[5]; j < bod->Aci[i]+sd[i].Aci[6]; ++j)
            c[j] = 1e3; /* minimize the movement of candidate contact points */


        /** DEBUG **/
        //c[ bod->Aci[i] + sd[i].Aci[8] ] = bodyRefWeight[i]; /* minimize the deviation with reference trajectories */
        c[ bod->Aci[i] + sd[i].Aci[8] ] = 1e4;

        for (j= bod->Aci[nb+3]; j < bod->Aci[nb+4]; ++j)
            c[j] = 1e-7; /* minimize aggregate tension */
        for (j= bod->Aci[nb+4]; j < bod->Aci[nb+5]; ++j)
            c[j] = 1e-7; /* minimize aggregate actuation force */
    }
    const int nd = 6;
    #define SET_NONNEGATIVE(j) { bkx[j] = MSK_BK_LO; blx[j] = 0; bux[j] = MSK_INFINITY; }
    #define SET_FIXED_ZERO(j)  { bkx[j] = MSK_BK_FX; blx[j] = 0; bux[j] = 0; }
    #define SET_FIXED_ONE(j)   { bkx[j] = MSK_BK_FX; blx[j] = 1; bux[j] = 1; }
    FOR_0(i, nb) {
        SET_NONNEGATIVE( Aci[i] + sd[i].Aci[0] + 2 ); /* chi_2_z */
        FOR_0(j, nplist[i]) SET_NONNEGATIVE( Aci[i] + sd[i].Aci[1] + nd*j + 2 ); /* f_c_z */
        FOR_0(j, nplist[i]) SET_FIXED_ZERO ( Aci[i] + sd[i].Aci[2] + 5*j + 2 ); /* c_c_z (TODO: Assumes flat ground) */
        FOR_0(j, nplist[i]) SET_FIXED_ZERO ( Aci[i] + sd[i].Aci[2] + 5*j + 3 ); /* c_c_w */
        FOR_0(j, nplist[i]) SET_NONNEGATIVE( Aci[i] + sd[i].Aci[2] + 5*j + 4 ); /* c_c_n */
        FOR_0(j, nplist[i]) SET_NONNEGATIVE( Aci[i] + sd[i].Aci[3] + 4*j + 2 ); /* p_c_2_z */
        //FOR_0(j, nplist[i]) SET_FIXED_ZERO ( Aci[i] + sd[i].Aci[3] + 4*j + 2 ); /* p_c_2_z (TODO: How to allow contact break?) */
        FOR_0(j, nplist[i]) SET_FIXED_ONE  ( Aci[i] + sd[i].Aci[3] + 4*j + 3 ); /* p_c_2_w */
        for(j=Aci[i] + sd[i].Aci[5]; j<Aci[i] + sd[i].Aci[6]; ++j) SET_NONNEGATIVE( j ); /* eps_fric */
        for(j=Aci[i] + sd[i].Aci[6]; j<Aci[i] + sd[i].Aci[7]; ++j) SET_NONNEGATIVE( j ); /* muf_cz */
        for(j=Aci[i] + sd[i].Aci[8]; j<Aci[i] + sd[i].Aci[9]; ++j) SET_NONNEGATIVE( j ); /* eps_delta */
    }

    FOR_0(j, nf) {
        /* Tension range constraint */
        i = Aci[nb+0]+j;
//        bkx[i] = MSK_BK_RA;
//        blx[i] = -1000;
//        bux[i] = +1000;
        /* Actuation force range constraint */
        i = Aci[nb+1]+j;
        bkx[i] = MSK_BK_RA;
        blx[i] = -400*9.81;
        bux[i] =  400*9.81;
        /* Rest length range constraint */
        i = Aci[nb+2]+j;
        bkx[i] = MSK_BK_RA;
        blx[i] = pymCfg->fiber[j].b.xrest_lower;
        bux[i] = pymCfg->fiber[j].b.xrest_upper;
    }

    /* TODO trunk next x position */
    i = Aci[0]+sd[0].Aci[0]+0;
//    bkx[i] = MSK_BK_RA;
//    blx[i] = 2.8;
//    bux[i] = 3.0;
    /* TODO trunk next z position */
    i = Aci[0]+sd[0].Aci[0]+2;
//    bkx[i] = MSK_BK_RA;
//    blx[i] = 1.8;
//    bux[i] = 2.0;


    /* TODO: ### DEBUG PURPOSE (REMOVE IT!) ### */
    FOR_0(k, nb) {
        /* Useful when 'stay still' mode.
//        FOR_0(j, 3) {
//            i = Aci[k]+3+j; /* angular rx,ry,rz-axis position of body i */
//            SET_FIXED_ZERO(i);
//        }
//        if (strncmp(pymCfg->body[k].b.name, "toe", 3) != 0) {
//            i = Aci[k]+1;   /* linear y-axis */
//            SET_FIXED_ZERO(i);
//        }
//        if (strcmp(pymCfg->body[k].b.name, "trunk") == 0) {
//            i = Aci[k]+0;   /* linear x-axis */
//            SET_FIXED_ZERO(i);
//        }

        /* Linear position reference as constraint (?) */
//        FOR_0(j, 3) {
//            i = Aci[k] + j;
//            bkx[i] = MSK_BK_FX;
//            blx[i] = pymCfg->body[k].b.chi_ref[j];
//            bux[i] = blx[i];
//        }
    }

    MSKtask_t   task;

    /* Create the optimization task. */
    r = MSK_maketask(*pEnv,NUMCON,NUMVAR,&task); assert(r == MSK_RES_OK);
    FILE *mosekLogFile = fopen("/tmp/pymoptimize_log", "w");
    if (!mosekLogFile) {
        printf("Opening MOSEK output log file failed.\n");
        return FLT_MAX;
    }
    MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, mosekLogFile, printstr);

    /* Give MOSEK an estimate of the size of the input data.
    This is done to increase the speed of inputting data.
    However, it is optional. */
    r = MSK_putmaxnumvar(task,NUMVAR);  assert(r == MSK_RES_OK);
    r = MSK_putmaxnumcon(task,NUMCON);  assert(r == MSK_RES_OK);
    r = MSK_putmaxnumanz(task,NUMANZ);  assert(r == MSK_RES_OK);

    /* Append 'NUMCON' empty constraints.
    The constraints will initially have no bounds. */
    r = MSK_append(task,MSK_ACC_CON,NUMCON);  assert(r == MSK_RES_OK);

    /* Append 'NUMVAR' variables.
    The variables will initially be fixed at zero (x=0). */
    r = MSK_append(task,MSK_ACC_VAR,NUMVAR);   assert(r == MSK_RES_OK);

    int __annz = 0;
    /* Optionally add a constant term to the objective. */
    r = MSK_putcfix(task,0.0);  assert(r == MSK_RES_OK);
    for(j=0; j<NUMVAR && r == MSK_RES_OK; ++j)
    {
        /* Set the linear term c_j in the objective.*/
        r = MSK_putcj(task,j,c[j]);
        assert(r == MSK_RES_OK);

        /* Set the bounds on variable j.
        blx[j] <= x_j <= bux[j] */
        r = MSK_putbound(task,
                       MSK_ACC_VAR, /* Put bounds on variables.*/
                       j,           /* Index of variable.*/
                       bkx[j],      /* Bound key.*/
                       blx[j],      /* Numerical value of lower bound.*/
                       bux[j]);     /* Numerical value of upper bound.*/
        assert(r == MSK_RES_OK);

        /* Input column j of A */
        if(r == MSK_RES_OK) {
            int colstart = ((int *)(bod->bipMat->p))[j  ];
            int colend   = ((int *)(bod->bipMat->p))[j+1];
            __annz += colend-colstart;
            r = MSK_putavec(task,
                      MSK_ACC_VAR,       /* Input columns of A.*/
                      j,                 /* Variable (column) index.*/
                      colend-colstart, /* Number of non-zeros in column j.*/
                      (int *)(bod->bipMat->i) + colstart,     /* Pointer to row indexes of column j.*/
                      (double *)(bod->bipMat->x) + colstart);    /* Pointer to Values of column j.*/
        }
    }
    assert(__annz == cholmod_nnz(bod->bipMat, cc));
    //printf("__annz = %d\n", __annz);

    /* Set the bounds on constraints.
    for i=1, ...,NUMCON : blc[i] <= constraint i <= buc[i] */
    for(i=0; i<NUMCON && r==MSK_RES_OK; ++i) {
        r = MSK_putbound(task,
                    MSK_ACC_CON, /* Put bounds on constraints.*/
                    i,           /* Index of constraint.*/
                    bkc[i],      /* Bound key.*/
                    blc[i],      /* Numerical value of lower bound.*/
                    buc[i]);     /* Numerical value of upper bound.*/
        assert(r == MSK_RES_OK );
    }



    /*
    #
    # Input the cones
    #
    */
    FOR_0(i, nb) {
        //# epsilon_Delta >= || Delta_chi_{i,ref} || (6-DOF)
        AppendConeRange(task,
                        Aci[i] + sd[i].Aci[8],
                        Aci[i] + sd[i].Aci[7],
                        Aci[i] + sd[i].Aci[8]);

        FOR_0(j, nplist[i]) {
            //# CP movement minimization
            AppendConeRange(task,
                            Aci[i] + sd[i].Aci[5] + j,
                            Aci[i] + sd[i].Aci[4]+4*j+0,
                            Aci[i] + sd[i].Aci[4]+4*j+3);

            //# Friction cone constraints
            AppendConeRange(task,
                            Aci[i] + sd[i].Aci[6]+j,
                            Aci[i] + sd[i].Aci[1]+6*j+0,
                            Aci[i] + sd[i].Aci[1]+6*j+2);
        }
    }
    //# Minimal tension force constraints
    AppendConeRange(task, Aci[nb+3], Aci[nb+0], Aci[nb+1]);
    //# Minimal actuation force constraints
    AppendConeRange(task, Aci[nb+4], Aci[nb+1], Aci[nb+2]);
    assert(r==MSK_RES_OK);

    double cost = FLT_MAX;
    MSKrescodee trmcode;
    MSK_writedata(task,"pymuscle_c.opf");
    /* Run optimizer */
    r = MSK_optimizetrm(task,&trmcode);

    /* Print a summary containing information
       about the solution for debugging purposes*/
    MSK_solutionsummary (task, MSK_STREAM_LOG);

    if ( r==MSK_RES_OK )
    {
        MSKsolstae solsta;

        MSK_getsolutionstatus (task,
                             MSK_SOL_ITR,
                             NULL,
                             &solsta);
        *_solsta = solsta;

        switch(solsta)
        {
        case MSK_SOL_STA_UNKNOWN:
            //printf("   ***   The status of the solution could not be determined.   ***\n");
        case MSK_SOL_STA_OPTIMAL:
        case MSK_SOL_STA_NEAR_OPTIMAL:
            MSK_getsolutionslice(task,
                                   MSK_SOL_ITR,    /* Request the interior solution. */
                                   MSK_SOL_ITEM_XX,/* Which part of solution.     */
                                   0,              /* Index of first variable.    */
                                   NUMVAR,         /* Index of last variable+1.   */
                                   xx);
            cost = Dot(NUMVAR, xx, c);
            break;
        case MSK_SOL_STA_DUAL_INFEAS_CER:
        case MSK_SOL_STA_PRIM_INFEAS_CER:
        case MSK_SOL_STA_NEAR_DUAL_INFEAS_CER:
        case MSK_SOL_STA_NEAR_PRIM_INFEAS_CER:
            printf("Primal or dual infeasibility certificate found.\n");
            cost = FLT_MAX;
            break;

        default:
            printf("Other solution status.");
            cost = FLT_MAX;
            break;
        }
    }
    else
    {
        printf("Error while optimizing.\n");

        /* In case of an error print error code and description. */
        char symname[MSK_MAX_STR_LEN];
        char desc[MSK_MAX_STR_LEN];

        printf("An error occurred while optimizing.\n");
        MSK_getcodedesc (r,
                         symname,
                         desc);
        printf("Error %s - '%s'\n",symname,desc);
        cost = FLT_MAX;
    }

    fclose(mosekLogFile);
    /* Delete the task and the associated data. */
    MSK_deletetask(&task);
    return cost;
}

