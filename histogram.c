void cal_hist(unsigned char* p, int w, int h, int stride, unsigned* hist)
{
	int r,c;
	unsigned char *pr;
	for (r = 0; r < h; r++)
	{
		pr = p+r*stride;
		for (c = 0; c < w; c++)
			hist[*pr++]++;
	}
}
