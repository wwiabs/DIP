#include "image.h"
#include "fftw3.h"

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
			*temp++ = spat[k] / N;
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