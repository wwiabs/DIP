#include <vector>
using std::vector;

//#define IN_IMG(x, y, w, h) ((x)>=0 && (y)>=0 &&(x)<(w) && (y)<(h))
#define OUT_IMG(x, y, w, h) ((x)<0 || (y)<0 || (x)>=(w) || (y)>=(h))

static char ntdir[8] ={7, 7, 1, 1, 3, 3, 5, 5};

static char dir_coord[8][2] ={
	{1, 0},
	{1, 1},
	{0, 1},
	{-1, 1},
	{-1, 0},
	{-1, -1},
	{0, -1},
	{1, -1}
};

typedef struct _CONTOUR_PT_INFO
{
	int x;
	int y;
	char dir;
}CONTOUR_PT_INFO;

template<typename LabelT>
static
CONTOUR_PT_INFO next_contour_pt(LabelT* p, int w, int h, int x, int y, char dir)
{
	int nx = x + dir_coord[dir][0];
	int ny = y + dir_coord[dir][1];
	while(OUT_IMG(nx, ny, w, h) || !p[ny * w + nx])
	{
		dir = (dir + 1) & 7;
		nx = x + dir_coord[dir][0];
	    ny = y + dir_coord[dir][1];
	}
	return CONTOUR_PT_INFO{nx, ny, dir};
}

template<typename LabelT>
static
bool chk_not_one_pt(LabelT* p, int w, int h, int x, int y, char dir)
{
	int nx = x + dir_coord[dir][0];
	int ny = y + dir_coord[dir][1];
	char count = 0;
	while(OUT_IMG(nx, ny, w, h) || !p[ny * w + nx])
	{
		if(++count == 8)
			return 0;
		dir = (dir + 1) & 7;
		nx = x + dir_coord[dir][0];
	    ny = y + dir_coord[dir][1];
	}
	return 1;
}

template<typename LabelT>
static
vector<CONTOUR_PT_INFO> find_an_out_contour(LabelT* p, int w, int h, int x, int y, char dir)
{
	vector<CONTOUR_PT_INFO> contour;
	if(chk_not_one_pt(p,w,h,x,y,dir))
	{
		CONTOUR_PT_INFO np = next_contour_pt(p,w,h,x,y,dir);
		contour.push_back(np);
		CONTOUR_PT_INFO first = np;
		do
		{
			np = next_contour_pt(p,w,h,np.x,np.y,ntdir[np.dir]);
			contour.push_back(np);
		}while(np.x != first.x || np.y != first.y || np.dir != first.dir);
	}
	else
		contour.push_back(CONTOUR_PT_INFO{x, y, -1});
	return contour;
}

//labeled first
template<typename LabelT>
static
vector<vector<CONTOUR_PT_INFO> > find_out_contour(LabelT* p, int w, int h, LabelT count)
{
	vector<vector<CONTOUR_PT_INFO> > contours(count - 1);
	char* flag = (char*)malloc(count);
	memset(flag, 0, count);
	for(int r=0; r<h; r++)
	{
		LabelT* ptmp = p + r * w;
		for(int c=0; c<w; c++)
		{
			if (ptmp[c] && !flag[ptmp[c]])
			{
				contours[ptmp[c] - 1] = find_an_out_contour(p, w, h, c, r, 0);
				if(--count == 1)
					goto end;
				flag[ptmp[c]] = 1;
			}
		}
	}
	end:
	free(flag);

	return contours;
}

#define UINT8  0
#define UINT16 1
#define INT32  2

vector<vector<CONTOUR_PT_INFO> > find_out_contour(void* p, int w, int h, int count, int type)
{
	vector<vector<CONTOUR_PT_INFO> > tmp;
	if (count < 2)
		return tmp;
	switch (type)
	{
	case UINT8:
		tmp = find_out_contour((unsigned char*)p, w, h, (unsigned char)count);
		break;
	case UINT16:
		tmp = find_out_contour((unsigned short*)p, w, h, (unsigned short)count);
		break;
	case INT32:
		tmp = find_out_contour((int*)p, w, h, (int)count);
		break;
	default:
		break;
	}
	return tmp;
}
//轮廓不一定按顺序排列, 预先做过背景填充、反转以及求BBDT
//template <class T>
//vector<vector<Point> > find_in_contour(const Image_<T>& img, int size)
//{
//	Rect rect(0, 0, img.width, img.height);
//	vector<vector<Point> > contours(size);
//	uchar* label = new uchar[size + 1]();
//	label[0] = 1;
//	Point first, dot, neighbor;
//	int dir;
//	T* p;
//	for (unsigned r = 0; r < img.height; r++)
//	{
//		p = img.ptr(r);
//		for (unsigned c = 0; c < img.width; c++)
//		{
//			if (label[*p] == 0)
//			{
//				vector<Point> &contour = contours.at(*p - 1);
//				first.x = c;
//				first.y = r - 1;
//				contour.push_back(first);
//				dir = southeast;
//				dot = first;
//				do
//				{
//					neighbor = dot.at_direction(dir);
//					find_adjacent_pt(img, rect, dot, &neighbor, &dir);
//					if (dir)
//						dir--;
//					else dir = 7;
//					dot = dot.at_direction(dir);
//					contour.push_back(dot);
//					GET_NEXT_TRACING_DIR(dir);
//				} while (dot != first);
//				label[*p] = 1;
//				if (--size == 0)
//					goto end_loop;
//			}
//			p++;
//		}
//	}
//end_loop:
//	delete[] label;
//	return contours;
//}
