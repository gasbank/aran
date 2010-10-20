/*
 * image.h - header file for image.c
 * Copyright (c) 2007 Cesare Tirabassi <norsetto@ubuntu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <GL/gl.h>
#include <GL/glext.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <malloc.h>
#include "common.h"

/*
 * Data definitions
 */

//FOURCC codes for DX compressed-texture pixel formats 

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | \
	((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24))
#endif //MAKEFOURCC

#define FOURCC_DXT1 MAKEFOURCC('D', 'X', 'T', '1')
#define FOURCC_DXT2 MAKEFOURCC('D', 'X', 'T', '2')
#define FOURCC_DXT3 MAKEFOURCC('D', 'X', 'T', '3')
#define FOURCC_DXT4 MAKEFOURCC('D', 'X', 'T', '4')
#define FOURCC_DXT5 MAKEFOURCC('D', 'X', 'T', '5')

/* For little-endian arches:

FOURCC_DXT1 827611204
FOURCC_DXT2 844388420
FOURCC_DXT3 861165636
FOURCC_DXT4 877942852
FOURCC_DXT5 894720068

*/

#define DDSD_CAPS                  0x00000001
#define DDSD_HEIGHT                0x00000002
#define DDSD_WIDTH                 0x00000004
#define DDSD_PITCH                 0x00000008
#define DDSD_PIXELFORMAT           0x00001000
#define DDSD_MIPMAPCOUNT           0x00020000
#define DDSD_LINEARSIZE            0x00080000
#define DDSD_DEPTH                 0x00800000

#define DDPF_ALPHAPIXELS           0x00000001
#define DDPF_ALPHA                 0x00000002
#define DDPF_FOURCC                0x00000004
#define DDPF_PALETTEINDEXED8       0x00000020
#define DDPF_RGB                   0x00000040
#define DDPF_LUMINANCE             0x00020000

#define DDSCAPS_COMPLEX            0x00000008
#define DDSCAPS_TEXTURE            0x00001000
#define DDSCAPS_MIPMAP             0x00400000

#define DDSCAPS2_CUBEMAP           0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000
#define DDSCAPS2_VOLUME            0x00200000

// TGAHEADER - tga header structure.

#pragma pack(1)
typedef struct TGAHEADER_TYPE
{
    char	identsize;              // Size of ID field that follows header (0)
    char	colorMapType;           // 0 = None, 1 = paletted
    char	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
    unsigned short	colorMapStart;  // First colour map entry
    unsigned short	colorMapLength; // Number of colors
    unsigned char 	colorMapBits;   // bits per palette entry
    unsigned short	xstart;         // image x origin
    unsigned short	ystart;         // image y origin
    unsigned short	width;          // width in pixels
    unsigned short	height;         // height in pixels
    char	bits;                   // bits per pixel (8, 16, 24, 32)
    char	descriptor;             // image descriptor
} TGAHEADER, *TGAHEADER_PTR;

// PCXHEADER - pcx header structure.

typedef struct PCXHEADER_TYPE
{
	unsigned char	manufacturer;		// manufacturer
	unsigned char	version;		// version
	unsigned char	encoding;		// encoding type
	unsigned char	bitsPerPixel;		// number of bits per pixel

	unsigned short	xStart, yStart;		// start coordinates
	unsigned short	xEnd, yEnd;		// end coordinates
	unsigned short	horzRes, vertRes;	// horizontal and vertical screen resolutions

	unsigned char	*palette;		// color palette
	unsigned char	reserved;		// reserved
	unsigned char	numColorPlanes;		// number of planes

	unsigned short	bytesPerScanLine;	// byte per line
	unsigned short	paletteType;		// palette type
	unsigned short	horzSize, vertSize;	// ...

	unsigned char	padding[54];		// ...

} PCXHEADER, *PCXHEADER_PTR;
#pragma pack(8)

typedef struct IMAGE_TYPE
{
	GLubyte		*data;		//Image data
	unsigned int	components;	//Image color depth in bytes per pixel
	unsigned int	width;		//Image width
	unsigned int	height;		//Image height
	GLenum		format;		//Data format, eg GL_RGBA
	unsigned int	numMipMaps;	//Number of MipMaps (for DDS formats)

} IMAGE, *IMAGE_PTR;

/*
 * Function prototypes
 */

GLboolean LoadImageFromDisk(IMAGE_PTR image, const char *filename);
GLboolean LoadBMP(IMAGE_PTR image, const char *filename);
GLboolean LoadTGA(IMAGE_PTR image, const char *filename);
GLboolean LoadPCX(IMAGE_PTR image, const char *filename);
GLboolean LoadDDS(IMAGE_PTR image, const char *filename);
GLboolean WriteTGA(IMAGE_PTR image, const char* filename);
GLboolean FlipVertically(IMAGE_PTR image);
GLboolean LoadTexture( GLint* texture_id, const char* filename, GLboolean clamp, GLfloat AnisoLevel );
GLboolean SaveTexture( GLint texture_id, const char* filename);
GLboolean PrintScreen( const char* filename );

#endif //IMAGE_H

