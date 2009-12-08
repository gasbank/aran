int clampxxx(int v, int lo, int hi)
{
	return ((v)<(lo))?(lo):((v)>(hi))?(hi):(v);
}

__kernel void Oilify(__global const uchar4* rgba, __global const uchar* inten, __global uchar4* outRgba, int w, int h, int radius, float exponent)
{
    // get index into global data array
    int iGID = get_global_id(0);
	// image pixel position
	int x = iGID % w;
	int y = iGID / w;
	
    // bound check (equivalent to the limit on a 'for' loop for standard/serial C code
    if (x < 0 || x >= w || y < 0 || y >= h)
        return;
	
	barrier(CLK_LOCAL_MEM_FENCE);

	int hist[256];
	int4 histRgb[256];
	
	for (int i = 0; i < 256; ++i)
	{
		hist[i] = 0;
		histRgb[i].x = 0;
		histRgb[i].y = 0;
		histRgb[i].z = 0;
		histRgb[i].w = 0;
	}
	
	barrier(CLK_LOCAL_MEM_FENCE);

	uchar mask_area = 0;
	const int mask_x1 = clampxxx(x - radius, 0, w-1);
	const int mask_y1 = clampxxx(y - radius, 0, h-1);
	const int mask_x2 = clampxxx(x + radius + 1, 0, w-1);
	const int mask_y2 = clampxxx(y + radius + 1, 0, h-1);

	barrier(CLK_LOCAL_MEM_FENCE);

	for (int mask_y = mask_y1; mask_y < mask_y2; ++mask_y)
	{
		const int dy_squared = (mask_y - y)*(mask_y - y);
		for (int mask_x = mask_x1; mask_x < mask_x2; ++mask_x)
		{
			const int dx_squared = (mask_x - x)*(mask_x - x);
			if ((dx_squared + dy_squared) <= (radius*radius))
			{
				//mask_area = atom_add(&mask_area, 1);

				const int maskOffset = w*mask_y + mask_x;
				
				unsigned char intenVal = inten[maskOffset];

				barrier(CLK_LOCAL_MEM_FENCE);

				hist[intenVal]++;

				barrier(CLK_LOCAL_MEM_FENCE);

				histRgb[intenVal].x += rgba[maskOffset].x;
				histRgb[intenVal].y += rgba[maskOffset].y;
				histRgb[intenVal].z += rgba[maskOffset].z;

				barrier(CLK_LOCAL_MEM_FENCE);
			}
		}
	}

	int   hist_max = 1;
	int   exponent_int = 0;
	float div = 1.0e-6f;
	
	float4 color;

	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	color[3] = 0;
	//color = (float4)(0);

	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	for (int i = 0; i < 256; i++)
	{
		hist_max = max(hist_max, hist[i]);
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	
	for (int i = 0; i < 256; i++)
	{
		float ratio = (float)hist[i] / (float)hist_max;
		float weight;

		weight = pown(ratio, (int)exponent);

		barrier(CLK_LOCAL_MEM_FENCE);
		
		if (hist[i] > 0)
		{
			for (int b = 0; b < 4; b++)
			{
				color[b] += weight * (float) histRgb[i][b] / (float) hist[i];
				barrier(CLK_LOCAL_MEM_FENCE);
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
		div += weight;
		
	}

	outRgba[iGID][0] = (uchar) clampxxx(color[0] / div, 0, 255);
	outRgba[iGID][1] = (uchar) clampxxx(color[1] / div, 0, 255);
	outRgba[iGID][2] = (uchar) clampxxx(color[2] / div, 0, 255);
	outRgba[iGID][3] = 255;

	

	/*
	outRgba[iGID][0] = mask_area;
	outRgba[iGID][1] = mask_area;
	outRgba[iGID][2] = mask_area;
	outRgba[iGID][3] = 255;
	*/

	// Original image out
	//outRgba[iGID] = rgba[iGID];

	// Intensity image out
	/*
	outRgba[iGID][0] = inten[iGID];
	outRgba[iGID][1] = inten[iGID];
	outRgba[iGID][2] = inten[iGID];
	outRgba[iGID][3] = 255;
	*/

}
