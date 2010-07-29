/*
 * Optimize.c
 * 2010 Geoyeob Kim
 * As a part of the thesis implementation
 */
#include "PymPch.h"
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
    assert ( r==MSK_RES_OK );
    /* Directs the log stream to the
       'printstr' function. */
    r = MSK_linkfunctoenvstream(*env,MSK_STREAM_LOG,NULL,printstr);
    assert ( r==MSK_RES_OK );
    /* Manually configure the CPU type */
    r = MSK_putcpudefaults(*env, MSK_CPU_INTEL_CORE2, 128*1024, 6144*1024);
    assert ( r==MSK_RES_OK );
    /* Initialize the environment. */
    assert ( r==MSK_RES_OK );
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
                   double *opttime,
                   const pym_biped_eqconst_t *bod,
                   const pym_rb_statedep_t *sd,
                   const pym_config_t *pymCfg,
                   MSKenv_t *pEnv, cholmod_common *cc) {
    const int NUMCON = bod->bipMat->nrow;   /* Number of constraints.             */
    const int NUMVAR = bod->bipMat->ncol;   /* Number of variables.               */
    const int NUMANZ = cholmod_nnz(bod->bipMat, cc);   /* Number of non-zeros in A.          */
    MSKrescodee  r;
    int i,j;
    MSKboundkeye bkc[NUMCON];
    double       blc[NUMCON];
    double       buc[NUMCON];
    /* Equality constraints */
    FOR_0(i, NUMCON) bkc[i] = MSK_BK_FX;
    memcpy(blc, bod->bipEta, sizeof(double)*NUMCON);
    memcpy(buc, bod->bipEta, sizeof(double)*NUMCON);
    //FOR_0(i, NUMCON) printf("%lf  ", bod->bipEta[i]);

    /* Initialize all optimization variables as free variables. */
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
    int nplist[nb]; /* Stores # of CPs for each RB */
    FOR_0(i, nb) {
        nplist[i] = sd[i].nContacts_2;
    }
    PRINT_VECTOR_INT(nplist, nb);

    //int *Ari = bod->Ari;
    int *Aci = bod->Aci;

    double       c[NUMVAR];
    memset(c, 0, sizeof(double)*NUMVAR);

    /***********************/
    /* Cost function setup */
    /***********************/
    int tauOffset;
    for (i = 0, tauOffset = 0; i < nb; tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
        FOR_0(j, nplist[i]) {
            /*
             * TODO [TUNE] Minimize the contact normal force
             * Walk0, Nav0  - 0
             * Exer0        - ?
             */
            c[ tauOffset + sd[i].Aci[2] + 5*j + 4 ] = 0;
            /* Estimated position of z-coordinate of contact point
             * Default: 2e-1      */
            c[ tauOffset + sd[i].Aci[3] + 4*j + 2 ] = 0;
        }
        for (j= tauOffset + sd[i].Aci[5]; j < tauOffset + sd[i].Aci[6]; ++j) {
            /*
             * TODO [TUNE] Minimize the movement of candidate contact points
             *
             * Walk0        -  1
             * Nav0         -  5e-1
             * Exer0        -  1
             */
            c[j] = 0;
        }
        /*
         * TODO [TUNE] Reference following coefficient
         */
        c[ tauOffset + sd[i].Aci[8] ] = 1;
    }
    assert(Aci[2] - Aci[1] == pymCfg->nFiber);
    assert(Aci[3] - Aci[2] == pymCfg->nFiber);
    /* minimize aggregate tension of actuated muscle fiber */
    //c[ Aci[4] ] = 0e-8; /* ligament actuation */

    /*
     * Since actuation forces on actuated muscle fibers are
     * non-negative values we can either minimize the second-order
     * cone constraint variable c[Aci[5]] OR
     * the optimization variables c[Aci[2]+j] separately where
     * 'j' is the index of an actuated muscle fiber.
     */
    //c[ Aci[5] ] = 1e-6; /* actuated muscle fiber actuation */


    /***********************/
    /***********************/
#define SET_NONNEGATIVE(j) { bkx[j] = MSK_BK_LO; blx[j] = 0; bux[j] = MSK_INFINITY; }
#define SET_FIXED_ZERO(j)  { bkx[j] = MSK_BK_FX; blx[j] = 0; bux[j] = 0; }
#define SET_FIXED_ONE(j)   { bkx[j] = MSK_BK_FX; blx[j] = 1; bux[j] = 1; }
#define SET_LOWER_BOUND(j,lb)   { bkx[j] = MSK_BK_LO; blx[j] = lb; bux[j] = MSK_INFINITY; }
#define SET_RANGE(j,lb,ub) { bkx[j] = MSK_BK_RA; blx[j] = lb; bux[j] = ub; }
    const int nd = 6;
    for (i = 0, tauOffset = 0; i < nb; tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
        SET_NONNEGATIVE( tauOffset + sd[i].Aci[0] + 2 ); /* chi_2_z */
        FOR_0(j, nplist[i]) SET_NONNEGATIVE( tauOffset + sd[i].Aci[1] + nd*j + 2 ); /* f_c_z */
        FOR_0(j, nplist[i]) SET_FIXED_ZERO ( tauOffset + sd[i].Aci[2] + 5*j + 3 ); /* c_c_w */
        FOR_0(j, nplist[i]) {
            /*
             * TODO [TUNE] Contact normal force constraint tuning
             * Walk0   :  Nonnegative
             * Nav0    :  0~200
             * Exer0   :  0~200 (failed)
             */
            //SET_NONNEGATIVE( tauOffset + sd[i].Aci[2] + 5*j + 4 ); /* c_c_n */
            //SET_RANGE( tauOffset + sd[i].Aci[2] + 5*j + 4 , 0, 200); /* c_c_n */
            //SET_RANGE( tauOffset + sd[i].Aci[2] + 5*j + 4 , 0, 140); /* c_c_n */
        	SET_RANGE( tauOffset + sd[i].Aci[2] + 5*j + 4 , 0, 115); /* c_c_n */
        }
        FOR_0(j, nplist[i]) {
            //SET_NONNEGATIVE( tauOffset + sd[i].Aci[3] + 4*j + 2 ); /* p_c_2_z : next step CP z-pos */

            /* DEBUG */
            const double ctY = sd[i].contactsFix_2[j][1];
            const double theta = pymCfg->slant;
            const double z = -ctY*tan(theta);
            SET_LOWER_BOUND( tauOffset + sd[i].Aci[3] + 4*j + 2, z ); /* p_c_2_z : next step CP z-pos */
        }
        /*
         * TODO [TUNE] Constraints for fixing contact points
         * p_c_2_z
         */
        //FOR_0(j, nplist[i]) SET_FIXED_ZERO ( tauOffset + sd[i].Aci[3] + 4*j + 2 );
        FOR_0(j, nplist[i]) SET_FIXED_ONE  ( tauOffset + sd[i].Aci[3] + 4*j + 3 ); /* p_c_2_w */
        for(j=tauOffset + sd[i].Aci[5]; j<tauOffset + sd[i].Aci[6]; ++j) SET_NONNEGATIVE( j ); /* eps_fric */
        for(j=tauOffset + sd[i].Aci[6]; j<tauOffset + sd[i].Aci[7]; ++j) SET_NONNEGATIVE( j ); /* muf_cz */
        for(j=tauOffset + sd[i].Aci[8]; j<tauOffset + sd[i].Aci[9]; ++j) SET_NONNEGATIVE( j ); /* eps_delta */

        for(j=Aci[6]; j<Aci[7]; j+=4) SET_FIXED_ZERO( j+3 ); /* d_A homogeneous part to 0 (vector) */
    }

    FOR_0(j, nf) {
        const pym_muscle_type_e mt = pymCfg->fiber[j].b.mType;
        const char *fibName = pymCfg->fiber[j].b.name;

        /* Tension range constraint */
        i = Aci[1]+j;
//        bkx[i] = MSK_BK_RA;
//        blx[i] = -1000;
//        bux[i] = +1000;
        /* Actuation force range constraint */
        i = Aci[2]+j;
        if (mt == PMT_ACTUATED_MUSCLE) {
            bkx[i] = MSK_BK_RA;
            blx[i] = 0;
            bux[i] =  200*9.81;
        }
        else if (mt == PMT_LIGAMENT) {
//            if (strncmp(fibName, "ankleLiga", 9) == 0) {
//                bkx[i] = MSK_BK_RA;
//                blx[i] = -800*9.81;
//                bux[i] =  800*9.81;
//            } else if (strncmp(fibName, "hipLiga", 7) == 0) {
//                bkx[i] = MSK_BK_RA;
//                blx[i] = -800*9.81;
//                bux[i] =  800*9.81;
//            } else {
//                bkx[i] = MSK_BK_RA;
//                blx[i] = -600*9.81;
//                bux[i] =  600*9.81;
//            }
        }
        else
            abort();

        /* Rest length range constraint */
        i = Aci[3]+j;
        bkx[i] = MSK_BK_RA;
        blx[i] = pymCfg->fiber[j].b.xrest_lower;
        bux[i] = pymCfg->fiber[j].b.xrest_upper;
    }

    MSKtask_t   task;

    /* Create the optimization task. */
    r = MSK_maketask(*pEnv,NUMCON,NUMVAR,&task); assert(r == MSK_RES_OK);
    assert(r == MSK_RES_OK);

    /* If some error or infeasibility condition detected from
     * MOSEK, the user can inspect this log file.     */
    FILE *mosekLogFile = 0;
    /* TODO MOSEK optimization logging */
//    mosekLogFile = fopen("/tmp/pymoptimize_log", "w");
//    if (!mosekLogFile) {
//        printf("Opening MOSEK output log file failed.\n");
//        return FLT_MAX;
//    }
//    r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, mosekLogFile, printstr);

    assert(r == MSK_RES_OK);
    /* On my(gykim) computer, the other values except 1 for the
     * number of threads does not work. (segfault or hang)
     * Other PCs seemed to fine, though.
     */
    r = MSK_putintparam (task, MSK_IPAR_INTPNT_NUM_THREADS, 1);
    assert(r == MSK_RES_OK);
    //MSK_putintparam (task , MSK_IPAR_OPTIMIZER , MSK_OPTIMIZER_FREE_SIMPLEX);

    //r = MSK_putintparam (task , MSK_IPAR_CPU_TYPE , MSK_CPU_INTEL_CORE2);
    //assert(r == MSK_RES_OK);
//    r = MSK_putintparam (task , MSK_IPAR_CACHE_SIZE_L1, 128*1024);
//    assert(r == MSK_RES_OK);
//    r = MSK_putintparam (task , MSK_IPAR_CACHE_SIZE_L2, 6144*1024);
//    assert(r == MSK_RES_OK);
    r = MSK_putintparam (task , MSK_IPAR_DATA_CHECK, MSK_OFF);
    assert(r == MSK_RES_OK);
//    r = MSK_putintparam (task , MSK_IPAR_INTPNT_MAX_ITERATIONS, 10);
//    assert(r == MSK_RES_OK);


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
     * Input the cones
     */
    for (i = 0, tauOffset = 0; i < nb; tauOffset += sd[i].Aci[ sd[i].Asubcols ], i++) {
        /* Reference trajectory cone constraints, i.e.,
         * epsilon_Delta >= || Delta_chi_{i,ref} || (6-DOF)      */
        AppendConeRange(task,
                        tauOffset + sd[i].Aci[8],
                        tauOffset + sd[i].Aci[7],
                        tauOffset + sd[i].Aci[8]);

        FOR_0(j, nplist[i]) {
            /*
             * TODO [TUNE] CP movement minimization
             *
             * Walk0, Nav0 - including z-axis movement (0~3)
             * Exer0       - excluding z-axis movement (0~2)
             */
            AppendConeRange(task,
                            tauOffset + sd[i].Aci[5] + j,
                            tauOffset + sd[i].Aci[4]+4*j+0,
                            tauOffset + sd[i].Aci[4]+4*j+3);

            /* Friction cone constraints */
            AppendConeRange(task,
                            tauOffset + sd[i].Aci[6]+j,            // mu*c_n
                            tauOffset + sd[i].Aci[2]+5*j+0,        // c_tx ~ c_tz
                            tauOffset + sd[i].Aci[2]+5*j+3);
        }
    }
    FOR_0(j, pymCfg->nJoint) {
        /* Anchored joint dislocation constraints */
        AppendConeRange(task,
                        Aci[7] + j,                      // epsilon_d
                        Aci[6] + 4*j,                    // dAx ~ dAz
                        Aci[6] + 4*j + 3);
    }

    /*
     * TODO [TUNE] Cone constraints for minimizing tension/actuation forces
     */
    int csubLigaAct[1+nf];
    int nCsubLigaAct = 1;
    csubLigaAct[0] = Aci[4];
    int csubActAct[1+nf];
    int nCsubActAct = 1;
    csubActAct[0] = Aci[5];
    FOR_0(j, nf) {
        const pym_muscle_type_e mt = pymCfg->fiber[j].b.mType;
        if (mt == PMT_ACTUATED_MUSCLE) {
            csubActAct[nCsubActAct] = Aci[1] + j;
            ++nCsubActAct;
        }
        else if (mt == PMT_LIGAMENT) {
            csubLigaAct[nCsubLigaAct] = Aci[1] + j;
            ++nCsubLigaAct;
        }
        else
            abort();
    }
//    r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, nCsubLigaAct, csubLigaAct);
//    assert(r == MSK_RES_OK);
    r = MSK_appendcone(task, MSK_CT_QUAD, 0.0, nCsubActAct, csubActAct);
    assert(r == MSK_RES_OK);


    double cost = FLT_MAX;
    MSKrescodee trmcode;
    /* Run optimizer */
    //r = MSK_optimize(task);
    r = MSK_optimizetrm(task, &trmcode);

    if (r == MSK_RES_TRM_MAX_ITERATIONS)
    	printf("Error - MSK_RES_TRM_MAX_ITERATIONS returned.\n");
    else if (r == MSK_RES_TRM_STALL)
    	printf("Error - MSK_RES_TRM_STALL returned.\n");

    if (opttime)
        MSK_getdouinf ( task , MSK_DINF_OPTIMIZER_TIME , opttime );

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
        printf("Error - An error occurred while optimizing.\n");
        printf("        Refer to /tmp/pymoptimize_log and pymuscle_c.opf\n");
        printf("        for further investigation.\n");
        MSK_writedata(task, "pymuscle_c.opf");

        /* In case of an error print error code and description. */
        char symname[MSK_MAX_STR_LEN];
        char desc[MSK_MAX_STR_LEN];
        MSK_getcodedesc (r, symname, desc);
        printf("Error - %s: %s\n", symname, desc);
        cost = FLT_MAX;
    }

    if (mosekLogFile)
        fclose(mosekLogFile);
    /* Delete the task and the associated data. */
    MSK_deletetask(&task);
    return cost;
}

