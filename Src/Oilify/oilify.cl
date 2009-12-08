#define BLOCK_SIZE (9)

int clampxxx(int v, int lo, int hi)
{
	return ((v)<(lo))?(lo):((v)>(hi))?(hi):(v);
}

__kernel void Oilify(__global const uchar4* rgba, __global const uchar* inten, __global uchar4* outRgba, __global float4* outDebug, int w, int h, int radius, float exponent)
{
    // Compute x and y pixel coordinates from group ID and local ID indexes
	const uint x = get_global_id(0);
    const uint y = get_global_id(1);
	const uint idx = y*w + x;
	const uint flipped_idx = (h - y - 1)*w+ x;
	
	if (x >= w || y >= h)
		return;

	const int mask_x1 = clampxxx(x - radius, 0, w-1);
	const int mask_y1 = clampxxx(y - radius, 0, h-1);
	const int mask_x2 = clampxxx(x + radius + 1, 0, w-1);
	const int mask_y2 = clampxxx(y + radius + 1, 0, h-1);

	__private int hist[256];
	__private int4 histRgb[256];
	
	for (int i = 0; i < 256; ++i)
	{
		hist[i] = 0;
		histRgb[i] = (int4)(0);
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	int4 t = (int4)(0);
	int4 merged = (int4)(0);
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
				
				merged.x += rgba[maskOffset].x;
				merged.y += rgba[maskOffset].y;
				merged.z += rgba[maskOffset].z;
				
				/*
				histRgb[0].x += intenVal;
				histRgb[0].y += intenVal;
				histRgb[0].z += intenVal;
				*/

				hist[intenVal]++;

				histRgb[intenVal].x += rgba[maskOffset].x;
				histRgb[intenVal].y += rgba[maskOffset].y;
				histRgb[intenVal].z += rgba[maskOffset].z;
			}
		}
	}
	
	int   hist_max = 1;
	float div = 0.000001f;
	float4 color = (float4)(0);
	color.x = 0;
	color.y = 0;
	color.z = 0;
	color.w = 0;

	for (int i = 0; i < 256; i++)
	{
		hist_max = max(hist_max, hist[i]);
	}
	float maxWeight = 0;
	for (int i = 0; i < 256; i++)
	{
		float ratio = (float)(hist[i]) / (float)(hist_max);
		float weight = pow(ratio, exponent);
		//float weight = pow(0.9f, exponent);
		if (hist[i] > 0)
		{
			for (int bb = 0; bb < 4; bb++)
			{
				//color[b] += weight * (float) histRgb[i][b] / (float) hist[i];
				//color.x = 200.0f;
			}
			
			
			//color[0] = 100.0f;
			//color[1] = 200.0f;
			//color[2] = 250.0f;
			
			
			
			//weight = 1.0f;
			color.x += weight * (float) histRgb[i].x / (float) hist[i];
			color.y += weight * (float) histRgb[i].y / (float) hist[i];
			color.z += weight * (float) histRgb[i].z / (float) hist[i];
			
		}
		div += weight;
		if (maxWeight < weight)
			maxWeight = weight;
	}

	outRgba[flipped_idx].x = (uchar)pown(2.0f, 6);
	if (hist_max > 255)
	{
		outRgba[flipped_idx].y = 255;
		outRgba[flipped_idx].z = 255;
	}
	else
	{
		outRgba[flipped_idx].y = (uchar)hist_max;
		outRgba[flipped_idx].z = (uchar)hist_max;
	}

	/*
	outRgba[flipped_idx].x = (uchar)merged.x;
	outRgba[flipped_idx].y = (uchar)merged.y;
	outRgba[flipped_idx].z = (uchar)merged.z;
	*/
	
	outRgba[flipped_idx].x = (uchar)(color.x / div);
	outRgba[flipped_idx].y = (uchar)(color.y / div);
	outRgba[flipped_idx].z = (uchar)(color.z / div);
	
	outRgba[flipped_idx].w = 255; // alpha remains saturated.

	/*
	outRgba[320*10 + globalX] = (uchar4)(0, 0, 255, 255);
	outRgba[globalY] = (uchar4)(255, 0, 0, 255);
	*/

	//outRgba[flipped_idx] = rgba[idx];
	outDebug[flipped_idx] = color;
	outDebug[flipped_idx].x = pown(0.9f, 8);
	outDebug[flipped_idx].y = div;
}
