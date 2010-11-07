/*
 * PymPodUtil.c
 *
 *  Created on: 2010. 7. 22.
 *      Author: johnu
 */
#include "PymCorePch.h"

int FindBodyIndex(int nBody, char bodyName[/*nBody*/][128], const char *bn) {
    int i;
    for (i = 0; i < nBody; ++i)
    {
        if (strncmp(bodyName[i], bn, 128) == 0)
            return i;
    }
    return -1;
}
