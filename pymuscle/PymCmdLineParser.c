#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "PymStruct.h"
#include "PymCmdLineParser.h"

int PymParseCmdlineOptions(pym_cmdline_options_t *cmdopt, int argc, const char **argv) {
    /* initialize default options first */
    cmdopt->simconf           =  0;
    cmdopt->frame             = -1;   /* not specified for now */
    cmdopt->trajconf          =  0;
    cmdopt->trajdata          =  0;
    cmdopt->output            =  0;
    cmdopt->notrack           =  0;
    cmdopt->freeTrajStrings   =  0;
    cmdopt->freeOutputStrings =  0;

    assert(argc >= 2);
    cmdopt->simconf = argv[1];
    int i;
    int _trajconf = 0, _trajdata = 0;
    for (i = 2; i < argc; ++i) {
        if (strncmp(argv[i], "--frame=", strlen("--frame=")) == 0) {
            char *endp;
            cmdopt->frame = strtol( strchr(argv[i], '=') + 1, &endp, 10 );
            if (!(endp && *endp == '\0') || cmdopt->frame <= 0) {
                printf("Error: --frame should be provided as a positive integer.\n");
                return -4;
            }
        } else if (strncmp(argv[i], "--trajconf=", strlen("--trajconf=")) == 0) {
            cmdopt->trajconf = strchr(argv[i], '=') + 1;
            _trajconf = 1;
        } else if (strncmp(argv[i], "--trajdata=", strlen("--trajdata=")) == 0) {
            cmdopt->trajdata = strchr(argv[i], '=') + 1;
            _trajdata = 1;
        } else if (strncmp(argv[i], "--output=", strlen("--output=")) == 0) {
            cmdopt->output = strchr(argv[i], '=') + 1;
        } else if (strncmp(argv[i], "--notrack", strlen("--notrack")) == 0) {
                    cmdopt->notrack = 1;
        } else if (strncmp(argv[i], "--slant=", strlen("--slant=")) == 0) {
            char *endp;
            cmdopt->slant = strtod(strchr(argv[i], '=') + 1, &endp);
            if (!(endp && *endp == '\0')) {
                printf("Error: --slant should be provided as a real number.\n");
                return -3;
            }
        } else {
            printf("Error: unknown argument provided - %s\n", argv[i]);
            return -2;
        }
    }
    if (_trajconf ^ _trajdata) {
        printf("Error: --trajconf should be specified with --trajdata and vice versa.\n\n");
        return -1;
    }
    if (cmdopt->notrack == 0 && cmdopt->trajconf == 0 && cmdopt->trajdata == 0) {
        /* trajconf and trajdata path implicitly decided
         *  from sim conf file name. In this case
         *  we assume the user provided conventional
         *  file name for sim conf. */
        assert( strcmp(strchr(cmdopt->simconf, '.'), ".sim.conf") == 0 );
        cmdopt->freeTrajStrings = 1;
        int prefixLen = (int)(strchr(cmdopt->simconf, '.') - cmdopt->simconf);

        cmdopt->trajconf = malloc(prefixLen + strlen(".traj.conf") + 1);
        strncpy(cmdopt->trajconf, cmdopt->simconf, prefixLen);
        cmdopt->trajconf[prefixLen] = '\0';
        strcat(cmdopt->trajconf, ".traj.conf");

        cmdopt->trajdata = malloc(prefixLen + strlen(".traj_EXP_q.txt") + 1);
        strncpy(cmdopt->trajdata, cmdopt->simconf, prefixLen);
        cmdopt->trajdata[prefixLen] = '\0';
        strcat(cmdopt->trajdata, ".traj_EXP_q.txt");

        /* set output file name accordingly if needed */
        if (cmdopt->output == 0) {
            cmdopt->freeOutputStrings = 1;
            cmdopt->output = malloc(prefixLen + strlen(".sim_EXP_q.txt") + 1);
            strncpy(cmdopt->output, cmdopt->simconf, prefixLen);
            cmdopt->output[prefixLen] = '\0';
            strcat(cmdopt->output, ".sim_EXP_q.txt");
        }
    }
    if (cmdopt->output == 0) {
        /* use default output file name if not set so far */
        cmdopt->output = "sample.sim_EXP_q.txt";
    }
    return 0;
}
