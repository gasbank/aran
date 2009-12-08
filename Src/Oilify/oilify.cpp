#include "oilify.h"

#define HDASHLINE "-----------------------------------------------------------\n"
#define CLAMP(v,lo,hi) ((v)<(lo))?(lo):((v)>(hi))?(hi):(v)
#define CLAMP0255(v) CLAMP((v), 0, 255)
#define HISTSIZE (256)
#define GIMP_RGB_LUMINANCE_RED    (0.2126)
#define GIMP_RGB_LUMINANCE_GREEN  (0.7152)
#define GIMP_RGB_LUMINANCE_BLUE   (0.0722)
#define MAX(a,b) ((a)<(b))?(b):(a)

/* for GIMP source code compatability */
typedef int				gint;
typedef float			gfloat;
typedef unsigned int	guint;
typedef unsigned char	guchar;

static inline unsigned char
	intensityFromRgb(unsigned char rgb[3])
{
	return static_cast<unsigned char>(CLAMP0255(GIMP_RGB_LUMINANCE_RED*rgb[0] + GIMP_RGB_LUMINANCE_GREEN*rgb[1] + GIMP_RGB_LUMINANCE_BLUE*rgb[2]));
}


/*
* This is a special-case form of the powf() function, limited to integer
* exponents. It calculates e.g. x^13 as (x^8)*(x^4)*(x^1).
*
* x can be anything, y must be in [0,255]
*/
static inline gfloat
	fast_powf (gfloat x, gint y)
{
	gfloat value;
	gfloat x_pow[8];
	guint  y_uint = (guint) y;
	guint  bitmask;
	gint   i;

	if (y_uint & 0x01)
		value = x;
	else
		value = 1.0;

	x_pow[0] = x;

	for (bitmask = 0x02, i = 1;
		bitmask <= y_uint;
		bitmask <<= 1, i++)
	{
		/*  x_pow[i] == x_pow[i-1]^2 == x^(2^i)  */

		x_pow[i] = SQR (x_pow[i - 1]);

		if (y_uint & bitmask)
			value *= x_pow[i];
	}

	return value;
}


/*
* For each i in [0, HISTSIZE), hist[i] is the number of occurrences of
* pixels with intensity i. hist_rgb[][i] is the average color of those
* pixels with intensity i, but with each channel multiplied by hist[i].
* Write to dest a pixel whose color is a weighted average of all the
* colors in hist_rgb[][], biased heavily toward those with the most
* frequently-occurring intensities (as noted in hist[]).
*
* The weight formula is the same as in weighted_average_value().
*/
static inline void
	weighted_average_color (gint    hist[HISTSIZE],
	gint    hist_rgb[4][HISTSIZE],
	gfloat  exponent,
	guchar *dest,
	gint    bpp)
{
	gint   i, b;
	gint   hist_max = 1;
	gint   exponent_int = 0;
	gfloat div = 1.0e-6f;
	gfloat color[4] = { 0.0, 0.0, 0.0, 0.0 };

	for (i = 0; i < HISTSIZE; i++)
		hist_max = MAX (hist_max, hist[i]);

	if ((exponent - floor (exponent)) < 0.001 && exponent <= 255.0)
		exponent_int = (gint) exponent;

	for (i = 0; i < HISTSIZE; i++)
	{
		gfloat ratio = (gfloat) hist[i] / (gfloat) hist_max;
		gfloat weight;

		if (exponent_int)
			weight = fast_powf (ratio, exponent_int);
		else
			weight = pow (ratio, exponent);

		if (hist[i] > 0)
			for (b = 0; b < bpp; b++)
				color[b] += weight * (gfloat) hist_rgb[b][i] / (gfloat) hist[i];

		div += weight;
	}

	for (b = 0; b < bpp; b++)
	{
		gint c = (gint) (color[b] / div);

		dest[b] = (guchar) CLAMP0255 (c);
	}
}

inline static void
	printHelpMessage(const char* cmdname)
{
	printf("%s : Oilify a picture\n", cmdname);
	printf("\n");
	printf("  ARGUMENTS\n");
	printf("    input file          <string>\n");
	printf("    output file         <string>              (overwrites if exists)\n");
	printf("    radius   [optional] <int>     (0,30]      (default: 4)\n");
	printf("    exponent [optional] <float>   [0.0, 20.0] (default: 8.0)\n");
	printf("\n");
	printf("  EXAMPLE\n");
	printf("    %s input.png output.png\n", cmdname);
	printf("    %s input.png output.png 6.0\n", cmdname);
	printf("    %s input.png output.png 2.0 4.0\n", cmdname);
}

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
};

OclContext oc;

void Cleanup (OclContext& oc)
{
	if (oc.cxGpuContext)
	{
		clReleaseContext(oc.cxGpuContext);
		oc.cxGpuContext = 0;
	}
	if(oc.cqCommandQueue)
	{
		clReleaseCommandQueue(oc.cqCommandQueue);
		oc.cqCommandQueue = 0;
	}
	if(oc.cdDevices)
	{
		free(oc.cdDevices);
		oc.cdDevices = 0;
	}
	if(oc.cSourceCL)
	{
		free(oc.cSourceCL);
		oc.cSourceCL = 0;
	}
	if(oc.ckKernel)
	{
		clReleaseKernel(oc.ckKernel);
		oc.ckKernel = 0;
	}
	if(oc.cpProgram)
	{
		clReleaseProgram(oc.cpProgram);
		oc.cpProgram = 0;
	}

	if(oc.rgba)
	{
		clReleaseMemObject(oc.rgba);
		oc.rgba = 0;
	}
	if(oc.inten)
	{
		clReleaseMemObject(oc.inten);
		oc.inten = 0;
	}
	if(oc.outRgba)
	{
		clReleaseMemObject(oc.outRgba);
		oc.outRgba = 0;
	}
	if(oc.outDebug)
	{
		clReleaseMemObject(oc.outDebug);
		oc.outDebug = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////
//! Loads a Program file and prepends the cPreamble to the code.
//!
//! @return the source string if succeeded, 0 otherwise
//! @param cFilename        program filename
//! @param cPreamble        code that is prepended to the loaded file, typically a set of #defines or a header
//! @param szFinalLength    returned length of the code string
//////////////////////////////////////////////////////////////////////////////
char* oclLoadProgSource(const char* cFilename, const char* cPreamble, size_t* szFinalLength)
{
	// locals 
	FILE* pFileStream = NULL;
	size_t szSourceLength;

	// open the OpenCL source code file
#ifdef _WIN32   // Windows version
	if(fopen_s(&pFileStream, cFilename, "rb") != 0) 
	{       
		return NULL;
	}
#else           // Linux version
	pFileStream = fopen(cFilename, "rb");
	if(pFileStream == 0) 
	{       
		return NULL;
	}
#endif

	size_t szPreambleLength = strlen(cPreamble);

	// get the length of the source code
	fseek(pFileStream, 0, SEEK_END); 
	szSourceLength = ftell(pFileStream);
	fseek(pFileStream, 0, SEEK_SET); 

	// allocate a buffer for the source code string and read it in
	char* cSourceString = (char *)malloc(szSourceLength + szPreambleLength + 1); 
	memcpy(cSourceString, cPreamble, szPreambleLength);
	if (fread((cSourceString) + szPreambleLength, szSourceLength, 1, pFileStream) != 1)
	{
		fclose(pFileStream);
		free(cSourceString);
		return 0;
	}

	// close the file and return the total length of the combined (preamble + source) string
	fclose(pFileStream);
	if(szFinalLength != 0)
	{
		*szFinalLength = szSourceLength + szPreambleLength;
	}
	cSourceString[szSourceLength + szPreambleLength] = '\0';

	return cSourceString;
}

// Round Up Division function
size_t shrRoundUp(int group_size, int global_size) 
{
    int r = global_size % group_size;
    if(r == 0) 
    {
        return global_size;
    } else 
    {
        return global_size + group_size - r;
    }
}




//////////////////////////////////////////////////////////////////////////////
//! Get and log the binary (PTX) from the OpenCL compiler for the requested program & device
//!
//! @param cpProgram    OpenCL program
//! @param cdDevice     device of interest
//////////////////////////////////////////////////////////////////////////////
void oclLogBuildInfo(cl_program cpProgram, cl_device_id cdDevice)
{
    // write out the build log and ptx, then exit
    char cBuildLog[10240];
    clGetProgramBuildInfo(cpProgram, cdDevice, CL_PROGRAM_BUILD_LOG, 
                          sizeof(cBuildLog), cBuildLog, NULL );
    printf("\n%s\nBuild Log:\n%s\n%s\n", HDASHLINE, cBuildLog, HDASHLINE);
}


//////////////////////////////////////////////////////////////////////////////
//! Get and log the binary (PTX) from the OpenCL compiler for the requested program & device
//!
//! @param cpProgram                   OpenCL program
//! @param cdDevice                    device of interest
//! @param const char*  cPtxFileName   optional PTX file name
//////////////////////////////////////////////////////////////////////////////
void oclLogPtx(cl_program cpProgram, cl_device_id cdDevice, const char* cPtxFileName)
{
    // Grab the number of devices associated with the program
    cl_uint num_devices;
    clGetProgramInfo(cpProgram, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, NULL);

    // Grab the device ids
    cl_device_id* devices = (cl_device_id*) malloc(num_devices * sizeof(cl_device_id));
    clGetProgramInfo(cpProgram, CL_PROGRAM_DEVICES, num_devices * sizeof(cl_device_id), devices, 0);

    // Grab the sizes of the binaries
    size_t* binary_sizes = (size_t*)malloc(num_devices * sizeof(size_t));    
    clGetProgramInfo(cpProgram, CL_PROGRAM_BINARY_SIZES, num_devices * sizeof(size_t), binary_sizes, NULL);

    // Now get the binaries
    char** ptx_code = (char**)malloc(num_devices * sizeof(char*));
    for( unsigned int i=0; i<num_devices; ++i)
    {
        ptx_code[i] = (char*)malloc(binary_sizes[i]);
    }
    clGetProgramInfo(cpProgram, CL_PROGRAM_BINARIES, 0, ptx_code, NULL);

    // Find the index of the device of interest
    unsigned int idx = 0;
    while((idx < num_devices) && (devices[idx] != cdDevice)) 
    {
        ++idx;
    }
    
    // If the index is associated, log the result
    if(idx < num_devices)
    {
         
        // if a separate filename is supplied, dump ptx there 
        if (NULL != cPtxFileName)
        {
            printf("\nWriting ptx to separate file: %s ...\n\n", cPtxFileName);
            FILE* pFileStream = NULL;
            #ifdef _WIN32
                fopen_s(&pFileStream, cPtxFileName, "wb");
            #else
                pFileStream = fopen(cPtxFileName, "wb");
            #endif

            fwrite(ptx_code[idx], binary_sizes[idx], 1, pFileStream);
            fclose(pFileStream);        
        }
        else // log to logfile and console if no ptx file specified
        {
           printf("\n%s\nProgram Binary:\n%s\n%s\n", HDASHLINE, ptx_code[idx], HDASHLINE);
        }
    }

    // Cleanup
    free(devices);
    free(binary_sizes);
    for(unsigned int i = 0; i < num_devices; ++i)
    {
        free(ptx_code[i]);
    }
    free( ptx_code );
}


//////////////////////////////////////////////////////////////////////////////
//! Gets the id of the first device from the context
//!
//! @return the id 
//! @param cxGPUContext         OpenCL context
//////////////////////////////////////////////////////////////////////////////
cl_device_id oclGetFirstDev(cl_context cxGPUContext)
{
    size_t szParmDataBytes;
    cl_device_id* cdDevices;

    // get the list of GPU devices associated with context
    clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, 0, NULL, &szParmDataBytes);
    cdDevices = (cl_device_id*) malloc(szParmDataBytes);

    clGetContextInfo(cxGPUContext, CL_CONTEXT_DEVICES, szParmDataBytes, cdDevices, NULL);

    cl_device_id first = cdDevices[0];
    free(cdDevices);

    return first;
}

int main (int argc, const char* argv[])
{
	// Default parameters
	int radius = 2;
	float exponent = 10.0f;

	// Parse arguments
	if (argc == 3)
	{
	}
	else if (argc == 4)
	{
		radius = atoi(argv[3]);
	}
	else if (argc == 5)
	{
		radius = atoi(argv[3]);
		exponent = (float)atof(argv[4]);
	}
	else
	{
		printHelpMessage(argv[0]);
		return 0;
	}

	// Check radius and exponent boundaries)
	if (radius <= 0 || radius > 30 || exponent < 0 || exponent > 20)
	{
		printHelpMessage(argv[0]);
		return 0;
	}

	cl_int ciErr1, ciErr2;
	OclContext oc;
	memset(&oc, 0, sizeof(OclContext));
	oc.cxGpuContext = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		Cleanup(oc);
		exit(EXIT_FAILURE);
	}

	// Get the list of GPU devices associated with context
	size_t szParmDataBytes;
	ciErr1 = clGetContextInfo(oc.cxGpuContext, CL_CONTEXT_DEVICES, 0, NULL, &szParmDataBytes);
	oc.cdDevices = (cl_device_id*)malloc(szParmDataBytes);
	ciErr1 |= clGetContextInfo(oc.cxGpuContext, CL_CONTEXT_DEVICES, szParmDataBytes, oc.cdDevices, NULL);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clGetContextInfo, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		Cleanup(oc);
		exit(EXIT_FAILURE);
	}

	// Create a command-queue on the first device
	oc.cqCommandQueue = clCreateCommandQueue(oc.cxGpuContext, oc.cdDevices[0], 0, &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateCommandQueue, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		Cleanup(oc);
		exit(EXIT_FAILURE);
	}

	size_t szKernelLength;
	oc.cSourceCL = oclLoadProgSource("d:/devel3/aran/src/oilify/oilify.cl", "", &szKernelLength);

	// Create the program
	oc.cpProgram = clCreateProgramWithSource(oc.cxGpuContext, 1, (const char **)&oc.cSourceCL, &szKernelLength, &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateProgramWithSource, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		Cleanup(oc);
		exit(EXIT_FAILURE);
	}
	
	ciErr1 = clBuildProgram(oc.cpProgram, 0, NULL, "-cl-unsafe-math-optimizations -cl-finite-math-only -cl-fast-relaxed-math", NULL, NULL);
	oclLogBuildInfo(oc.cpProgram, oclGetFirstDev(oc.cxGpuContext));
	oclLogPtx(oc.cpProgram, oclGetFirstDev(oc.cxGpuContext), "oilify-log.ptx");
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clBuildProgram, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		Cleanup(oc);
		exit(EXIT_FAILURE);
	}

	// Create the kernel
	oc.ckKernel = clCreateKernel(oc.cpProgram, "Oilify", &ciErr1);
	if (ciErr1 != CL_SUCCESS)
	{
		printf("Error in clCreateKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		Cleanup(oc);
		exit(EXIT_FAILURE);
	}



	const char* inputFileName = argv[1];
	const char* outputFileName = argv[2];

	if (ArnInitializeImageLibrary() < 0)
	{
		printf("Image library initialization failed.\n");
		abort();
	}

	ArnTexture* texture = ArnTexture::createFrom(inputFileName);
	texture->init();
	printf("Dimension : %d x %d\n", texture->getWidth(), texture->getHeight());
	printf("      BPP : %d\n", texture->getBpp());
	printf("     Type : ");
	switch (texture->getFormat())
	{
	case ACF_RGB: printf("RGB"); break;
	case ACF_RGBA: printf("RGBA"); break;
	case ACF_BGR: printf("BGR"); break;
	case ACF_BGRA: printf("BGRA"); break;
	default: throw std::runtime_error("xxx");
	}
	printf("\n");
	const std::vector<unsigned char>& rgbData = texture->getRawData();
	const int w = texture->getWidth();
	const int h = texture->getHeight();
	const int bpp = texture->getBpp();
	assert(rgbData.size() == w*h*texture->getBpp());

	

	// Global and local work size calculation
	//
	// two-dimension work-item
	// Use 9x9 neighbor pixels are used to calculate a pixel output
	//

	size_t szGlobalWorkSize[2];
	size_t szLocalWorkSize[2];
	const size_t pixelNeighborSize = 9;
	
	/*
	szGlobalWorkSize[0] = shrRoundUp(pixelNeighborSize, w);
	szGlobalWorkSize[1] = shrRoundUp(pixelNeighborSize, h);
	szLocalWorkSize[0] = pixelNeighborSize;
	szLocalWorkSize[1] = pixelNeighborSize;
	*/
	szLocalWorkSize[0] = 256;
	szLocalWorkSize[1] = 1;
	szGlobalWorkSize[0] = shrRoundUp(256, w*h);
	szGlobalWorkSize[1] = 1;
	
	// Allocate the OpenCL buffer memory objects for source and result on the device GMEM
    printf("Actual global work size            = %d = %dx%d\n", w*h, w, h);
	printf("szGlobalWorkSize                   = %d = %dx%d\n", szGlobalWorkSize[0]*szGlobalWorkSize[1], szGlobalWorkSize[0], szGlobalWorkSize[1]);
	printf("szLocalWorkSize                    = %d = %dx%d\n", szLocalWorkSize[0]*szLocalWorkSize[1], szLocalWorkSize[0], szLocalWorkSize[1]);
	oc.rgba = clCreateBuffer(oc.cxGpuContext, CL_MEM_READ_ONLY, sizeof(cl_uchar4) * szGlobalWorkSize[0] * szGlobalWorkSize[1], NULL, &ciErr1);
    oc.inten = clCreateBuffer(oc.cxGpuContext, CL_MEM_READ_ONLY, sizeof(cl_uchar) * szGlobalWorkSize[0] * szGlobalWorkSize[1], NULL, &ciErr2);
    ciErr1 |= ciErr2;
    oc.outRgba = clCreateBuffer(oc.cxGpuContext, CL_MEM_WRITE_ONLY, sizeof(cl_uchar4) * szGlobalWorkSize[0] * szGlobalWorkSize[1], NULL, &ciErr2);
    ciErr1 |= ciErr2;
	oc.outDebug = clCreateBuffer(oc.cxGpuContext, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * szGlobalWorkSize[0] * szGlobalWorkSize[1], NULL, &ciErr2);
    ciErr1 |= ciErr2;
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clCreateBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(oc);
		exit(EXIT_FAILURE);
    }

    // Set the Argument values
	int argIdx = 0;
    ciErr1 = clSetKernelArg(oc.ckKernel, argIdx++, sizeof(cl_mem), (void*)&oc.rgba);
    ciErr1 |= clSetKernelArg(oc.ckKernel, argIdx++, sizeof(cl_mem), (void*)&oc.inten);
    ciErr1 |= clSetKernelArg(oc.ckKernel, argIdx++, sizeof(cl_mem), (void*)&oc.outRgba);
	//ciErr1 |= clSetKernelArg(oc.ckKernel, argIdx++, sizeof(cl_mem), (void*)&oc.outDebug);
    ciErr1 |= clSetKernelArg(oc.ckKernel, argIdx++, sizeof(cl_int), (void*)&w);
	ciErr1 |= clSetKernelArg(oc.ckKernel, argIdx++, sizeof(cl_int), (void*)&h);
	ciErr1 |= clSetKernelArg(oc.ckKernel, argIdx++, sizeof(cl_int), (void*)&radius);
	ciErr1 |= clSetKernelArg(oc.ckKernel, argIdx++, sizeof(cl_float), (void*)&exponent);
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clSetKernelArg, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(oc);
		exit(EXIT_FAILURE);
    }


	
	std::vector<unsigned char> srcInten(w * h);
	BwWin32Timer timer;
	timer.start();
#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			const int pixelOffset = w*y + x;
			unsigned char srcRgb[3] = { rgbData[bpp*pixelOffset + 0], rgbData[bpp*pixelOffset + 1], rgbData[bpp*pixelOffset + 2] };
			srcInten[pixelOffset] = intensityFromRgb(srcRgb);
		}
	}

	// --------------------------------------------------------
    // Start Core sequence... copy input data to GPU, compute, copy results back
	
    // Asynchronous write of data to GPU device
    ciErr1 = clEnqueueWriteBuffer(oc.cqCommandQueue, oc.rgba, CL_FALSE, 0, sizeof(cl_uchar4) * w * h, &rgbData[0], 0, NULL, NULL);
    ciErr1 |= clEnqueueWriteBuffer(oc.cqCommandQueue, oc.inten, CL_FALSE, 0, sizeof(cl_uchar) * w * h, &srcInten[0], 0, NULL, NULL);
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clEnqueueWriteBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(oc);
		exit(EXIT_FAILURE);
    }

	
    // Launch kernel
    ciErr1 = clEnqueueNDRangeKernel(oc.cqCommandQueue, oc.ckKernel, 1, NULL, szGlobalWorkSize, szLocalWorkSize, 0, NULL, NULL);
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clEnqueueNDRangeKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(oc);
		exit(EXIT_FAILURE);
    }

	std::vector<unsigned char> outData(w * h * 4); // output image consists of RGB channels
    // Synchronous/blocking read of results, and check accumulated errors
    ciErr1 = clEnqueueReadBuffer(oc.cqCommandQueue, oc.outRgba, CL_TRUE, 0, sizeof(cl_uchar4) * w * h, &outData[0], 0, NULL, NULL);
    if (ciErr1 != CL_SUCCESS)
    {
        printf("Error in clEnqueueReadBuffer, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
        Cleanup(oc);
		exit(EXIT_FAILURE);
    }
	printf("Process time: %lf ms\n", timer.getTicks());

	delete texture;

	ILuint ImageName;
	ilGenImages(1, &ImageName);
	ilBindImage(ImageName);
	ilTexImage(w, h, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, &outData[0]);
	ilEnable(IL_FILE_OVERWRITE);
	ilSave(IL_PNG, outputFileName);
	ilDeleteImages(1, &ImageName);
		
	if (ArnCleanupImageLibrary() < 0)
	{
		printf("Image library cleanup failed.\n");
	}
	
	Cleanup(oc);

	return 0;
}
