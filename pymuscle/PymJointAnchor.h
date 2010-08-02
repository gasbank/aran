/*
 * PymJointAnchor.h
 *
 *  Created on: 2010. 7. 21.
 *      Author: johnu
 */

#ifndef PYMJOINTANCHOR_H_
#define PYMJOINTANCHOR_H_

struct _pym_joint_anchor_t {
    char name[128];
    char bodyName[128];
    double localPos[3];
};

struct _pym_anchored_joint_t {
    int aIdx;             /* body 'a' index */
    int aAnchorIdx;       /* joint anchor index of body 'a' */
    int aAnchorIdxGbl;    /* joint anchor index in global list */
    int bIdx;             /* body 'b' index */
    int bAnchorIdx;       /* joint anchor index of body 'b' */
    int bAnchorIdxGbl;    /* joint anchor index in global list */
    double maxDisloc;
};

inline void ExtractAnchorIdentifier(char *iden, const char *aName);

int PymInferJointAnchorConfFileName(char fnJaCfg[128], const char *fnTrajCfg);
int PymParseJointAnchorFile(pym_joint_anchor_t *ja, const int maxNa, const char *fnJaCfg);
int PymConstructAnchoredJointList(pym_config_t *pymCfg);
int PymInitJointAnchors(pym_config_t *pymCfg, FILE *dmstreams[]);

#endif /* PYMJOINTANCHOR_H_ */
