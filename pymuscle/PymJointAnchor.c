/*
 * PymJointAnchor.c
 *
 *  Created on: 2010. 7. 21.
 *      Author: johnu
 */
#include "PymPch.h"
#include "PymStruct.h"
#include "Config.h"
#include "PymJointAnchor.h"
#include "PymuscleConfig.h"
#include "RigidBody.h"

int PymParseJointAnchorFile(pym_joint_anchor_t *ja, const int maxNa, const char *fnJaCfg) {
    FILE *jaCfg = fopen(fnJaCfg, "r");
    if (jaCfg == 0) {
        printf("Joint anchor configuration file %s parse failed.\n", fnJaCfg);
        return -1;
    }
    int i;
    int na = 0;
    int nLine = 0;
    const char delimiters[] = " \n";
    while (1) {
        char *aLine = 0;
        size_t aLineLen = 0;
        ssize_t ret = getline(&aLine, &aLineLen, jaCfg);
        if (ret < 0 || aLineLen <= 1)
            break;
        ++nLine;
        if (maxNa <= na) {
            printf("Error - maximum capacity for storing anchors exceeded. (Max=%d)\n", maxNa);
            return -5;
        }
        //printf("%s", aLine);
        char *cp = strdup(aLine);
        char *name = strtok(cp, delimiters);
        char *bodyName = strtok(0, delimiters);
        char *dblStr;
        FOR_0(i, 3) {
            dblStr = strtok (0, delimiters);
            char *endp;
            ja[na].localPos[i] = strtod(dblStr, &endp);
            if (!(endp && *endp == '\0')) {
                printf("Error - corruption on anchor config file %s(%d)\n", fnJaCfg, nLine);
                return -2;
            }
        }
        strcpy(ja[na].name, name);
        strcpy(ja[na].bodyName, bodyName);
        ++na;
        free(cp);
        free(aLine);
    }
    printf("nCorresMap = %d\n", na);

    fclose(jaCfg);
    return na;
}

void ExtractAnchorIdentifier(char *iden, const char *aName) {
    int substrLen = (int)(strrchr(aName, '.') - strchr(aName, '.')) - 1;
    strncpy(iden, strchr(aName, '.')+1, substrLen);
    iden[substrLen] = 0;
    assert(iden[substrLen-1] == 'L' || iden[substrLen-1] == 'R');
}

int PymConstructAnchoredJointList(pym_config_t *pymCfg) {
    int marked[pymCfg->na];
    memset(marked, 0, pymCfg->na*sizeof(int));
    int njProcessed = 0;
    int j, k, l;
    const int nb = pymCfg->nBody;
    FOR_0(j, pymCfg->na-1) {
        if (marked[j])
            continue;
        const char *ajName = pymCfg->pymJa[j].name; /* has a form of 'JA.*L.[a|b]' */
        char ajName2[128]; /* extracts *L part */
        ExtractAnchorIdentifier(ajName2, ajName);
        for (k = j+1; k < pymCfg->na; ++k) {
            const char *akName = pymCfg->pymJa[k].name; /* has a form of 'JA.*L.[a|b]' */
            char akName2[128]; /* extracts *L part */
            ExtractAnchorIdentifier(akName2, akName);
            if (strcmp(ajName2, akName2) == 0) {
                assert( (ajName[ strlen(ajName)-1 ] == 'a' && akName[ strlen(akName)-1 ] == 'b')
                        || (ajName[ strlen(ajName)-1 ] == 'b' && akName[ strlen(akName)-1 ] == 'a') );
                /* these two anchors should be constrained as an anchored joint. */
                const char *ajbn = pymCfg->pymJa[j].bodyName;
                const char *akbn = pymCfg->pymJa[k].bodyName;
                const pym_rb_named_t *ajBody = 0, *akBody = 0;
                int ajBodyIdx = -1, akBodyIdx = -1;
                FOR_0(l, nb) {
                    if (strcmp(pymCfg->body[l].b.name, ajbn) == 0) {
                        ajBody = &pymCfg->body[l].b;
                        ajBodyIdx = l;
                    }
                    else if (strcmp(pymCfg->body[l].b.name, akbn) == 0) {
                        akBody = &pymCfg->body[l].b;
                        akBodyIdx = l;
                    }
                }
                if (!(ajBody && akBody && ajBodyIdx != akBodyIdx)) {
                    printf("Warn - a body indicated by an anchor does not exist:\n");
                    printf("       %s(%s) or %s(%s)\n", ajName, ajbn, akName, akbn);
                    break;
                }

                int ajIdx = -1, akIdx = -1;
                FOR_0(l, ajBody->nAnchor) {
                    if (strcmp(ajBody->jointAnchorNames[l], ajName) == 0) {
                        ajIdx = l;
                    }
                }
                FOR_0(l, akBody->nAnchor) {
                    if (strcmp(akBody->jointAnchorNames[l], akName) == 0) {
                        akIdx = l;
                    }
                }
                assert(ajIdx >= 0 && akIdx >= 0);

                pym_anchored_joint_t *aJoint = pymCfg->anchoredJoints + njProcessed;
                aJoint->aIdx        = ajBodyIdx;
                aJoint->aAnchorIdx  = ajIdx;
                aJoint->bIdx        = akBodyIdx;
                aJoint->bAnchorIdx  = akIdx;
                aJoint->maxDisloc   = 0;

//                FOR_0(l, 4) {
//                    SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 3, 0,
//                                              4*njProcessed + l,
//                                              bod->Aici[ ajBodyIdx ] + sd[ ajBodyIdx ].Aci[10] + 4*ajIdx + l,
//                                              1);
//                    SET_TRIPLET_RCV_SUBBLOCK2(AMatrix_trip, 3, 0,
//                                              4*njProcessed + l,
//                                              bod->Aici[ akBodyIdx ] + sd[ akBodyIdx ].Aci[10] + 4*akIdx + l,
//                                              -1);
//                }
                marked[j] = 1;
                marked[k] = 1;
                ++njProcessed;
                break;
            }
        }
    }
    pymCfg->nJoint = njProcessed;
    assert(njProcessed*2 <= pymCfg->na);
    return njProcessed;
}
