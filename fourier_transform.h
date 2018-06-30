#pragma once 

#ifndef FOURIER_TRANSFORM_H_
#define FOURIER_TRANSFORM_H_

#include <vector>
#include <complex>
//#include <iostream>
#include "image.h"

//#include <ctime>

using std::vector;
using std::complex;

#ifndef PI
#define PI 3.1416
#endif

template <class T>
vector<complex<T> > DFT_1D(const vector<T>& _in)
{
	int N = (int)_in.size();
	vector<complex<T> > result(N);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			double thumb = -2 * PI / N * i * j;
			result[i] += _in[j] * complex<T>(cos(thumb), sin(thumb));
		}
	}
	return result;
}

template <class T>
vector<complex<T> > IDFT_1D(const vector<complex<T> >& _in)
{
	int N = (int)_in.size();
	vector<complex<T> > result(N);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			double thumb = 2 * PI / N * i * j;
			result[i] += _in[j] * complex<T>(cos(thumb), sin(thumb));
		}
		result[i] /= N;
	}
	return result;
}

template <class T>
int prominent_harmonic_component(const vector<T>& sample)
{
	int N = (int)sample.size();

	int M;
	if ((N & 1) == 1)
		M = (N + 1) / 2;
	else
		M = N / 2 + 1;

	complex<double>* ft = new complex<double>[M]();
	double* magnitude = new double[M];
	*magnitude = 0.0;
	int k = 0;
	for (int i = 1; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			double thumb = -2 * PI / N * i * j;
			ft[i] +=/* (double)*/sample[j] * complex<double>(cos(thumb), sin(thumb));
		}
		magnitude[i] = norm(ft[i]);
		//std::cout << magnitude[i] << std::endl;
		if (magnitude[i] > *magnitude)
		{
			k = i;
			*magnitude = magnitude[i];
		}
	}

	delete[] magnitude;
	delete[] ft;

	return k;
}

//template <class T>
//void DFT_2D(const Image& img, Image_<T>& _mag, Image_<T>& _arg)
//{
//	for (unsigned i = 0; i < img.height; i++)
//	{
//		T* temp_mag = _mag.ptr(i);
//		T* temp_arg = _arg.ptr(i);
//		for (unsigned j = 0; j < img.width; j++)
//		{
//			complex<T> ft;
//			for (unsigned m = 0; m < img.height; m++)
//			{
//				uchar* temp = img.ptr(m);
//				for (unsigned n = 0; n < img.width; n++)
//				{
//					double thumb = -2 * PI * ((double)j * n / img.width + (double)i * m / img.height);
//					ft += ((T)*temp++) * complex<T>(cos(thumb), sin(thumb));
//				}
//			}
//			*temp_mag++ = /*abs(ft)*/ft.real();
//			*temp_arg++ = /*arg(ft)*/ft.imag();
//		}
//	}
//}



//template <class T>
//void split2mag_arg(const Image_<T>& img, Image_<T>& mag, Image_<T>& arg)
//{
//	for (unsigned i = 0; i < img.height; i++)
//	{
//		T* img_ = img.ptr(i);
//		T* mag_ = mag.ptr(i);
//		T* arg_ = arg.ptr(i);
//		for (unsigned j = 0; j < img.width; j++)
//		{
//			*mag_++ = *img_++;
//			*arg_++ = *img_++;
//		}
//	}
//}

template <class T>
double period(const vector<T>& sample)
{
	return (double)sample.size() / prominent_harmonic_component(sample);
}

template <class T>
void move2center(Image_<T>* img)
{
	//对于偶数，中心选偏右或下
	int x = img->width / 2;
	int n = img->width - x;
	int y = img->height / 2;
	
	T* tmp = new T[img->width];

	if ((img->height & 1) == 0)
	{
		for (int i = 0; i < y; i++)
		{
			memcpy(tmp, img->ptr(i), sizeof(T)*img->width);
			memcpy(img->ptr(i), img->ptr(i + y, n), sizeof(T)*x);
			memcpy(img->ptr(i, x), img->ptr(i + y), sizeof(T)*n);
			memcpy(img->ptr(i + y), tmp + n, sizeof(T)*x);
			memcpy(img->ptr(i + y, x), tmp, sizeof(T)*n);
		}	
	}
	else
	{
		memcpy(tmp, img->ptr(y), sizeof(T)*img->width);
		for (int i = 0; i < y; i++)
		{
			memcpy(img->ptr(i + y), img->ptr(i, n), sizeof(T)*x);
			memcpy(img->ptr(i + y, x), img->ptr(i), sizeof(T)*n);
			memcpy(img->ptr(i), img->ptr(i + y + 1, n), sizeof(T)*x);
			memcpy(img->ptr(i, x), img->ptr(i + y + 1), sizeof(T)*n);
		}
		memcpy(img->ptr(img->height - 1), tmp + n, sizeof(T)*x);
		memcpy(img->ptr(img->height - 1, x), tmp, sizeof(T)*n);
	}
	delete[] tmp;
}

template <class T>
void moveback(Image_<T>* img)
{
	//对于偶数，中心选偏右或下
	int x = img->width / 2;
	int n = img->width - x;
	int y = img->height / 2;

	T* tmp = new T[img->width];

	if ((img->height & 1) == 0)
	{
		for (int i = 0; i < y; i++)
		{
			memcpy(tmp, img->ptr(i), sizeof(T)*img->width);
			memcpy(img->ptr(i), img->ptr(i + y, x), sizeof(T)*n);
			memcpy(img->ptr(i, n), img->ptr(i + y), sizeof(T)*x);
			memcpy(img->ptr(i + y), tmp + x, sizeof(T)*n);
			memcpy(img->ptr(i + y, n), tmp, sizeof(T)*x);
		}
	}
	else
	{
		memcpy(tmp, img->ptr(y), sizeof(T)*img->width);
		for (int i = y; i > 0; i--)
		{
			memcpy(img->ptr(i), img->ptr(i + y, x), sizeof(T)*n);
			memcpy(img->ptr(i, n), img->ptr(i + y), sizeof(T)*x);
			memcpy(img->ptr(i + y), img->ptr(i - 1, x), sizeof(T)*n);
			memcpy(img->ptr(i + y, n), img->ptr(i - 1), sizeof(T)*x);
		}
		memcpy(img->ptr(), tmp + x, sizeof(T)*n);
		memcpy(img->ptr(0, n), tmp, sizeof(T)*x);
	}
	delete[] tmp;
}

template <class T>
void shrink256(const Image_<T>& src, Image* show)
{
	for (unsigned i = 0; i < src.height; i++)
	{
		T* tmp = src.ptr(i);
		uchar* tmp_show = show->ptr(i);
		for (unsigned j = 0; j < src.width; j++)
		{
			int k = (int)10 * log(1 + *tmp++);

			if (k>255)
				k = 255;
			*tmp_show++ = k;
		}
	}
}


#endif