#include <stdlib.h>

#define GRAY_ROW_COL(p, r, c, bytepr) (p + (r) * (bytepr) + c)
#define OUT_IMG(x, y, w, h) ((x)<0 || (y)<0 || (x)>=(w) || (y)>=(h))

//bresenham/
void draw_line(unsigned char* p, int w, int h, unsigned char v, int x0, int y0, int x1, int y1)
{
	if (OUT_IMG(x0, y0, w, h) || OUT_IMG(x1, y1, w, h))
		return;
	int dx = abs(x1 - x0), sx = x0<x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0<y1 ? 1 : -1;
	int err = dx + dy, e2; /* error value e_xy */
	for (;;){ /* loop */
		*GRAY_ROW_COL(p, y0, x0, w) = v;
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

void draw_circle(unsigned char* p, int w, int h, unsigned char v, int xm, int ym, int r)
{
	if (xm < r || xm + r >= w || ym < r || ym + r >= h)
		return;
	int x = -r, y = 0, err = 2 - 2 * r; /* bottom left to top right */
	do {
		*GRAY_ROW_COL(p, ym + y, xm - x, w) = v; /* I. Quadrant +x +y */
		*GRAY_ROW_COL(p, ym - x, xm - y, w) = v; /* II. Quadrant -x +y */
		*GRAY_ROW_COL(p, ym - y, xm + x, w) = v; /* III. Quadrant -x -y */
		*GRAY_ROW_COL(p, ym + x, xm + y, w) = v; /* IV. Quadrant +x -y */
		r = err;
		if (r <= y) err += ++y * 2 + 1; /* e_xy+e_y < 0 */
		if (r > x || err > y) /* e_xy+e_x > 0 or no 2nd y-step */
			err += ++x * 2 + 1; /* -> x-step now */
	} while (x < 0);
}

//  p1p2 cross p3p4  p1(x1,y1), p2(x2,y2), p3(x3, y3), p4(x4, y4) 
static void crosspt(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double *x, double *y)
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