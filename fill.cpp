#include "image.h"
#include <vector>
using std::vector;
using std::pair;

typedef unsigned short ushort;

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


void floodFill(Image* image, int _x, int _y, uchar newVal, uchar flags)
{
	if (*image->ptr(_y, _x) == newVal) return /*Rect()*/;
	std::vector<FFillSegment> buffer;
	buffer.resize(std::max(image->width, image->height) * 2);
	uchar* img = image->ptr(_y);
	int i, L, R;
	//int XMin, XMax, YMin = _y, YMax = _y;
	int _8_connectivity = (flags & 255) == 8;
	FFillSegment* buffer_end = &buffer.front() + buffer.size(), *head = &buffer.front(), *tail = &buffer.front();

	L = R /*= XMin = XMax*/ = _x;

	uchar val0 = img[L];
	img[L] = newVal;

	while ((unsigned)++R < image->width && img[R] == val0)
		img[R] = newVal;

	while (--L >= 0 && img[L] == val0)
		img[L] = newVal;

	/*XMax =*/ --R;
	/*XMin =*/ ++L;

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

			if ((unsigned)(YC + dir) >= image->height)
				continue;

			img = image->ptr(YC + dir);
			int left = data[k][1];
			int right = data[k][2];

			for (i = left; i <= right; i++)
			{
				if ((unsigned)i < image->width && img[i] == val0)
				{
					int j = i;
					img[i] = newVal;
					while (--j >= 0 && img[j] == val0)
						img[j] = newVal;

					while ((unsigned)++i < image->width && img[i] == val0)
						img[i] = newVal;

					ICV_PUSH(YC + dir, j + 1, i - 1, L, R, -dir);
				}
			}
		}
	}
	//return Rect(XMin, YMin, XMax - XMin + 1, YMax - YMin + 1);
}

#define LEFT 0x01
#define RIGHT 0x02
//#define OTHER 0x10

// clockwise
static inline int get_edge_label(int pre, int next, uchar* label)
{
	switch (next)
	{
	case east:
		if (pre == northeast || pre == north || pre == northwest || pre == west)
			*label |= LEFT;
		break;
	case southeast:
	{
		if (pre == southwest || pre == south || pre == southeast || pre == east)
			*label |= RIGHT;
		else if (pre == northeast || pre == north || pre == northwest)
			*label |= LEFT + RIGHT;
		else return -1;
		break;
	}
	case south:
	{
		if (pre == southwest || pre == south || pre == southeast || pre == east)
			*label |= RIGHT;
		else if (pre == northeast || pre == north)
			*label |= LEFT + RIGHT;
		else return -1;
		break;
	}
	case southwest:
	{
		if (pre == southwest || pre == south || pre == southeast || pre == east)
			*label |= RIGHT;
		else if (pre == northeast)
			*label |= LEFT + RIGHT;
		break;
	}
	case west:
		if (pre == southwest || pre == south || pre == southeast || pre == east)
			*label |= RIGHT;
		break;
	case northwest:
	{
		if (pre == northeast || pre == north || pre == northwest || pre == west)
			*label |= LEFT;
		else if (pre == southwest || pre == south || pre == southeast)
			*label |= LEFT + RIGHT;
		else return -1;
		break;
	}
	case north:
	{
		if (pre == northeast || pre == north || pre == northwest || pre == west)
			*label |= LEFT;
		else if (pre == southwest || pre == south)
			*label |= LEFT + RIGHT;
		else return -1;
		break;
	}
	case northeast:
	{
		if (pre == northeast || pre == north || pre == northwest || pre == west)
			*label |= LEFT;
		else if (pre == southwest)
			*label |= LEFT + RIGHT;
		break;
	}
	default:
		return -1;
	}
	return 0;
}

static inline int get_chaincode(const Point& pre, const Point& next)
{
	if (next.x - pre.x == 1 && next.y - pre.y == 0)
		return east;
	else if (next.x - pre.x == 1 && next.y - pre.y == 1)
		return southeast;
	else if (next.x - pre.x == 0 && next.y - pre.y == 1)
		return south;
	else if (next.x - pre.x == -1 && next.y - pre.y == 1)
		return southwest;
	else if (next.x - pre.x == -1 && next.y - pre.y == 0)
		return west;
	else if (next.x - pre.x == -1 && next.y - pre.y == -1)
		return northwest;
	else if (next.x - pre.x == 0 && next.y - pre.y == -1)
		return north;
	else if (next.x - pre.x == 1 && next.y - pre.y == -1)
		return northeast;
	else return -1;
}

// doi:10.1016/j.cag.2005.03.005
static int edge_labeling_contour(Image* img, const vector<Point>& pts)
{
	int pre, next;
	if (pts.size() == 1)
	{
		*img->ptr(pts[0]) = LEFT + RIGHT;
		return 0;
	}
	else
	{
		pre = get_chaincode(pts[0], pts[1]);
		for (size_t i = 1; i < pts.size() - 1; i++)
		{
			next = get_chaincode(pts[i], pts[i + 1]);
			if (get_edge_label(pre, next, img->ptr(pts[i])) == -1)
				return -1;
			pre = next;
		}
		next = get_chaincode(pts[0], pts[1]);
		if (get_edge_label(pre, next, img->ptr(pts[0])) == -1)
			return -1;
	}
	return 0;
}

int fill_contour(Image* img, const vector<vector<Point> >& pts, uchar value)
{
	int st;
	uchar* p;
	for (size_t i = 0; i < pts.size(); i++)
	{
		if (edge_labeling_contour(img, pts[i]) == -1)
			return -1;
	}
	for (unsigned r = 0; r < img->height; r++)
	{
		p = img->ptr(r);
		for (unsigned c = 0; c < img->width; c++)
		{
			if (*p & LEFT)
				st = c;
			if (*p & RIGHT)
			{
				memset(img->ptr(r, st), value, c - st + 1);
			}
			p++;
		}
	}
	return 0;
}