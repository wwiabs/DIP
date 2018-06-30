#include "image.h"
#include "fftw3.h"
#include "imgprocess.h"

using namespace std;
#pragma comment(lib,"D:/include/libfftw3-3.lib")

void fftw_forward(const Image& img, Image2d* _mag, Image2d* _arg)
{
	unsigned w(img.width), h(img.height);
	double* spat = (double*)fftw_malloc(sizeof(double)* w * h);
	fftw_complex* freq = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* (w / 2 + 1) * h);

	fftw_plan plan = fftw_plan_dft_r2c_2d(h, w, spat, freq, FFTW_ESTIMATE);

	//Initializing input image
	int k = 0;
	unsigned r, c;
	for (r = 0; r < h; r++)
	{
		uchar *temp = img.ptr(r);
		for (c = 0; c < w; c++)
		{
			spat[k] = *temp++;
			k++;
		}
	}

	fftw_execute(plan);

	k = 0;
	for (r = 0; r < h; r++)
	{
		double *temp_m = _mag->ptr(r);
		double *temp_a = _arg->ptr(r);
		for (c = 0; c < w / 2 + 1; c++)
		{
			cout << freq[k][0] << '\t' << freq[k][1] << endl;
			*temp_m++ = sqrt(freq[k][0] * freq[k][0] + freq[k][1] * freq[k][1]);
			*temp_a++ = atan2(freq[k][1], freq[k][0]);
			//cout << sqrt(freq[k][0] * freq[k][0] + freq[k][1] * freq[k][1]) << endl;
			k++;
		}
	}

	//for (unsigned m = img.width / 2 + 1; m < img.width; m++)
	//{
	//	*_mag.ptr(0, j) = *_mag.ptr(0, img.width - j);
	//	*_arg.ptr(0, j) = -(*_arg.ptr(0, img.width - j));
	//}
	//
	//for (unsigned i = 1; i < img.height; i++)
	//{
	//	for (unsigned j = img.width / 2 + 1; j < img.width; j++)
	//	{
	//		*_mag.ptr(i, j) = *_mag.ptr(img.height - i, img.width - j);
	//		*_arg.ptr(i, j) = -*_arg.ptr(img.height - i, img.width - j);
	//	}
	//}
	//k = 0;
	//for (unsigned i = 1; i < img.height; i++)
	//{
	//	for (unsigned j = 1; j < img.width / 2 + 1; j++)
	//	{
	//		k += (*_mag.ptr(i, j) - *_mag.ptr(img.height - i, img.width - j))*(*_mag.ptr(i, j) - *_mag.ptr(img.height - i, img.width - j));
	//		k += (*_arg.ptr(i, j) + *_arg.ptr(img.height - i, img.width - j))*(*_arg.ptr(i, j) + *_arg.ptr(img.height - i, img.width - j));
	//	}
	//}
	//cout << k<< endl;
	fftw_destroy_plan(plan);
	fftw_free(spat);
	fftw_free(freq);
}

void fftw_backward(const Image2d& _mag, const Image2d& _arg, Image* img)
{
	unsigned w(img->width), h(img->height);
	unsigned N = w * h, r, c;
	double* spat = (double*)fftw_malloc(sizeof(double)* N);
	fftw_complex* freq = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* (w / 2 + 1) * h);

	fftw_plan plan = fftw_plan_dft_c2r_2d(h, w, freq, spat, FFTW_ESTIMATE);
	//Initializing input image
	int k = 0;
	for (r = 0; r < h; r++)
	{
		double* temp_m = _mag.ptr(r);
		double* temp_a = _arg.ptr(r);
		for (c = 0; c < w / 2 + 1; c++)
		{

			freq[k][0] = (*temp_m) * cos(*temp_a);
			freq[k][1] = (*temp_m++) * sin(*temp_a++);
			//std::cout << spat[k][0] << std::endl;
			k++;
		}
	}
	//clock_t t = clock();
	fftw_execute(plan);
	//cout <<"c2r_2d"<< clock() - t << endl;
	k = 0;
	for (r = 0; r < h; r++)
	{
		uchar *temp = img->ptr(r);
		for (c = 0; c < w; c++)
		{
			*temp++ =uchar(spat[k] / N);
			k++;
		}
	}
	fftw_destroy_plan(plan);
	fftw_free(spat);
	fftw_free(freq);
}



enum THRESHOLD_TYPE
{
	THRESHOLD_BINARY,		// dst(x,y) = src(x, y) > thresh ? maxval : 0
	//THRESHOLD_BINARY_INV,	// dst(x,y) = src(x, y) > thresh ? 0 : maxval
	//THRESHOLD_TRUNC,		// dst(x,y) = src(x, y) > thresh ? thres : src(x, y)
	THRESHOLD_TOZERO,		// dst(x,y) = src(x, y) > thresh ? src(x, y) : 0
	//THRESHOLD_TOZERO_INV	// dst(x,y) = src(x, y) > thresh ? 0 : src(x, y)
};

void threshold(Image* img, int type, uchar th, uchar v)
{
	unsigned r, c;
	uchar* p;
	switch (type)
	{
	case THRESHOLD_BINARY:
	{
							 for (r = 0; r < img->height; r++)
							 {
								 p = img->ptr(r);
								 for (c = 0; c < img->width; c++)
								 {
									 if (*p>th)
										 *p = v;
									 else *p = 0;
									 p++;
								 }
							 }
							 break;	
	}
	case THRESHOLD_TOZERO:
	{
							 for (r = 0; r < img->height; r++)
							 {
								 p = img->ptr(r);
								 for (c = 0; c < img->width; c++)
								 {
									 if (*p <= th)
										 *p = 0;
									 p++;
								 }
							 }
							 break;
	}
	default:
		break;
	}
}

int mean_filter(uchar* ps, unsigned w, unsigned h, unsigned ss, unsigned mw, unsigned mh, uchar* pd, unsigned ds)
{
	unsigned r, c, s, *psum, *psumt, *psumy, *pt5, ml(mw*mh), *pt7, *pt8, sw(w - mw + 1);
	uchar* pt1, *pt2;

	for (r = 0; r < mh / 2; r++)
		ZERO(ROW_HEAD(pd, r, ds), w);

	for (; r < h - mh / 2; r++)
	{
		ZERO(ROW_HEAD(pd, r, ds), mw / 2);
		ZERO(GRAY_ROW_COL(pd, r, w - mw / 2, ds), mw / 2);
	}
	for (; r < h; r++)
		ZERO(ROW_HEAD(pd, r, ds), w);

	psum = new unsigned[sw*(mh + 1)];

	for (r = 0; r < mh; r++)
	{
		pt1 = ROW_HEAD(ps, r, ss);
		pt2 = pt1 + mw;
		pt5 = ROW_HEAD(psum, r%mh, sw);
		s = 0;
		ADD(pt1, mw, s);
		for (c = mw / 2; c < w - mw / 2 - 1; c++)
		{
			*pt5++ = s;
			s += *pt2++ - *pt1++;
		}
		*pt5 = s;
	}

	pt1 = GRAY_ROW_COL(pd, mh / 2, mw / 2, ds);
	pt5 = psum;
	pt7 = psumt = ROW_HEAD(psum, mh, sw);
	for (c = 0; c < sw; c++)
	{
		s = 0;
		ADD_STEP(pt5, sw, mh, s);
		*pt7++ = s - *pt5++;
		*pt1++ = (s + ml / 2) / ml;
	}
	for (; r < h; r++)
	{
		pt1 = ROW_HEAD(ps, r, ss);
		pt2 = pt1 + mw;
		pt5 = psumy = ROW_HEAD(psum, r%mh, sw);
		s = 0;
		ADD(pt1, mw, s);
		for (c = mw / 2; c < w - mw / 2 - 1; c++)
		{
			*pt5++ = s;
			s += *pt2++ - *pt1++;
		}
		*pt5 = s;

		pt5 = psumy;
		pt7 = psumt;
		pt8 = ROW_HEAD(psum, (r + 1)%mh, sw);  //oldest
		pt1 = GRAY_ROW_COL(pd, r - mh / 2, mw / 2, ds);
		for (c = 0; c < sw; c++)
		{
			*pt7 += *pt5++;
			*pt1++ = (*pt7 + ml / 2) / ml;
			*pt7++ -= *pt8++;
		}
	}
	delete[] psum;
	return 0;
}