#include "basic_shape.h"
#include <vector>
#include "image.h"

using std::vector;

#define CHANGE_NEXT_TRACING_DIRECTION(dir)   \
	switch (dir)             \
{                               \
	case east:                      \
		dir = northeast;           \
		break;                      \
	case southeast:                   \
		dir = northeast;                    \
		break;                   \
	case south:                       \
		dir = southeast;\
		break;\
	case southwest:\
		dir = southeast;\
		break;\
	case west:\
		dir = southwest;\
		break;\
	case northwest:\
		dir = southwest;\
		break;\
	case north:\
		dir = northwest;\
		break;\
	case northeast:\
		dir = northwest;\
		break;\
	default:\
		break;\
}

//找外轮廓  外轮廓标记按顺序排列
vector<vector<Point>> find_out_contour_lable_seq(const Image& img)
{
	vector<vector<Point>> contours;
	uchar lable = 0;
	for (unsigned i = 0; i < img.height; i++)
	{
		uchar* p = img.ptr(i);
		for (unsigned j = 0; j < img.width; j++)
		{
			if (p[j] > lable)
			{
				Point first_encountered_dot(j, i);
				contours.resize(lable + 1);
				contours[lable].push_back(first_encountered_dot);
				int dir = east;
				Point dot = first_encountered_dot;
				do
				{
					Point neighbor = dot.at_direction(dir);
					int n = 0;
					while (*img.ptr(neighbor.y, neighbor.x) == 0 || neighbor.x < 0 || (unsigned)neighbor.x >= img.width || neighbor.y < 0 || (unsigned)neighbor.y >= img.height)
					{
						n++;
						if (n == 8)  goto end_of_tracing;
						dir < 7 ? dir++ : dir = 0;
						neighbor = dot.at_direction(dir);
					}

					contours[lable].push_back(dot = neighbor);

					CHANGE_NEXT_TRACING_DIRECTION(dir)

				} while (dot != first_encountered_dot);
				
				end_of_tracing: lable++;
			}
		}
	}
	return contours;
}

// 标记不一定按顺序
vector<vector<Point>> find_out_contour_label_not_seq(const Image_<unsigned>& img, const int& size)
{
	vector<vector<Point>> contours(size);
	uchar* lable = new uchar[size + 1]();
	lable[0] = 1;
	for (unsigned i = 0; i < img.height; i++)
	{
		unsigned* p = img.ptr(i);
		for (unsigned j = 0; j < img.width; j++)
		{
			if (lable[p[j]] == 0)
			{
				Point first_encountered_dot(j, i);
				contours[p[j] - 1].push_back(first_encountered_dot);
				int dir = east;
				Point dot = first_encountered_dot;
				do
				{
					Point neighbor = dot.at_direction(dir);
					int n = 0;
					while (*img.ptr(neighbor.y, neighbor.x) == 0 || neighbor.x < 0 || (unsigned)neighbor.x >= img.width || neighbor.y < 0 || (unsigned)neighbor.y >= img.height)
					{
						n++;
						if (n == 8)  goto end_of_tracing;
						dir < 7 ? dir++ : dir = 0;
						neighbor = dot.at_direction(dir);
					}

					contours[p[j] - 1].push_back(dot = neighbor);

					CHANGE_NEXT_TRACING_DIRECTION(dir)

				} while (dot != first_encountered_dot);

				end_of_tracing: lable[p[j]] = 1;
			}
		}
	}

	delete[] lable;

	return contours;
}
