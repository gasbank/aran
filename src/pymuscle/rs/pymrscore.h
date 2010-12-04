#ifndef __PYMRSCORE_H__
#define __PYMRSCORE_H__

void PrintCmdLineHelp(int argc, char *argv[]);
void pym_init_debug_msg_streams(FILE *dmstreams[]);

struct pym_rs_t {
  pym_config_t			 pymCfg;
  pym_cmdline_options_t		 cmdopt;
  pym_traj_t			 pymTraj;
  FILE				*outputFile;
  FILE				*dmstreams[PDMTE_COUNT];
  bool *drawing_options;
  pym_physics_thread_context_t	 phyCon;
  cholmod_common		 cc;
  MSKenv_t			 env;
  int ssIdx; /* screenshot start index */
};

int pym_init_global(int argc, char *argv[], pym_config_t *pymCfg,
		    pym_traj_t *pymTraj,
		    pym_cmdline_options_t *cmdopt,
		    FILE** _outputFile,
		    FILE *dmstreams[]);

pym_physics_thread_context_t
pym_init_phy_thread_ctx(pym_config_t *pymCfg,
			pym_cmdline_options_t *cmdopt,
			pym_traj_t *pymTraj,
			FILE *outputFile,
			FILE *dmstreams[]);

#ifdef __cplusplus
extern "C" {
#endif
  PYMRS_API pym_rs_t *PymRsInitContext(int argc, char *argv[]);
  PYMRS_API void PymRsDestroyContext(pym_rs_t *rs);
  PYMRS_API inline FILE *PymGetDevnull();
#ifdef __cplusplus
}
#endif

#endif // #ifndef __PYMRSCORE_H__
