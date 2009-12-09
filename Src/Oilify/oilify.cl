int clamp_int(int v, int lo, int hi)
{
	return ((v)<(lo))?(lo):((v)>(hi))?(hi):(v);
}

__kernel void Oilify(__global const uchar4* rgba, __global const uchar* inten,
	__global uchar4* outRgba, int w, int h, int radius, int exponent, int bFlipY)
{
	const uint idx = get_global_id(0);
	const uint x = idx % w;
    const uint y = idx / w;
	
	if (x >= w || y >= h)
		return;

	const int mask_x1 = clamp_int(x - radius, 0, w-1);
	const int mask_y1 = clamp_int(y - radius, 0, h-1);
	const int mask_x2 = clamp_int(x + radius + 1, 0, w-1);
	const int mask_y2 = clamp_int(y + radius + 1, 0, h-1);

	int hist[256];
	int4 histRgb[256];
	for (int i = 0; i < 256; ++i)
	{
		hist[i] = 0;
		histRgb[i] = (int4)(0);
	}

	for (int mask_y = mask_y1; mask_y < mask_y2; ++mask_y)
	{
		const int dy_squared = (mask_y - y)*(mask_y - y);
		for (int mask_x = mask_x1; mask_x < mask_x2; ++mask_x)
		{
			const int dx_squared = (mask_x - x)*(mask_x - x);
			if ((dx_squared + dy_squared) <= (radius*radius))
			{
				const int maskOffset = w*mask_y + mask_x;
				unsigned char intenVal = inten[maskOffset];
				hist[intenVal]++;
				histRgb[intenVal].x += rgba[maskOffset].x;
				histRgb[intenVal].y += rgba[maskOffset].y;
				histRgb[intenVal].z += rgba[maskOffset].z;
			}
		}
	}
	
	int   hist_max = 1;
	float div = 1.0e-6f;
	float4 color = (float4)(0);

	for (int i = 0; i < 256; i++)
	{
		hist_max = max(hist_max, hist[i]);
	}
	for (int i = 0; i < 256; i++)
	{
		float ratio = (float)(hist[i]) / (float)(hist_max);
		float weight = pown(ratio, (int)exponent);
		if (hist[i] > 0)
		{
			color.x += weight * (float) histRgb[i].x / (float) hist[i];
			color.y += weight * (float) histRgb[i].y / (float) hist[i];
			color.z += weight * (float) histRgb[i].z / (float) hist[i];
		}
		div += weight;
	}

	int dest_idx = bFlipY ? ((h - y - 1)*w+ x) : idx;
	outRgba[dest_idx].x = (uchar)(color.x / div);
	outRgba[dest_idx].y = (uchar)(color.y / div);
	outRgba[dest_idx].z = (uchar)(color.z / div);
	outRgba[dest_idx].w = 255; // alpha remains saturated.
}
