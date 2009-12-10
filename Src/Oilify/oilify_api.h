#pragma once

#include <CL/cl.h>
#include <CL/clext.h>

struct OclContext
{
	cl_context			cxGpuContext;
	cl_device_id*		cdDevices;
	cl_command_queue	cqCommandQueue;
	char*				cSourceCL;
	cl_kernel			ckKernel;
	cl_program			cpProgram;
	cl_mem				rgba;
	cl_mem				inten;
	cl_mem				outRgba;
	cl_mem				outDebug;
	size_t				szGlobalWorkSize[2];
	size_t				szLocalWorkSize[2];
};

#ifdef __cplusplus
extern "C" {
#endif
int oilify_prepare(OclContext* pOc, unsigned int w, unsigned int h, unsigned int radius, unsigned int exponent, int bFlipY);
int oilify_run(const OclContext* pOc, const unsigned char * const rgbData, unsigned int w, unsigned int h, unsigned char* ub_out);
int oilify_cleanup(OclContext* pOc);
int oilify_radius(const OclContext* pOc, unsigned int radius);
int oilify_exponent(const OclContext* pOc, unsigned int exponent);
#ifdef __cplusplus
}
#endif
