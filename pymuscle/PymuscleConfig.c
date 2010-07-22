#include "PymPch.h"
#include "PymStruct.h"
#include "MathUtil.h"
#include "Biped.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "StateDependents.h"
#include "SymbolicTensor.h"
#include "Config.h"
#include "CholmodMacro.h"
#include "dRdv_real.h"
#include "PymuscleConfig.h"
#include "PymPodUtil.h"

void PrintRigidBody(const pym_rb_t *rb) {
    int i; for (i=0; i<18; ++i) printf("%2d : %20lf\n", i, rb->a[i]);
}

int PymDestoryConfig(pym_config_t *pymCfg) {
    free(pymCfg->body) ;
    free(pymCfg->fiber) ;
    pymCfg->body = 0 ;
    pymCfg->fiber = 0 ;
    pymCfg->nBody = 0 ;
    pymCfg->nFiber = 0 ;
    return 0;
}

int PymConstructConfig(const char *fnConf, pym_config_t *pymCfg) {
    #define csgfe(a,b) config_setting_get_float_elem(a,b)
    #define BODY(j,ai) body[j].a[ai]
    #define FIBER(j,ai) fiber[j].a[ai]
    int j, k;
    config_t conf;
    config_init(&conf);
    config_set_auto_convert(&conf, 1);
    printf("Opening %s...\n", fnConf);
    int confret = config_read_file(&conf, fnConf);
    if (confret != CONFIG_TRUE)
    {
        const char *errText = config_error_text(&conf);
        const int errLine = config_error_line(&conf);
        /* TODO: libconfig version mismatch */
        //const char *errFile = config_error_file(&conf);
        //printf("Configuration file %s (line %d) %s!\n", errFile, errLine, errText);
        printf("Configuration file ?? (line %d) %s!\n", errLine, errText);
        config_destroy(&conf);
        return (-10);
    }

    const char *ver;
    config_lookup_string(&conf, "version", &ver);
    printf("  Config file version: %s\n", ver);

    confret = config_lookup_float(&conf, "h", &pymCfg->h);
    assert(confret == CONFIG_TRUE);
    const double h = pymCfg->h;
    confret = config_lookup_float(&conf, "mu", &pymCfg->mu);

    /* TODO: Slant */
    pymCfg->slant = 0.0;

    assert(confret == CONFIG_TRUE);
    /*
     * Parse body configurations
     */
    config_setting_t *bodyConf = config_lookup(&conf, "body");
    assert(bodyConf);
    const unsigned int nBody = config_setting_length(bodyConf);
    pym_rb_t body[nBody];
	double extForce[nBody][6];
	char bodyNames[nBody][128];
	memset(extForce, 0, sizeof(double)*nBody*6); /* NOTE: No external force for now. */
    for (j = 0; j < nBody; ++j)
    {
        pym_rb_named_t *bjb = &body[j].b;
        bjb->nAnchor = 0;
        config_setting_t *bConf = config_setting_get_elem(bodyConf, j);
        const char *bName;
        config_setting_lookup_string(bConf, "name", &bName);
        strncpy(bodyNames[j], bName, 128);
        strncpy(bjb->name, bName, 128);
        //printf("    Body %3d : %s\n", j, bName);
        const char *rotParamStr;
        config_setting_lookup_string(bConf, "rotParam", &rotParamStr);
        pym_rot_param_t rp;
        int nrp = 0; /* Numer of Rotation Parameters */
        if (strcmp(rotParamStr, "QUAT_WFIRST") == 0)    { rp = RP_QUAT_WFIRST; nrp = 4; }
        else if (strcmp(rotParamStr, "EXP") == 0)       { rp = RP_EXP;         nrp = 3; }
        else if (strcmp(rotParamStr, "EULER_XYZ") == 0) { rp = RP_EULER_XYZ;   nrp = 3; }
        else if (strcmp(rotParamStr, "EULER_ZXZ") == 0) { rp = RP_EULER_ZXZ;   nrp = 3; }
        else assert(0);
        int i;

        bjb->rotParam = rp;
        config_setting_t *p    = config_setting_get_member(bConf, "p");    assert(p);    for (i=0;i<3;  ++i) bjb->p[i]       = csgfe(p, i);
        config_setting_t *q    = config_setting_get_member(bConf, "q");    assert(q);    for (i=0;i<nrp;++i) bjb->q[i]       = csgfe(q, i);
        config_setting_t *pd   = config_setting_get_member(bConf, "pd");   assert(pd);   for (i=0;i<3;  ++i) bjb->pd[i]      = csgfe(pd, i);
        config_setting_t *qd   = config_setting_get_member(bConf, "qd");   assert(qd);   for (i=0;i<nrp;++i) bjb->qd[i]      = csgfe(qd, i);
        config_setting_t *size = config_setting_get_member(bConf, "size"); assert(size); for (i=0;i<3;  ++i) bjb->boxSize[i] = csgfe(size, i);
        config_setting_t *mass = config_setting_get_member(bConf, "mass"); assert(mass); bjb->m = config_setting_get_float(mass);
        BoxInertiaTensorFromMassAndSize(bjb->I, bjb->m, bjb->boxSize[0], bjb->boxSize[1], bjb->boxSize[2]);
        const double volume = bjb->boxSize[0] * bjb->boxSize[1] * bjb->boxSize[2];
        SymbolicTensor(bjb->Ixyzw, bjb->boxSize[0], bjb->boxSize[1], bjb->boxSize[2], bjb->m/volume);
        BoxCorners(bjb->corners, bjb->boxSize[0], bjb->boxSize[1], bjb->boxSize[2]);
        config_setting_t *cExtForce = config_setting_get_member(bConf, "extForce");
        if (cExtForce)
        {
            extForce[j][0] = csgfe(cExtForce, 0);
            extForce[j][1] = csgfe(cExtForce, 1);
            extForce[j][2] = csgfe(cExtForce, 2);
        }
        config_setting_t *cExtTorque = config_setting_get_member(bConf, "extTorque");
        if (cExtTorque)
        {
            extForce[j][3] = csgfe(cExtTorque, 0);
            extForce[j][4] = csgfe(cExtTorque, 1);
            extForce[j][5] = csgfe(cExtTorque, 2);
        }
        /*
         * Add gravitational force as external force if grav field is true.
         */
        config_setting_t *grav = config_setting_get_member(bConf, "grav");
        if (grav && config_setting_get_bool(grav))
        {
            extForce[j][2] += bjb->m * (-9.81);
        }

        bjb->nFiber = 0; /* will be set after loading muscle fiber sections of configuration file */
        FOR_0(k, 3) {
            bjb->p0[k] = bjb->p[k] - h*bjb->pd[k];
            assert(bjb->p[k] == bjb->p0[k]);
        }
        FOR_0(k, 4) {
            bjb->q0[k] = bjb->q[k] - h*bjb->qd[k];
            assert(bjb->q[k] == bjb->q0[k]);
        }
    }
    /*
     * Parse muscle fiber configurations
     */
    config_setting_t *muscleConf = config_lookup(&conf, "muscle");
    assert(muscleConf);
	const unsigned int nMuscle = config_setting_length(muscleConf);
	pym_mf_t fiber[nMuscle];
	unsigned int musclePair[nMuscle][2];
	char muscleName[nMuscle][128];
    for (j = 0; j < nMuscle; ++j)
    {
        pym_mf_named_t  *mfn = &fiber[j].b;
        config_setting_t *mConf = config_setting_get_elem(muscleConf, j);
        const char *mName;
        config_setting_lookup_string(mConf, "name", &mName);
        strncpy(muscleName[j], mName, 128);
        strncpy(mfn->name, mName, 128);
        //printf("    Fiber %3d : %s\n", j, mName);
        config_setting_t *KSE = config_setting_get_member(mConf, "KSE"); assert(KSE);
        FIBER(j,0) = config_setting_get_float(KSE);
        config_setting_t *KPE = config_setting_get_member(mConf, "KPE"); assert(KPE);
        FIBER(j,1) = config_setting_get_float(KPE);
        config_setting_t *b = config_setting_get_member(mConf, "b"); assert(b);
        FIBER(j,2) = config_setting_get_float(b);
        config_setting_t *xrest = config_setting_get_member(mConf, "xrest"); assert(xrest);
        FIBER(j,3) = config_setting_get_float(xrest);
        config_setting_t *T = config_setting_get_member(mConf, "T"); assert(T);
        FIBER(j,4) = config_setting_get_float(T);
        config_setting_t *A = config_setting_get_member(mConf, "A"); assert(A);
        FIBER(j,5) = config_setting_get_float(A);

        config_setting_t *originPos = config_setting_get_member(mConf, "originPos"); assert(originPos);
        FIBER(j,6) = csgfe(originPos, 0);
        FIBER(j,7) = csgfe(originPos, 1);
        FIBER(j,8) = csgfe(originPos, 2);
        config_setting_t *insertionPos = config_setting_get_member(mConf, "insertionPos"); assert(insertionPos);
        FIBER(j,9) = csgfe(insertionPos, 0);
        FIBER(j,10) = csgfe(insertionPos, 1);
        FIBER(j,11) = csgfe(insertionPos, 2);


        const char *orgBodyName, *insBodyName;
        config_setting_lookup_string(mConf, "origin", &orgBodyName);
        config_setting_lookup_string(mConf, "insertion", &insBodyName);
        int boIdx = FindBodyIndex(nBody, bodyNames, orgBodyName);
        int biIdx = FindBodyIndex(nBody, bodyNames, insBodyName);
        assert(boIdx >= 0 && biIdx >= 0);
        assert(boIdx != biIdx);
        musclePair[j][0] = boIdx;
        musclePair[j][1] = biIdx;
        mfn->org = boIdx;
        mfn->ins = biIdx;
        /* TODO: xrest upper and lower bounds */
        mfn->xrest_lower = mfn->xrest*0.5;
        mfn->xrest_upper = mfn->xrest*2.0;
//        mfn->xrest_lower = -100;
//        mfn->xrest_upper = +100;
        pym_rb_named_t *bjbOrg = &body[ boIdx ].b;
        pym_rb_named_t *bjbIns = &body[ biIdx ].b;
        bjbOrg->fiber[ bjbOrg->nFiber ] = j; ++bjbOrg->nFiber;
        bjbIns->fiber[ bjbIns->nFiber ] = j; ++bjbIns->nFiber;

    }

    printf("Info - # of RB parsed = %d\n", nBody);
    printf("Info - # of MF parsed = %d\n", nMuscle);

    pymCfg->nJoint = 0;
    int simFrame; /* simulation length */
    confret = config_lookup_int(&conf, "simFrame", &simFrame);
    assert(confret == CONFIG_TRUE);
    pymCfg->nSimFrame = simFrame;
    int plotSamplingRate; /* Write 1 sample per 'plotSamplingRate' */
    confret = config_lookup_int(&conf, "plotSamplingRate", &plotSamplingRate);
    assert(confret == CONFIG_TRUE);
    char cOutput[128]; /* File name result is written */
    const char *cOutputTemp;
    confret = config_lookup_string(&conf, "output", &cOutputTemp);
    assert(confret == CONFIG_TRUE);
    strncpy(cOutput, cOutputTemp, 128);
    config_destroy(&conf);

    /***************************************************************/
    for (j = 0; j < nBody; ++j)
    {
        /*
         * Normalize the quaternions for each body
         * to ensure they represent rotations.
         */
        NormalizeVector(4, body[j].b.q);
        NormalizeVector(4, body[j].b.q0);
    }
    /***************************************************************/

    /*
     * Return body, bodyName, extForce, muscle, musclePair
     */
    pymCfg->nBody  = nBody ;
    pymCfg->nFiber = nMuscle ;
    pymCfg->body   = (pym_rb_t *)malloc(sizeof(pym_rb_t)*nBody  ) ;
    pymCfg->fiber  = (pym_mf_t   *)malloc(sizeof(pym_mf_t  )*nMuscle) ;
    memcpy(pymCfg->body , body , sizeof(pym_rb_t)*nBody  );
    memcpy(pymCfg->fiber, fiber, sizeof(pym_mf_t  )*nMuscle);

    int totFiber = 0;
    FOR_0(j, pymCfg->nBody) {
        printf("   %s has %d fibers.\n", pymCfg->body[j].b.name, pymCfg->body[j].b.nFiber);
        totFiber += pymCfg->body[j].b.nFiber;
    }
    assert(totFiber == 2*nMuscle);



    return 0;
    #undef csgfe
    #undef BODY
    #undef FIBER
}

void PymConvertRotParamInPlace(pym_config_t *pymCfg, pym_rot_param_t targetRotParam) {
    assert(targetRotParam == RP_EXP);
    int i;
    for (i = 0; i < pymCfg->nBody; ++i) {
        assert(pymCfg->body[i].b.rotParam == RP_QUAT_WFIRST);
        double v[3];
        /* chi 1 */
        QuatToV(v, pymCfg->body[i].b.q);
        memcpy(pymCfg->body[i].b.q, v, sizeof(double)*3);
        pymCfg->body[i].b.q[3] = DBL_MAX; /* Should be ignored */
        /* chi 0 */
        QuatToV(v, pymCfg->body[i].b.q0);
        memcpy(pymCfg->body[i].b.q0, v, sizeof(double)*3);
        pymCfg->body[i].b.q0[3] = DBL_MAX; /* Should be ignored */

        pymCfg->body[i].b.rotParam = RP_EXP;
    }
}

void GetR_i(cholmod_triplet **_R_i, const pym_rb_statedep_t *sd, const int i /* RB index */, const pym_config_t *pymCfg, cholmod_common *cc) {
    int j, k;
    /*
     * PARAMETER   i   global RB index
     * LOCAL       j   local MF index of RB i
     * LOCAL       k   <generic iterate variable>
     */
    const pym_rb_named_t *rbn = &pymCfg->body[i].b;
    const int nf = pymCfg->nFiber;
    const int nd = 6;
    cholmod_triplet *R_i = cholmod_allocate_triplet(nd*rbn->nFiber, nf, nd*rbn->nFiber, 0, CHOLMOD_REAL, cc);
    FOR_0(j, rbn->nFiber) {
        int fidx = rbn->fiber[j]; /* global MF index of local MF j of RB i */
        assert(fidx < nf);
        const double *attPos1, *attPos2;
        int oppositeBidx = -1;
        const pym_mf_named_t *mf = &pymCfg->fiber[ fidx ].b;
        if (i == mf->org) {
            attPos1 = mf->fibb_org;
            attPos2 = mf->fibb_ins;
            oppositeBidx = mf->ins;
        } else if (i == mf->ins) {
            attPos1 = mf->fibb_ins;
            attPos2 = mf->fibb_org;
            oppositeBidx = mf->org;
        } else {
            assert (0);
        }
        double pt1[3], pt2[3], direction[3];
        AffineTransformPoint(pt1, sd[i].W_1, attPos1);
        AffineTransformPoint(pt2, sd[oppositeBidx].W_1, attPos2);
        FOR_0(k, 3) direction[k] = pt2[k] - pt1[k];
        double dirLen = NormalizeVector(3, direction);
        assert(dirLen > 1e-4);
        double gf[6];
        GeneralizedForce(gf, rbn->q, direction, attPos1);

        FOR_0(k, 6) SET_TRIPLET_RCV(R_i, nd*j + k, fidx, gf[k]);
    }

//    /**** DEBUG ***/
//    printf("====R[%d]====\n", i);
//    cholmod_sparse *R_i_sp = cholmod_triplet_to_sparse(R_i, R_i->nnz, cc);
//    PrintEntireSparseMatrix(R_i_sp);
//    cholmod_free_sparse(&R_i_sp, cc);

    *_R_i = R_i;
}

void PymConstructBipedEqConst(pym_biped_eqconst_t *bod, const pym_rb_statedep_t *sd, const pym_config_t *pymCfg, cholmod_common *cc)
{
    const int nb = pymCfg->nBody;
    const int nf = pymCfg->nFiber;
    const int nj = pymCfg->nJoint;
    cholmod_triplet *A_trip[nb]; /* should be deallocated here */
    const int nd = 6;

    int Asubrowsizes[          ] = { 0, /* be calculated later */
                                     2*nd*nf,
                                     nf,
                                     4*nj,
                                     nj };
    int Asubcolsizes[          ] = { 0, /* be calculated later */
                                     nf,
                                     nf,
                                     nf,
                                     1,
                                     1,
                                     4*nj,
                                     nj };
    int i, j, l;
    FOR_0(i, nb) {
        GetAMatrix(A_trip + i, sd + i, pymCfg->body + i, pymCfg, cc);
        Asubrowsizes[0] += A_trip[i]->nrow;
        Asubcolsizes[0] += A_trip[i]->ncol;
    }
    const int Asubrows = sizeof(Asubrowsizes)/sizeof(int);
    const int Asubcols = sizeof(Asubcolsizes)/sizeof(int);
    int *Ari = (int *)malloc( sizeof(int)*(1+Asubrows) );
    int *Aci = (int *)malloc( sizeof(int)*(1+Asubcols) );
    bod->Ari = Ari;
    bod->Aci = Aci;
    Ari[0] = 0;
    Aci[0] = 0;
    FOR_0(i, Asubrows) Ari[i+1] = Ari[i] + Asubrowsizes[i];
    FOR_0(i, Asubcols) Aci[i+1] = Aci[i] + Asubcolsizes[i];

    int *Airi = (int *)malloc( sizeof(int)*(1+nb) );
    int *Aici = (int *)malloc( sizeof(int)*(1+nb) );
    bod->Airi = Airi;
    bod->Aici = Aici;
    Airi[0] = 0;
    Aici[0] = 0;
    FOR_0(i, nb) Airi[i+1] = Airi[i] + A_trip[i]->nrow;
    FOR_0(i, nb) Aici[i+1] = Aici[i] + A_trip[i]->ncol;

    size_t nzmax = 0;
    size_t totFiberCon = 0;
    FOR_0(i, nb) {
        nzmax += A_trip[i]->nnz;                /* Sub-block 01 - A_i */
        nzmax += nd*pymCfg->body[i].b.nFiber;   /* Sub-block 02 - [0 -1] */
        nzmax += nd*pymCfg->body[i].b.nFiber;   /* Sub-block 03 - R_i */

        totFiberCon += pymCfg->body[i].b.nFiber;
    }
    assert(totFiberCon == 2*nf); /* Since a fiber always connected to two different RB */
    nzmax += 3*nf;     /* Sub-block 04, 05, 06 - K_11, K_12, K_13 */
    nzmax += 2*4*nj;   /* sub-block 07 - D */
    nzmax += 4*nj;     /* sub-block 08 - (-1) */
    nzmax += nj;       /* sub-block 09 - (1) */

//    printf("    BipA matrix constants (nb)      : %d\n", nb);
//    printf("    BipA matrix constants (nf)      : %d\n", nf);
//    printf("    BipA matrix subblock dimension  : %d x %d\n", Asubrows, Asubcols);
//    printf("    BipA matrix size                : %d x %d\n", Ari[Asubrows], Aci[Asubcols]);
    cholmod_triplet *AMatrix_trip = cholmod_allocate_triplet(Ari[Asubrows], Aci[Asubcols], nzmax, 0, CHOLMOD_REAL, cc);
    assert(AMatrix_trip->nnz == 0);

    /* Sub-block 01 - A */
    int AoffsetRow = 0, AoffsetCol = 0;
    FOR_0(i, nb) {
        FOR_0(j, A_trip[i]->nnz) {
            SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 0, 0,
                                      AoffsetRow + ((int    *)(A_trip[i]->i))[j],
                                      AoffsetCol + ((int    *)(A_trip[i]->j))[j],
                                      ((double *)(A_trip[i]->x))[j]);
        }
        AoffsetRow += A_trip[i]->nrow;
        AoffsetCol += A_trip[i]->ncol;
    }
    assert(AoffsetRow == Ari[1] && AoffsetCol == Aci[1]);
    int EoffsetRow = 0, EoffsetCol = 0;
    /* Sub-block 02 - E := [0 -1 0] */
    FOR_0(i, nb) {
        const int nfi = pymCfg->body[i].b.nFiber;
        const int nai = pymCfg->body[i].b.nAnchor;
        FOR_0(j, nd*nfi) {
            SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 1, 0,
                                      EoffsetRow + j,
                                      EoffsetCol + A_trip[i]->ncol - nd*nfi + j - 4*nai,
                                      -1);
        }
        EoffsetRow += nd*pymCfg->body[i].b.nFiber;
        EoffsetCol += A_trip[i]->ncol;
    }
    assert(Ari[1]+EoffsetRow == Ari[2] && Aci[0]+EoffsetCol == Aci[1]);
    /* Sub-block 03 - R */
    int RoffsetRow = 0;
    FOR_0(i, nb) {
        cholmod_triplet *R_i;
        GetR_i(&R_i, sd, i, pymCfg, cc);
        FOR_0(j, R_i->nnz) {
            SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 1, 1,
                                      RoffsetRow + ((int    *)(R_i->i))[j],
                                      0          + ((int    *)(R_i->j))[j],
                                      ((double *)(R_i->x))[j]);
        }
        RoffsetRow += nd*pymCfg->body[i].b.nFiber;
        cholmod_free_triplet(&R_i, cc);
    }
    assert(Ari[1]+RoffsetRow == Ari[2]);
    /* Sub-block 04, 05, 06 - K_11, K_12, K_13 */
    FOR_0(i, nf) {
        double k[3];
        GetMuscleFiberK(k, pymCfg->fiber + i, pymCfg);
        SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 2, 1, i, i, k[0]);
        SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 2, 2, i, i, k[1]);
        SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 2, 3, i, i, k[2]);
    }
    /* TODO Sub-block 07 - D */
    FOR_0(i, nj) {
        FOR_0(l, 4) {
            const pym_anchored_joint_t *aJoint = pymCfg->anchoredJoints + i;
            SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 3, 0,
                                      4*i + l,
                                      bod->Aici[ aJoint->aIdx ] + sd[ aJoint->aIdx ].Aci[10] + 4*aJoint->aAnchorIdx + l,
                                      1);
            SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 3, 0,
                                      4*i + l,
                                      bod->Aici[ aJoint->bIdx ] + sd[ aJoint->bIdx ].Aci[10] + 4*aJoint->bAnchorIdx + l,
                                      -1);
        }
    }
    /* Sub-block 08 - (-1) */
    FOR_0(i, 4*nj)
        SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 3, 6, i, i, -1);
    /* Sub-block 09 - (1) */
    FOR_0(i, nj)
        SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 4, 7, i, i, 1);
    bod->bipMat = cholmod_triplet_to_sparse(AMatrix_trip, AMatrix_trip->nnz, cc);
    cholmod_free_triplet(&AMatrix_trip, cc); /* not needed anymore */
    assert(bod->bipMat);

    /*********************
     * Eta vector setup  *
     *********************/
    double *bipEta = calloc(Ari[Asubrows], sizeof(double));
    //memset(bipEta, 0, sizeof(double)*Ari[Asubrows]);
    int etaOffset = 0;
    FOR_0(i, nb) {
        double *etai;
        GetEta(&etai, sd + i, pymCfg->body + i, pymCfg, cc); /* Space for 'etai' is allocated at GetEta() */
        memcpy(bipEta + etaOffset, etai, sizeof(double)*A_trip[i]->nrow);
        free(etai); /* so free here */
        etaOffset += A_trip[i]->nrow;
    }
    FOR_0(i, nf) {
        bipEta[ Ari[2] + i ] = GetMuscleFiberS(i, sd, pymCfg);
    }
    FOR_0(i, nj) {
        bipEta[ Ari[4] + i ] = 0.05; /* TODO joint dislocation threshold */
    }
    bod->bipEta = bipEta;

    FOR_0(i, nb) cholmod_free_triplet(&A_trip[i], cc);
}

void PymSetPymCfgChiRefToCurrentState(pym_config_t *pymCfg) {
    int i;
    FOR_0(i, pymCfg->nBody) {
        memcpy(pymCfg->body[i].b.chi_ref  , pymCfg->body[i].b.p, sizeof(double)*3);
        memcpy(pymCfg->body[i].b.chi_ref+3, pymCfg->body[i].b.q, sizeof(double)*4);
    }
}
