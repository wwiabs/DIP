#include "image.h"


//bresenham
void draw_line(Image* img, uchar v, int x0, int y0, int x1, int y1)
{
	if (x0<0 || x0>(int)img->width - 1 || x1<0 || x1>(int)img->width - 1 || y0<0 || y0>(int)img->height - 1 || y1<0 || y1>(int)img->height - 1)
		return;
	int dx = abs(x1 - x0), sx = x0<x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0<y1 ? 1 : -1;
	int err = dx + dy, e2; /* error value e_xy */
	for (;;){ /* loop */
		*img->ptr(y0, x0) = v;
		e2 = 2 * err;
		if (e2 >= dy) { /* e_xy+e_x > 0 */
			if (x0 == x1) break;
			err += dy; x0 += sx;
		}
		if (e2 <= dx) { /* e_xy+e_y < 0 */
			if (y0 == y1) break;
			err += dx; y0 += sy;
		}
	}
}

void draw_circle(Image* img, uchar v, int xm, int ym, int r)
{
	if (xm - r<0 || xm + r>(int)img->width - 1 || ym - r<0 || ym + r>(int)img->height - 1)
		return;
	int x = -r, y = 0, err = 2 - 2 * r; /* bottom left to top right */
	do {
		*img->ptr(ym + y, xm - x) = v; /* I. Quadrant +x +y */
		*img->ptr(ym - x, xm - y) = v; /* II. Quadrant -x +y */
		*img->ptr(ym - y, xm + x) = v; /* III. Quadrant -x -y */
		*img->ptr(ym + x, xm + y) = v; /* IV. Quadrant +x -y */
		r = err;
		if (r <= y) err += ++y * 2 + 1; /* e_xy+e_y < 0 */
		if (r > x || err > y) /* e_xy+e_x > 0 or no 2nd y-step */
			err += ++x * 2 + 1; /* -> x-step now */
	} while (x < 0);
}



//void draw4BMline(uchar* p, int w, int h, Point2d p1, Point2d p2, double a, double b, double c, uchar v)
//{
//	Image img(p, (unsigned)w, (unsigned)h);
//	draw_line(&img, p1, p2, a, b, c, v);
//	memcpy(p, img.data, w*h);
//}
//
//void draw4BMcirlce(uchar* p, int w, int h, Point2d c, double r, uchar v)
//{
//	Image img(p, (unsigned)w, (unsigned)h);
//	draw_circle(&img, c, r, v);
//	memcpy(p, img.data, w*h);
//}

void fill(Image* img, const Rect& rect, uchar v)
{
	if (rect.x < 0 || rect.y < 0 || rect.y + rect.height > (int)img->height || rect.x + rect.width > (int)img->width)
		return;

	for (int r = rect.y; r < rect.y + rect.height; r++)
	{
		memset(img->ptr(r, rect.x), v, rect.width);
	}
}