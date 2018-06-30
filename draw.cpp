#include "image.h"

//bresenham
void draw_line(Image* img, uchar v, int x0, int y0, int x1, int y1)
{
	if (x0<0 || x0>=(int)img->width || x1<0 || x1>=(int)img->width || y0<0 || y0>=(int)img->height || y1<0 || y1>=(int)img->height)
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
	if (xm < r || xm + r>=(int)img->width || ym < r || ym + r>=(int)img->height)
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

//  p1p2 cross p3p4  p1(x1,y1), p2(x2,y2), p3(x3, y3), p4(x4, y4) 
inline void crosspt(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double *x, double *y)
{
	double A = y2 - y1;
	double B = x1 - x2;
	double C = x1*y2 - x2*y1;
	double D = y4 - y3;
	double E = x3 - x4;
	double F = x3*y4 - x4*y3;
	double G = A*E - B*D;
	*x = (C*E - B*F) / G;
	*y = (A*F - C*D) / G;
}