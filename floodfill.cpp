#include "basic_shape.h"
#include "image.h"
#include <vector>


#define ICV_PUSH( Y, L, R, PREV_L, PREV_R, DIR )  \
	{                                                 \
	tail->y = (ushort)(Y);                        \
	tail->l = (ushort)(L);                        \
	tail->r = (ushort)(R);                        \
	tail->prevl = (ushort)(PREV_L);               \
	tail->prevr = (ushort)(PREV_R);               \
	tail->dir = (short)(DIR);                     \
if (++tail == buffer_end)                    \
	{                                             \
	buffer.resize(buffer.size() * 3 / 2);     \
	tail = &buffer.front() + (tail - head);  \
	head = &buffer.front();                  \
	buffer_end = head + buffer.size();       \
	}                                             \
	}

#define ICV_POP( Y, L, R, PREV_L, PREV_R, DIR )   \
	{                                                 \
	--tail;                                       \
	Y = tail->y;                                  \
	L = tail->l;                                  \
	R = tail->r;                                  \
	PREV_L = tail->prevl;                         \
	PREV_R = tail->prevr;                         \
	DIR = tail->dir;                              \
	}

struct FFillSegment
{
	ushort y;
	ushort l;
	ushort r;
	ushort prevl;
	ushort prevr;
	short dir;
};


void floodFill(Image& image, int _x, int _y, uchar newVal, const int& flags)
{
	if (*image.ptr(_y, _x) == newVal) return /*Rect()*/;
	std::vector<FFillSegment> buffer;
	buffer.resize(std::max(image.width, image.height) * 2);
	uchar* img = image.ptr(_y);
	int i, L, R;
	//int XMin, XMax, YMin = _y, YMax = _y;
	int _8_connectivity = (flags & 255) == 8;
	FFillSegment* buffer_end = &buffer.front() + buffer.size(), *head = &buffer.front(), *tail = &buffer.front();

	L = R /*= XMin = XMax*/ = _x;

	uchar val0 = img[L];
	img[L] = newVal;

	while ((unsigned)++R < image.width && img[R] == val0)
		img[R] = newVal;

	while (--L >= 0 && img[L] == val0)
		img[L] = newVal;

	//XMax = --R;
	//XMin = ++L;

	ICV_PUSH(_y, L, R, R + 1, R, 1);

	while (head != tail)
	{
		int k, YC, PL, PR, dir;
		ICV_POP(YC, L, R, PL, PR, dir);

		int data[][3] =
		{
			{ -dir, L - _8_connectivity, R + _8_connectivity },
			{ dir, L - _8_connectivity, PL - 1 },
			{ dir, PR + 1, R + _8_connectivity }
		};

		//if (XMax < R) XMax = R;
		//if (XMin > L) XMin = L;
		//if (YMax < YC) YMax = YC;
		//if (YMin > YC) YMin = YC;

		for (k = 0; k < 3; k++)
		{
			dir = data[k][0];

			if ((unsigned)(YC + dir) >= image.height)
				continue;

			img = image.ptr(YC + dir);
			int left = data[k][1];
			int right = data[k][2];

			for (i = left; i <= right; i++)
			{
				if ((unsigned)i < image.width && img[i] == val0)
				{
					int j = i;
					img[i] = newVal;
					while (--j >= 0 && img[j] == val0)
						img[j] = newVal;

					while ((unsigned)++i < image.width && img[i] == val0)
						img[i] = newVal;

					ICV_PUSH(YC + dir, j + 1, i - 1, L, R, -dir);
				}
			}
		}
	}
	//return Rect(XMin, YMin, XMax - XMin + 1, YMax - YMin + 1);
}