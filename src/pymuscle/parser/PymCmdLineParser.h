#ifndef __PYMCMDLINEPARSER_H_
#define __PYMCMDLINEPARSER_H_

struct pym_cmdline_options_t {
    const char *simconf;
    int frame;
    char *trajconf;
    char *trajdata;
    char *output;
    double slant;
    int notrack;
    int freeTrajStrings;
    int freeOutputStrings;
    int test; /* test mode: 0 (off, default), 1 (on) */
};

PYMPARSER_API int PymParseCmdlineOptions(pym_cmdline_options_t *cmdopt, int argc, char *argv[]);

#endif
