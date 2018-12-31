#include <vector>
using std::vector;

#define GRAY_ROW_COL(p, r, c, bytepr) (p + (r) * (bytepr) + c)
#define LEFT 0x01
#define RIGHT 0x02
//#define OTHER 0x10
// doi:10.1016/j.cag.2005.03.005
static char LM[8][8] = {
	{0,    RIGHT,        RIGHT,        RIGHT,        RIGHT, -1,           -1,           0},
	{0,    RIGHT,        RIGHT,        RIGHT,        RIGHT, LEFT | RIGHT, -1,           0},
	{-1,   RIGHT,        RIGHT,        RIGHT,        RIGHT, LEFT | RIGHT, LEFT | RIGHT, -1},
	{-1,   RIGHT,        RIGHT,        RIGHT,        RIGHT, LEFT | RIGHT, LEFT | RIGHT, LEFT | RIGHT},
	{LEFT, -1,           -1,           0,            0,     LEFT,         LEFT,         LEFT},
	{LEFT, LEFT | RIGHT, -1,           0,            0,     LEFT,         LEFT,         LEFT},
	{LEFT, LEFT | RIGHT, LEFT | RIGHT, -1,           -1,    LEFT,         LEFT,         LEFT},
	{LEFT, LEFT | RIGHT, LEFT | RIGHT, LEFT | RIGHT, -1,    LEFT,         LEFT,         LEFT}
};

typedef struct _CONTOUR_PT_INFO
{
	int x;
	int y;
	char dir;
}CONTOUR_PT_INFO;

static int labeling_contour(unsigned char* p, int w, int h, const vector<CONTOUR_PT_INFO>& pts)
{
	int n = pts.size() - 1;
	char label;
	if (n == 0)
		*GRAY_ROW_COL(p, pts[0].y, pts[0].x, w) = LEFT | RIGHT;
	else
	{
		for (int i = 0; i < n; i++)
		{
			if ((label = LM[pts[i].dir][pts[i+1].dir]) == -1)
				return -1;
			*GRAY_ROW_COL(p, pts[i].y, pts[i].x, w) |= label;
		}
	}
	return 0;
}

int fill_contour(unsigned char* p, int w, int h, const vector<vector<CONTOUR_PT_INFO> >& contours, unsigned char value)
{
	int st;
	memset(p, 0, w*h);
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (labeling_contour(p, w, h, contours[i]) == -1)
			return -1;
	}
	for (int r = 0; r < h; r++)
	{
		unsigned char* ptmp = GRAY_ROW_COL(p, r, 0, w);
		for (int c = 0; c < w; c++)
		{
			if (*ptmp & LEFT)
				st = c;
			if (*ptmp++ & RIGHT)
				memset(GRAY_ROW_COL(p, r, st, w), value, c - st + 1);
		}
	}
	return 0;
}
