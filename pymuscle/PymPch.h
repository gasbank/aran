/*
 * PymPch.h
 *
 *  Created on: 2010. 7. 22.
 *      Author: johnu
 */

#ifndef PYMPCH_H_
#define PYMPCH_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#ifndef WIN32
#include <execinfo.h>
#endif

/* PYMCORE */
#ifdef WIN32
#define PYMCORE_API_EXPORT __declspec(dllexport)
#define PYMCORE_API_IMPORT __declspec(dllimport)
#else
#define PYMCORE_API_EXPORT
#define PYMCORE_API_IMPORT
#endif

#if defined(pymcore_EXPORTS)
#define PYMCORE_API			PYMCORE_API_EXPORT
#define PYMCORE_API_EXTERN
#else
#define PYMCORE_API			PYMCORE_API_IMPORT
#define PYMCORE_API_EXTERN		extern
#endif

/* PYMPARSER */
#ifdef WIN32
#define PYMPARSER_API_EXPORT __declspec(dllexport)
#define PYMPARSER_API_IMPORT __declspec(dllimport)
#else
#define PYMPARSER_API_EXPORT
#define PYMPARSER_API_IMPORT
#endif

#if defined(pymparser_EXPORTS)
#define PYMPARSER_API			PYMPARSER_API_EXPORT
#define PYMPARSER_API_EXTERN
#else
#define PYMPARSER_API			PYMPARSER_API_IMPORT
#define PYMPARSER_API_EXTERN		extern
#endif

/* PYMOPTIMIZER */
#ifdef WIN32
#define PYMOPTIMIZER_API_EXPORT __declspec(dllexport)
#define PYMOPTIMIZER_API_IMPORT __declspec(dllimport)
#else
#define PYMOPTIMIZER_API_EXPORT
#define PYMOPTIMIZER_API_IMPORT
#endif

#if defined(pymoptimizer_EXPORTS)
#define PYMOPTIMIZER_API			PYMOPTIMIZER_API_EXPORT
#define PYMOPTIMIZER_API_EXTERN
#else
#define PYMOPTIMIZER_API			PYMOPTIMIZER_API_IMPORT
#define PYMOPTIMIZER_API_EXTERN		extern
#endif


#endif /* PYMPCH_H_ */
