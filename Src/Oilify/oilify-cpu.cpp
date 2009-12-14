#include "oilify.h"

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

int main_cpu(int argc, const char* argv[])
{
	// Default parameters
	int radius = 4;
	float exponent = 8.0f;

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

	BwWin32Timer timer;
	timer.start();

	std::vector<unsigned char> srcInten(w * h);
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

	std::vector<unsigned char> outData(w*h*3); // output image consists of RGB channels

#pragma omp parallel for schedule(dynamic)
	for (int y = 0; y < h; ++y)
	{
		//printf("row %d / %d...\n", y+1, h);
		for (int x = 0; x < w; ++x)
		{
			int hist[HISTSIZE];
			int histRgb[4][HISTSIZE];
			memset(hist, 0, sizeof(hist));
			memset(histRgb, 0, sizeof(histRgb));

			const int mask_x1 = CLAMP((x - radius), 0, w-1);
			const int mask_y1 = CLAMP((y - radius), 0, h-1);
			const int mask_x2 = CLAMP((x + radius + 1), 0, w-1);
			const int mask_y2 = CLAMP((y + radius + 1), 0, h-1);

			for (int mask_y = mask_y1; mask_y < mask_y2; ++mask_y)
			{
				const int dy_squared = (mask_y - y)*(mask_y - y);
				for (int mask_x = mask_x1; mask_x < mask_x2; ++mask_x)
				{
					const int dx_squared = (mask_x - x)*(mask_x - x);
					if ((dx_squared + dy_squared) > (radius*radius))
						continue;

					const int maskOffset = w*mask_y + mask_x;
					unsigned char inten = srcInten[maskOffset];
					++hist[inten];
					for (int b = 0; b < 3; ++b)
						histRgb[b][inten] += rgbData[bpp*maskOffset + b];
				}
			}
			unsigned char dest[4];
			weighted_average_color(hist, histRgb, exponent, dest, bpp);


			const int yFlippedPixelOffset = w*(h-1-y) + x;
			outData[3*yFlippedPixelOffset + 0] = dest[0];
			outData[3*yFlippedPixelOffset + 1] = dest[1];
			outData[3*yFlippedPixelOffset + 2] = dest[2];

		}
	}
	printf("Process time: %lf ms\n", timer.getTicks());
	delete texture;

	ILuint ImageName;
	ilGenImages(1, &ImageName);
	ilBindImage(ImageName);
	ilTexImage(w, h, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, &outData[0]);
	ilEnable(IL_FILE_OVERWRITE);
	ilSave(IL_PNG, outputFileName);
	ilDeleteImages(1, &ImageName);

	if (ArnCleanupImageLibrary() < 0)
	{
		printf("Image library cleanup failed.\n");
	}
	return 0;
}
