#pragma once

#ifndef CONTOUR_H
#define CONTOUR_H

#include <vector>
//#include "image.h"

using std::vector;

#define GET_NEXT_TRACING_DIR(dir)				\
{												\
	switch (dir)								\
	{											\
	case east:									\
		dir = northeast;						\
		break;									\
	case southeast:								\
		dir = northeast;						\
		break;									\
	case south:									\
		dir = southeast;						\
		break;									\
	case southwest:								\
		dir = southeast;						\
		break;									\
	case west:									\
		dir = southwest;						\
		break;									\
	case northwest:								\
		dir = southwest;						\
		break;									\
	case north:									\
		dir = northwest;						\
		break;									\
	case northeast:								\
		dir = northwest;						\
		break;									\
	default:									\
		break;									\
	}											\
}


//轮廓按顺序排列
//template <class T>
//vector<vector<Point> > find_out_contour_lable_seq(const Image_<T>& img)
//{
//	Rect rect(0, 0, img.width, img.height);
//	vector<vector<Point> > contours;
//	T lable = 0;
//	int n, dir;
//	for (unsigned r = 0; r < img.height; r++)
//	{
//		T* p = img.ptr(r);
//		for (unsigned c = 0; c < img.width; c++)
//		{
//			if (p[c] > lable)
//			{
//				Point first_encountered_dot(c, r);
//				vector<Point> contour;
//				contour.push_back(first_encountered_dot);
//				dir = east;
//				Point dot = first_encountered_dot;
//				Point neighbor = dot.at_direction(dir);
//				n = 0;
//				while (!rect.contains(neighbor) || *img.ptr(neighbor) == 0)
//				{
//					n++;
//					if (n == 8)  goto end_of_tracing;
//					if (dir < 7) dir++;
//					else dir = 0;
//					neighbor = dot.at_direction(dir);
//				}
//				dir = east;
//				do
//				{
//					neighbor = dot.at_direction(dir);
//					while (!rect.contains(neighbor) || *img.ptr(neighbor) == 0)
//					{
//						if (dir < 7) dir++;
//						else dir = 0;
//						neighbor = dot.at_direction(dir);
//					}
//					contour.push_back(dot = neighbor);
//					GET_NEXT_TRACING_DIR(dir)
//				} while (dot != first_encountered_dot);
//				
//			end_of_tracing: lable++;
//				contours.push_back(contour);
//			}
//		}
//	}
//	return contours;
//}
template <class T>
inline static void find_adjacent_pt(const Image_<T>& img, const Rect& rect, const Point& c, Point* neighbor, int* dir)
{
	while (!rect.contains(*neighbor) || *img.ptr(*neighbor) == 0)
	{
		if (*dir < 7) (*dir)++;
		else *dir = 0;
		*neighbor = c.at_direction(*dir);
	}
}

template <class T>
inline static int find_adjacent_pt_chkone(const Image_<T>& img, const Rect& rect, const Point& c, Point* neighbor, int* dir)
{
	int n = 0;
	while (!rect.contains(*neighbor) || *img.ptr(*neighbor) == 0)
	{
		n++;
		if (n == 8)
			return -1;
		if (*dir < 7) (*dir)++;
		else *dir = 0;
		*neighbor = c.at_direction(*dir);
	}
	return 0;
}

//轮廓不一定按顺序排列， img为标记图像
template <class T>
vector<vector<Point> > find_out_contour(const Image_<T>& img, int size)
{
	Rect rect(0, 0, img.width, img.height);
	vector<vector<Point> > contours(size);
	uchar* label = new uchar[size + 1]();
	label[0] = 1;
	int dir, dirchk;
	Point first, dot, neighbor;
	T* p;
	for (unsigned r = 0; r < img.height; r++)
	{
		p = img.ptr(r);
		for (unsigned c = 0; c < img.width; c++)
		{
			if (label[*p] == 0)
			{
				vector<Point> &contour = contours.at(*p - 1);
				first.x = c;
				first.y = r;
				contour.push_back(first);
				dir = east;
				dot = first;
				neighbor = dot.at_direction(dir);
				if (find_adjacent_pt_chkone(img, rect, dot, &neighbor, &dir) == -1)
					goto end_of_tracing;
				dir = east;
				do
				{
					start: neighbor = dot.at_direction(dir);
					find_adjacent_pt(img, rect, dot, &neighbor, &dir);
					dot = neighbor;
					contour.push_back(dot);
					GET_NEXT_TRACING_DIR(dir)
				} while (dot != first);
				dirchk = dir;
				neighbor = dot.at_direction(dirchk);
				find_adjacent_pt(img, rect, dot, &neighbor, &dirchk);
				if (neighbor != contour[1]) goto start;
			end_of_tracing: label[*p] = 1;
			}
			p++;
		}
	}
	delete[] label;
	return contours;
}


//轮廓不一定按顺序排列, 预先做过背景填充、反转以及求BBDT
template <class T>
vector<vector<Point> > find_in_contour(const Image_<T>& img, int size)
{
	Rect rect(0, 0, img.width, img.height);
	vector<vector<Point> > contours(size);
	uchar* label = new uchar[size + 1]();
	label[0] = 1;
	Point first, dot, neighbor;
	int dir;
	T* p;
	for (unsigned r = 0; r < img.height; r++)
	{
		p = img.ptr(r);
		for (unsigned c = 0; c < img.width; c++)
		{
			if (label[*p] == 0)
			{
				vector<Point> &contour = contours.at(*p - 1);
				first.x = c;
				first.y = r - 1;
				contour.push_back(first);
				dir = southeast;
				dot = first;
				do
				{
					neighbor = dot.at_direction(dir);
					find_adjacent_pt(img, rect, dot, &neighbor, &dir);
					if (dir)
						dir--;
					else dir = 7;
					dot = dot.at_direction(dir);
					contour.push_back(dot);
					GET_NEXT_TRACING_DIR(dir)
				} while (dot != first);
				label[*p] = 1;
			}
			p++;
		}
	}
	delete[] label;
	return contours;
}

//轮廓不一定按顺序排列 扩一周
//template <class T>
//vector<vector<Point> > find_out_contour_label_not_seq2(const Image_<T>& img, int size)
//{
//	Image_<T> imglg(img.width + 2, img.height + 2);
//	memset(imglg.ptr(), 0, imglg.width * sizeof(T));
//	memset(imglg.ptr(imglg.height - 1), 0, imglg.width * sizeof(T));
//	for (unsigned i = 0; i < img.height; i++)
//	{
//		*imglg.ptr(i + 1) = 0;
//		memcpy(imglg.ptr(i + 1) + 1, img.ptr(i), img.width * sizeof(T));
//		*imglg.ptr(i + 1, imglg.width - 1) = 0;
//	}
//	
//	vector<vector<Point> > contours(size);
//	uchar* lable = new uchar[size + 1]();
//	lable[0] = 1;
//	int dir, n;
//	T *p;
//	for (unsigned r = 1; r < imglg.height - 1; r++)
//	{
//		p = imglg.ptr(r);
//		for (unsigned c = 1; c < imglg.width - 1; c++)
//		{
//			if (lable[p[c]] == 0)
//			{
//				Point first_encountered_dot(c, r);
//				contours[p[c] - 1].push_back(first_encountered_dot - Point(1, 1));
//				dir = east;
//				Point dot = first_encountered_dot;
//				Point neighbor = dot.at_direction(dir);
//				n = 0;
//				while (*imglg.ptr(neighbor) == 0)
//				{
//					n++;
//					if (n == 8)  goto end_of_tracing;
//					if (dir < 7) dir++;
//					else dir = 0;
//					neighbor = dot.at_direction(dir);
//				}
//				dir = east;
//				do
//				{
//					neighbor = dot.at_direction(dir);
//					while (*imglg.ptr(neighbor) == 0)
//					{
//						if (dir < 7) dir++;
//						else dir = 0;
//						neighbor = dot.at_direction(dir);
//					}
//					dot = neighbor;
//					contours[p[c] - 1].push_back(dot - Point(1, 1));
//					GET_NEXT_TRACING_DIR(dir)
//				} while (dot != first_encountered_dot);
//
//			end_of_tracing: lable[p[c]] = 1;
//			}
//		}
//	}
//
//	delete[] lable;
//
//	return contours;
//}
#endif  