//#include <immintrin.h>
#include <string.h>
#include <stdlib.h>
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#define GRAY_ROW_COL(p, r, c, bytepr) (p + (r) * (bytepr) + c)


#define TL_MIR(x)      (((x) < 0) ? (-(x) - 1) : (x))
#define BR_MIR(x, w)   (((x) >= (w)) ? (2*(w) - 1 - (x)) : (x))

static inline void row_sum(uint8_t* psrc, uint16_t* psum, int w, int maskwidth)
{
	int c, i, maskr = maskwidth / 2;
	memset(psum, 0, sizeof(uint16_t)*w);
	for (c = 0; c <= maskr; c++)
	{
		for (i = -maskr; i <= maskr; i++)
			psum[c] += psrc[TL_MIR(c + i)];
	}
	int right = maskwidth;
	int left = 0;
	for (; c < w - maskr; c++)
		psum[c] = psum[c - 1] + psrc[right++] - psrc[left++];
	for (; c < w; c++)
	{
		for (i = -maskr; i <= maskr; i++)
			psum[c] += psrc[BR_MIR(c + i, w)];
	}
}

//add: O(w*h*4)
int mean_filter(uint8_t* psrc, int w, int h, int stride, uint8_t* pdst, int maskwidth, int maskheight)
{
	if (maskwidth > 257)
		return -1;
	int i, r, c, masksize = maskwidth * maskheight, avg_tbl_size = 0xff * masksize + 1;
	uint8_t* pavgtbl = (uint8_t*)malloc(avg_tbl_size + sizeof(uint16_t)*w*maskheight + sizeof(int)*w);
	uint16_t *pitmp = (uint16_t *)(pavgtbl + avg_tbl_size);
	memset(pavgtbl, 0, masksize);

	for (r = masksize / 2 + 1, c = 0, i = 1; r < avg_tbl_size; r++, c++)
	{
		if (c == masksize)
		{
			i++;
			c = 0;
		}
		pavgtbl[r] = i;
	}
	uint8_t *pdstr;

	for (r = 0; r < maskheight; r++)
		row_sum(GRAY_ROW_COL(psrc, r, 0, stride), GRAY_ROW_COL(pitmp, r, 0, w), w, maskwidth);

	int *total_sum = (int *)(pitmp + w*maskheight);
	for (r = 0; r <= maskheight / 2; r++)
	{
		pdstr = GRAY_ROW_COL(pdst, r, 0, w);
		memset(total_sum, 0, sizeof(int)*w);
		for (c = 0; c < w; c++)
		{
			for (i = -maskwidth / 2; i <= maskwidth / 2; i++)
				total_sum[c] += *GRAY_ROW_COL(pitmp, TL_MIR(r + i), c, w);
			pdstr[c] = pavgtbl[total_sum[c]];
		}
	}
	int bottom = maskheight;

	for (; r < h - maskheight / 2; r++)
	{
		pdstr = GRAY_ROW_COL(pdst, r, 0, w);
		uint16_t* replacer = GRAY_ROW_COL(pitmp, bottom % maskheight, 0, w);
		for (c = 0; c < w; c++)
			total_sum[c] -= replacer[c];
		row_sum(GRAY_ROW_COL(psrc, bottom++, 0, stride), replacer, w, maskwidth);
		for (c = 0; c < w; c++)
			pdstr[c] = pavgtbl[total_sum[c] += replacer[c]];
	}
	for (; r < h; r++)
	{
		pdstr = GRAY_ROW_COL(pdst, r, 0, w);
		memset(total_sum, 0, sizeof(int)*w);
		for (c = 0; c < w; c++)
		{
			for (i = -maskwidth / 2; i <= maskwidth / 2; i++)
				total_sum[c] += *GRAY_ROW_COL(pitmp, BR_MIR(r + i, h) % maskheight, c, w);
			pdstr[c] = pavgtbl[total_sum[c]];
		}
	}
	free(pavgtbl);
	return 0;
}