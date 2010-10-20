/*
 * image.c - utilities for loading and saving image files
 *
 * For the time being it only handles the following formats:
 *
 * bmp, tga (with or without RLE), pcx and dds (DTX1, DTX3 and DTX5)
 *
 * Simple test program for OpenGL and glX
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
 */

#include "image.h"

/*
 * Module globals
 */

FILE *pFile;
unsigned int iSize;

/*
 * Load an image from a file
 */

GLboolean LoadImageFromDisk(IMAGE_PTR image, const char * filename)
{
int filenameLength;

	//Call the correct loading function based on the filename
	filenameLength=strlen(filename);

	if(	strncmp(filename+filenameLength-3, "BMP", 3) == 0 ||
		strncmp(filename+filenameLength-3, "bmp", 3) == 0)
		return LoadBMP(image, filename);

	if(	strncmp(filename+filenameLength-3, "TGA", 3) == 0 ||
		strncmp(filename+filenameLength-3, "tga", 3) == 0)
		return LoadTGA(image, filename);

	if(	strncmp(filename+filenameLength-3, "PCX", 3) == 0 ||
		strncmp(filename+filenameLength-3, "pcx", 3) == 0)
		return LoadPCX(image, filename);

	if(	strncmp(filename+filenameLength-3, "DDS", 3) == 0 ||
		strncmp(filename+filenameLength-3, "dds", 3) == 0)
		return LoadDDS(image, filename);

	printf("<!> File %s does not end with a valid extension\n", filename);
	return GL_FALSE;
}

//Open a TGA and find out what type it is
GLboolean LoadTGA(IMAGE_PTR image, const char * filename)
{
TGAHEADER tgaHeader;	// TGA file header

	//Clear the data in image
	if(image->data)
		free(image->data);
	
	image->data=NULL;
	image->components=3;
	image->format=GL_RGB;
	image->height=0;
	image->width=0;
	image->numMipMaps=0;

    // Attempt to open the file
    if(!(pFile = fopen(filename, "rb")))
    {
	printf("<!> File %s cannot be opened\n", filename);
	return GL_FALSE;
    }

    // Read in header (binary)
    if (!fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile))
    {
	printf("<!> Error reading file %s header data\n", filename);
	fclose(pFile);
	return GL_FALSE;
    }

    //Check if image is a valid type (only type 2, 3 or 10 is handled here)
    if( ( tgaHeader.imageType != 2 ) && ( tgaHeader.imageType != 3 ) && ( tgaHeader.imageType != 10 ) )
    {
	printf("<!> Image type %d in %s cannot be handled\n", tgaHeader.imageType, filename);
	fclose(pFile);
	return GL_FALSE;
    }

    // Get width, height, and depth of image
    image->width = tgaHeader.width;
    image->height = tgaHeader.height;
    image->components = tgaHeader.bits / 8;

    // Put some validity checks here. Very simply, I only understand
    // or care about 8, 24, or 32 bit targa's.
    if(image->components != 1 && image->components != 3 && image->components != 4)
    {
	printf("<!> File format in %s is not 8, 24 or 32 bits\n", filename);
	fclose(pFile);
	return GL_FALSE;
    }

    // Calculate size of image buffer
    iSize = image->width * image->height * image->components;

    // Allocate memory and check for success
    if((image->data = (unsigned char *)malloc(iSize * sizeof(unsigned char))) == NULL)
    {
	printf("<!> Not enough memory to load image in %s\n", filename);
        fclose(pFile);
	return GL_FALSE;
    }

    // Read in the bits
    if(tgaHeader.imageType == 10)
    {
    int n=0, i, j, k;
    unsigned char p[5];
    unsigned char* pixel;

	pixel = image->data;

	while( n < tgaHeader.width * tgaHeader.height )
	{
		if( fread(p, 1, image->components+1, pFile) != image->components+1 )
		{
			free(image->data);
			fclose(pFile);
			printf("<!> Error reading image data in file %s\n", filename);
			return GL_FALSE;
		}

		//Read repetition count
		j = p[0] & 0x7f;

		//Read pixel data
		for( k = 1; k < image->components+1; k++)
			*pixel++ = p[k];
		n++;

		if (p[0] & 0x80) /* RLE chunk */
		{
			for ( i = 0; i < j; i++ )
			{
				//Repeat pixel data for the run length
				for( k = 1; k< image->components+1; k++)
					*pixel++ = p[k];
				n++;
			}
		} else /* Normal chunk */
		{
			for ( i = 0; i < j; i++ )
			{
				if ( fread(p, 1, image->components, pFile) != image->components )
				{
					free(image->data);
					fclose(pFile);
					printf("<!> Unexpected end of file at pixel %d in file %s\n", i, filename);
					return GL_FALSE;
				}
				for( k = 0; k< image->components; k++)
					*pixel++ = p[k];
				n++;
			}
		}
	}
    }
    else if(fread(image->data, iSize, 1, pFile)!=1)
    {
	free(image->data);
	fclose(pFile);
	printf("<!> Error reading image data in file %s\n", filename);
	return GL_FALSE;
    }

    // Set OpenGL format expected
    switch(image->components)
    {
	case 3:
		image->format = GL_BGR_EXT;
		break;
	case 4:
		image->format = GL_BGRA_EXT;
		break;
	case 1:
		image->format = GL_LUMINANCE;
		break;
     };

    // Done with File
    fclose(pFile);

    return GL_TRUE;
}

//Open a BMP and find out what type it is
GLboolean LoadBMP(IMAGE_PTR image, const char * filename)
{
char magic[2];
short int Depth;

	//Clear the data in image
	if(image->data)
		free(image->data);
	
	image->data=NULL;
	image->components=3;
	image->format=GL_BGR;
	image->height=0;
	image->width=0;
	image->numMipMaps=0;

	// Attempt to open the file
	if(!(pFile = fopen(filename, "rb")))
	{
		printf("<!> File %s cannot be opened\n", filename);
		return GL_FALSE;
	}

	fread(magic, sizeof(magic), 1, pFile);

	if ((magic[0] != 'B') || (magic[1] != 'M'))
	{
		printf("<!> File %s is not a valid BMP file\n", filename);
		return GL_FALSE;
        }

	fseek(pFile, 16, SEEK_CUR);
	fread(&image->width,  sizeof(image->width),  1, pFile);
	fread(&image->height, sizeof(image->height), 1, pFile);
	fseek(pFile, 2, SEEK_CUR);
	fread(&Depth, sizeof(Depth), 1, pFile);

	image->components = (unsigned int)Depth / 8;

	// Put some validity checks here. Very simply, I only understand
	// or care about 8, 24, or 32 bit bmp's.
	if(image->components != 1 && image->components != 3 && image->components != 4)
	{
		printf("<!> File format in %s is not 8, 24 or 32 bits\n", filename);
	        return GL_FALSE;
	}

	// Calculate size of image buffer
	iSize = image->width * image->height * image->components;

	// Allocate memory and check for success
	if( !(image->data = (unsigned char *)malloc(iSize * sizeof(unsigned char))) )
	{
		printf("<!> Not enough memory to load image in %s\n", filename);
		return GL_FALSE;
	}


	// Read in the bits
	// Check for read error. This should catch RLE or other 
	// weird formats that I don't want to recognize
	fseek(pFile, 24, SEEK_CUR);
	if (fread(image->data, iSize, 1, pFile)!=1)
	{
		free(image->data);
		printf("<!> Error reading image data in file %s\n", filename);
		return GL_FALSE;
	}
	
	// Set OpenGL format expected
	switch(image->components)
	{
		case 3:     // Most likely case
			image->format = GL_BGR_EXT;
			break;
		case 4:
			image->format = GL_BGRA_EXT;
			break;
		case 1:
			image->format = GL_LUMINANCE;
			break;
	};

	// Done with File
	fclose(pFile);

	return GL_TRUE;
}

/*
 * LoadFilePCX() - load a Zsoft PCX image [.pcx]
 *
 * parameters :
 *	- filename [in]  : image source file
 *
 * return value :
 *	- pointer to image data
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * accepted image formats :
 *     # RLE 8 bits / version 5
 */

GLboolean LoadPCX(IMAGE_PTR image, const char *filename)
{
PCXHEADER_PTR	pcxHeader;	// PCX Header
int		flength = -1;	// General file lenght
unsigned char	*data   = NULL;	// Uncompressed paletted image data
unsigned char	*pImage = NULL;	// Image pointer
unsigned char	*pixels = NULL;	// Temporary image pointer
char		*pBuff  = NULL;	// Temporary buffer
char		*buffer = NULL;	// Temporary PCX file storage
unsigned char	c;
unsigned int	idx = 0;
unsigned int	numRepeat, i, j;

	//Clear the data in image
	if(image->data)
		free(image->data);
	
	image->data=NULL;
	image->components=3;
	image->format=GL_RGB;
	image->height=0;
	image->width=0;
	image->numMipMaps=0;

	// Attempt to open the file
	if(!(pFile = fopen(filename, "rb")))
	{
		printf("<!> File %s cannot be opened\n", filename);
		return GL_FALSE;
	}

	// Find the lenght of the file
	fseek(pFile, 0, SEEK_END);
	flength = ftell(pFile);

	// Read the entire file in the buffer
	fseek(pFile, 0, SEEK_SET);
	if ( (buffer = (char *)malloc(flength+1)) == NULL )
	{
		fclose(pFile);
		printf("<!> Unable to allocate memory for PCX image in %s\n", filename);
		return GL_FALSE;
	}

	// Done with File
	fclose(pFile);

	// Check file validity
	pcxHeader = (PCXHEADER_PTR)buffer;

	if( (pcxHeader->manufacturer	!= 10)	||
		(pcxHeader->version	!= 5)	||
		(pcxHeader->encoding	!= 1)	||
		(pcxHeader->bitsPerPixel!= 8) )
	{
		printf("<!> PCX image in %s is not valid\n", filename);
		return GL_FALSE;
	}

	// Get width, height, and size of decompressed image
	
	image->width = pcxHeader->xEnd - pcxHeader->xStart + 1;
	image->height = pcxHeader->yEnd - pcxHeader->yStart + 1;
	iSize = image->width * image->height;

	// Allocate memory for image data
	if ( !(data = (unsigned char *)malloc(iSize)))
	{
		free(buffer);
		printf("<!> Not enough memory to decompress image data in %s\n", filename);
		return GL_FALSE;
	}

	// Pointer to compressed image
	pBuff = (char *)&buffer[ 128 ];

	// Decode compressed image (RLE)
	while( idx < iSize )
	{
		if( (c = *(pBuff++)) > 0xbf )	//2 MSBs are set to 1, it is run encoded
		{
			numRepeat = 0x3f & c;		//the remaining 6 bits contains the run lngth
			c = *(pBuff++);			//read color data from next byte

			for( i = 0; i < numRepeat; i++ )
				data[ idx++ ] = c;	//fill image data
		}
		else
			data[ idx++ ] = c;		//simply copy image data
	}

	// the palette is located at the 769th last byte of the file
	pBuff = &buffer[ flength - 769 ];

	// verify the palette; first char must be equal to 12
	if( *(pBuff++) != 12 )
	{
		free(buffer);
		free(data);
		printf("<!> Palette in PCX image from %s is not correct\n", filename);
		return GL_FALSE;
	}

	// read the palette
	pcxHeader->palette = (unsigned char *)pBuff;

	// allocate memory for 24 bits pixel data
	iSize *= 3;
	pixels = (unsigned char *)malloc(iSize);

	// convert from paletted to 24 bits rgb pixels
	for( j = image->height - 1; j > 0; j-- )
	{
		pImage = &pixels[ j * image->width * 3 ];
		for( i = 0; i < image->width; i++, pImage += 3 )
		{
			int color = 3 * data[ j * image->width + i ];

			pImage[0] = (unsigned char)pcxHeader->palette[ color     ];
			pImage[1] = (unsigned char)pcxHeader->palette[ color + 1 ];
			pImage[2] = (unsigned char)pcxHeader->palette[ color + 2 ];
		}
	}

	//return data
	image->data = pImage;

	// free memory
	free(data);
	free(buffer);

	return GL_TRUE;
}

/*
 * Open a DDS and find out what type it is
 * Supports the loading of compressed formats DXT1, DXT3 and DXT5.
 */

GLboolean LoadDDS(IMAGE_PTR image, const char *filename)
{
char magic[4];
unsigned int uiFourCC;
unsigned int uiLinearSize;
unsigned int uiSize;
unsigned int uiFlags;
unsigned int nBlock;

int i;

	//Clear the data in image
	if(image->data)
		free(image->data);
	
	image->data=NULL;
	image->components=3;
	image->format=GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	image->height=0;
	image->width=0;
	image->numMipMaps=1;

	// Attempt to open the file
	if(!(pFile = fopen(filename, "rb")))
	{
		printf("<!> File %s cannot be opened\n", filename);
		return GL_FALSE;
	}

	fread(magic, sizeof(magic), 1, pFile);
	fread(&uiSize, sizeof(uiSize), 1, pFile);

	if (	(magic[0] != 'D') || (magic[1] != 'D') ||
		(magic[2] != 'S') || (magic[3] != ' ') ||
		(uiSize != 124) )
	{
		printf("<!> File %s is not a valid DDS file\n", filename);
		return GL_FALSE;
        }

	//Get the surface descriptor
	fread(&uiFlags, sizeof(uiFlags), 1, pFile);
	fread(&image->height,  sizeof(image->height),  1, pFile);
	fread(&image->width,  sizeof(image->width),  1, pFile);
	fread(&uiLinearSize, sizeof(uiLinearSize), 1, pFile);
	fseek(pFile, 4, SEEK_CUR);
	fread(&image->numMipMaps, sizeof(image->numMipMaps), 1, pFile);
	fseek(pFile, 52, SEEK_CUR);
	fread(&uiFourCC, sizeof(uiFourCC), 1, pFile);

	//Check if the FOURCC flag is set
	if( !( uiFlags & DDPF_FOURCC ) )
	{
            printf("<!> The file \"%s\" doesn't appear to be compressed using DXT1, DXT3 or DXT5\n", filename );
            return GL_FALSE;
	}

	//Only load compressed formats DXT1, DXT3 or DXT5
	switch( uiFourCC )
	{
        case FOURCC_DXT1:
        	// DXT1's compression ratio is 8:1
        	image->format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		image->components = 3;
		nBlock = 8;
        	break;

        case FOURCC_DXT3:
		// DXT3's compression ratio is 4:1
		image->format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		image->components = 4;
		nBlock = 16;
            break;

        case FOURCC_DXT5:
		// DXT5's compression ratio is 4:1
		image->format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		image->components = 4;
		nBlock = 16;
		break;

        default:
            printf("<!> The file \"%s\" doesn't appear to be compressed using DXT1, DXT3 or DXT5\n", filename );
            return GL_FALSE;
	}

	//Compute buffer size
	unsigned int uiExpected = ((image->width+3)>>2) * ((image->height+3)>>2) * nBlock;

	if( uiLinearSize != uiExpected )
	{
		printf("<?> The file \"%s\" doesn't seem to contain a valid linear size. Found %d, expected %d\n",
			filename, uiLinearSize, uiExpected );
	}

	iSize = 0;

	if( !( uiFlags & DDSD_MIPMAPCOUNT ) && ( image->numMipMaps > 0 ) )
	{
            printf("<?> The file \"%s\" doesn't seem to contain correct mipmaps\n", filename );
            image->numMipMaps = 0;
	}

        for( i = 0; i < image->numMipMaps; ++i )
        {
		iSize += uiExpected;

		// Quarter of the image size for the next mip-map level
		uiExpected /= 4;
		if (uiExpected < nBlock)
			uiExpected = nBlock;
        }
	
	// Allocate memory and check for success
	if( !(image->data = (GLubyte *)malloc(iSize * sizeof(GLubyte))) )
	{
		printf("<!> Not enough memory to load image in %s\n", filename);
		return GL_FALSE;
	}

	fseek(pFile, 40, SEEK_CUR);
	if (fread(image->data, 1, iSize, pFile) != iSize)
	{
		fclose(pFile);
		free(image->data);
		printf("<!> Unable to read DDS image in %s \n", filename);
		return GL_FALSE;
	}

	fclose(pFile);

	return GL_TRUE;
}

/*
 *	Flip vertically
 *	Flip an image along the vertical axis
 *	Based on the work from: www.paulsprojects.net
 */	
GLboolean FlipVertically(IMAGE_PTR image)
{
GLubyte *tempRow = NULL;
unsigned int rowsToSwap = 0;
unsigned int tempSize = 0;
unsigned int i;

	//don't flip zero or 1 height images
	if( (image->height == 0) || (image->height == 1) )
	{
		printf("<?> FlipVertically: Image too small for flipping\n");
		return GL_TRUE;
	}

	//see how many rows to swap
	if(image->height % 2 == 1)
		rowsToSwap = (image->height-1)/2;
	else
		rowsToSwap = image->height/2;

	//create space for a temporary row
	tempSize = image->width*image->components;
	tempRow = (GLubyte *)malloc(tempSize*sizeof(GLubyte));

	if(!tempRow)
	{
		printf("<!> FlipVertically: Cannot allocate memory for flipping image\n");
		return GL_FALSE;
	}

	//loop through rows to swap
	for(i=0; i<rowsToSwap; ++i)
	{
		//copy row i into temp
		memcpy(tempRow, &image->data[i*tempSize], tempSize);
		//copy row height-i-1 to row i
		memcpy(&image->data[i*tempSize], &image->data[(image->height-i-1)*tempSize], tempSize);
		//copy temp into row height-i-1
		memcpy(&image->data[(image->height-i-1)*tempSize], tempRow, tempSize);
	}

	//free tempRow
	if(tempRow)
		free(tempRow);

	return GL_TRUE;
}

/*
 * Write TGA images
 *
 * Only handles RGB or RGBA types, non-interleaved, uncompressed
 * Origin is always in the lower left-hand corner
 *
 */

GLboolean WriteTGA(IMAGE_PTR image, const char* filename)
{
TGAHEADER tgaHeader;
GLuint i;
unsigned char* temp;

	tgaHeader.identsize = 0;
	tgaHeader.colorMapType = 0;
	tgaHeader.imageType = 2;
	tgaHeader.colorMapStart = 0;
	tgaHeader.colorMapLength = 0;
	tgaHeader.colorMapBits = 0;
	tgaHeader.xstart = 0;
	tgaHeader.ystart = 0;
	tgaHeader.height = image->height;
	tgaHeader.width  = image->width;
	tgaHeader.bits  = image->components * 8;
	if ( image->components == 4 )
		tgaHeader.descriptor = 8;
	else
		tgaHeader.descriptor = 0;

	iSize = image->height * image->width * image->components;

	temp = (unsigned char *)malloc( sizeof(unsigned char) * iSize );

	//Make a local copy and swap the red and blue channels
	if( ( image->components == 3 ) || ( image->components == 4 ) )
	{
		for( i = 0; i < image->height * image->width; i++ )
		{
			temp[image->components*i  ] = image->data[image->components*i+2];
			temp[image->components*i+1] = image->data[image->components*i+1];
			temp[image->components*i+2] = image->data[image->components*i  ];
			if( image->components == 4 )
				temp[image->components*i+3] = image->data[image->components*i+3];
		}
	}

	if( !(pFile = fopen( filename, "wb") ) )
	{
		printf("<!> File %s cannot be created\n", filename);
		free(temp);
		return GL_FALSE;
	}

	if ( fwrite(&tgaHeader, sizeof(TGAHEADER), 1, pFile) != 1 )
	{
		fclose(pFile);
		free(temp);
		printf("<!> Unable to write TGA header in %s\n", filename);
		return GL_FALSE;
	}

	if ( fwrite(temp, iSize, 1, pFile) != 1 )
	{
		fclose(pFile);
		free(temp);
		printf("<!> Unable to write TGA image in %s\n", filename);
		return GL_FALSE;
	}
	
	fclose(pFile);
	free(temp);

	return GL_TRUE;
}

GLboolean LoadTexture( GLint* texture_id, const char* filename, GLboolean clamp, GLfloat AnisoLevel )
{
IMAGE_PTR image;

	glGenTextures( 1, texture_id );

	if( ( image = (IMAGE_PTR)malloc(sizeof(IMAGE)) ) == NULL )
	{
		printf("<!> Unable to allocate memory for image in %s\n", filename);
		return GL_FALSE;
	}

	image->data = NULL;

	if ( !LoadImageFromDisk(image, filename ) )
	{
		printf("<!> Unable to load image from \"%s\"\n", filename);
		return GL_FALSE;
	}
	
	glBindTexture(GL_TEXTURE_2D, *texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

	if( clamp )
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (AnisoLevel)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, AnisoLevel);

	if ( image->numMipMaps > 0 ) //Texture is a DDS one
	{
	int j, nSize, BlockSize, nOffset;
	unsigned int nWidth, nHeight;

		nOffset = 0;
		nWidth = image->width;
		nHeight = image->height;

		if( image->format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT )
			BlockSize = 8;
		else
			BlockSize = 16;

		//Load the mip-map levels
		for( j = 0; j < image->numMipMaps; ++j )
		{
			if( nWidth  == 0 ) nWidth  = 1;
			if( nHeight == 0 ) nHeight = 1;

			nSize = ((nWidth+3)>>2) * ((nHeight+3)>>2) * BlockSize;

			glCompressedTexImage2D( GL_TEXTURE_2D,
					j,
					image->format,
					nWidth,
					nHeight,
					0,
					nSize,
					image->data + nOffset );

			nOffset += nSize;

			// Half the image size for the next mip-map level
			nWidth  = nWidth >> 1;
			nHeight = nHeight >> 1;
		}
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D( GL_TEXTURE_2D,
					0,
					image->components,
					image->width,
					image->height,
					0,
					image->format,
					GL_UNSIGNED_BYTE,
					image->data);
	}

	if( image )
	{
		SAFE_DELETE_ARRAY(image->data);
		free( image );
	}

	return GL_TRUE;
}

/*
 * Save to disk the given texture to a given filename
 * Assumes texture is in RGBA format
 *
 */

GLboolean SaveTexture(GLint tex_id, const char* FileName)
{
IMAGE_PTR image;
GLboolean status;

	if( ( image = (IMAGE_PTR)malloc(sizeof(IMAGE)) ) == NULL )
		return GL_FALSE;

	glBindTexture(GL_TEXTURE_2D, tex_id);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &image->width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &image->height);
	image->components = 4;

	if( ( image->data = (GLubyte *)malloc(image->width * image->height * image->components) ) == NULL )
		return GL_FALSE;

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

	status = WriteTGA( image, FileName );

	SAFE_DELETE_ARRAY(image->data)
	SAFE_DELETE_ARRAY(image)

	return status;
}

GLboolean PrintScreen( const char* FileName )
{
IMAGE_PTR image;
GLboolean status;
GLint ViewPort[4];

	if( ( image = (IMAGE_PTR)malloc(sizeof(IMAGE)) ) == NULL )
		return GL_FALSE;

	glGetIntegerv(GL_VIEWPORT, ViewPort);

	image->width  = ViewPort[2];
	image->height = ViewPort[3];
	image->components = 3;

	if( ( image->data = (GLubyte *)malloc(image->width * image->height * image->components) ) == NULL )
		return GL_FALSE;

	glReadPixels(0, 0, image->width, image->height, GL_RGB, GL_UNSIGNED_BYTE, image->data);

	status = WriteTGA( image, FileName );

	SAFE_DELETE_ARRAY(image->data)
	SAFE_DELETE_ARRAY(image)

	return status;
}
