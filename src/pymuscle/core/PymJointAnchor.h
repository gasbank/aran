/*
 * PymJointAnchor.h
 *
 *  Created on: 2010. 7. 21.
 *      Author: johnu
 */

#ifndef PYMJOINTANCHOR_H_
#define PYMJOINTANCHOR_H_

struct pym_joint_anchor_t {
    char name[128];
    char bodyName[128];
    double localPos[3];
};

struct pym_anchored_joint_t {
    int aIdx;             /* body 'a' index */
    int aAnchorIdx;       /* joint anchor index of body 'a' */
    int aAnchorIdxGbl;    /* joint anchor index in global list */
    int bIdx;             /* body 'b' index */
    int bAnchorIdx;       /* joint anchor index of body 'b' */
    int bAnchorIdxGbl;    /* joint anchor index in global list */
    double maxDisloc;
};

struct pym_config_t;

PYMCORE_API void ExtractAnchorIdentifier(char *iden, const char *aName);
PYMCORE_API int PymConstructAnchoredJointList(pym_config_t *pymCfg);
PYMCORE_API int PymInitJointAnchors(pym_config_t *pymCfg, FILE *dmstreams[]);
PYMCORE_API int PymParseJointAnchorFile(pym_joint_anchor_t *ja, const int maxNa, const char *fnJaCfg);
PYMCORE_API int PymInferJointAnchorConfFileName(char fnJaCfg[128], const char *fnTrajCfg);

#endif /* PYMJOINTANCHOR_H_ */
