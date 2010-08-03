#ifndef __PYMCMDLINEPARSER_H_
#define __PYMCMDLINEPARSER_H_

struct _pym_cmdline_options_t {
    const char *simconf;
    int frame;
    char *trajconf;
    char *trajdata;
    char *output;
    double slant;
    int notrack;
    int freeTrajStrings;
    int freeOutputStrings;
};

int PymParseCmdlineOptions(pym_cmdline_options_t *cmdopt, int argc, const char **argv);

#endif
