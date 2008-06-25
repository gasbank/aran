#ifndef __LOAD_ARN_H_
#define __LOAD_ARN_H_

#include <stdio.h>
#include <stdlib.h>


#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    extern
#endif
#endif


EXTERN_C int load_arn(const char* filename, void* ob);

#endif // #ifndef __LOAD_ARN_H_