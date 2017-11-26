#pragma once

#ifndef BASIC_SHAPE_H
#define BASIC_SHAPE_H

#include <algorithm>


template<class T> class Point_;
template<class T> class Rect_;
template<class T> class Circle_;


enum direction { east = 0, southeast = 1, south = 2, southwest = 3, west = 4, northwest = 5, north = 6, northeast = 7 };


////////////////////////////////////////Point_////////////////////////////////////////////

template<class T> class Point_
{
public:
	Point_();
	Point_(T _x, T _y);

	//赋值
	//Point_& operator = (const Point_& pt);

	//!  T类型转换成T2  example below
	//!  Point_<T> pt;
	//!  Point_<T2> pt2 = pt;
	template<class T2> operator Point_<T2>() const;

	////> 判断点是否在矩形内
	//bool inside(const Rect_<T>& rect) const;
	////> 判断点是否在圆内
	//bool inside(const Circle_<T>& c) const;

	Point_<T> at_direction(const int) const;

	T x, y; //< the point coordinates   x对应水平方向，y对应竖直方向
};

typedef Point_<int> Point2i;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
typedef Point2i Point;


/////////////////////////////////////////////Rect_///////////////////////////////////////////


template<class T> class Rect_
{
public:
	Rect_();
	//左上角横坐标和纵坐标 和 宽 高
	Rect_(T _x, T _y, T _width, T _height);


	//Rect_& operator = (const Rect_& r);

	//// 矩形面积
	//T area() const;

	// 判断矩形是否包含点
	//bool contains(const Point_<T>& pt) const;

	//! conversion to another data type
	template<class T2> operator Rect_<T2>() const;


	T x, y, width, height; //< the top-left corner, as well as width and height of the rectangle
};

typedef Rect_<int> Rect2i;
typedef Rect_<float> Rect2f;
typedef Rect_<double> Rect2d;
typedef Rect2i Rect;


//////////////////////////////////////////////Circle_///////////////////////////////////////////

template<class T> class Circle_
{
public:
	// various constructors
	Circle_();
	// 圆心横坐标、纵坐标 以及半径
	Circle_(T _x, T _y, T r);
	// 圆心 和 半径
	Circle_(const Point_<T>& pt, T r);
	//Circle_(const Circle_& pt);

	//Circle_& operator = (const Circle_& c);

	// 圆是否包含点
	//bool contains(const Point_<T>& pt) const;
	// 圆面积 ，返回double类型
	double area() const;
	//! 判断是否为空 ,空则true
	//bool empty() const;
	//! conversion to another data type
	template<class T2> operator Circle_<T2>() const;

	T x, y, radius; //< 横、纵坐标和半径
};

typedef Circle_<int> Circle2i;
typedef Circle_<float> Circle2f;
typedef Circle_<double> Circle2d;
typedef Circle2i Circle;



////////////////////////////////////////////////Contour///////////////////////////////////////////

//class Contour
//{
//public :
//	vector<Point> pts;
//	bool out;
//	double get_perimeter();
//	double get_diameter();
//	double diameter, perimeter;
//};



////////////////////////////////////Point////////////////////////////////////


template<class T> inline 
Point_<T>::Point_()
	: x(0), y(0) {}

template<class T> inline 
Point_<T>::Point_(T _x, T _y)
	: x(_x), y(_y) {}

//template<class T> inline Point_<T>& Point_<T>::operator = (const Point_& pt)
//{
//	x = pt.x; y = pt.y;
//	return *this;
//}
//
template<class T> template<class T2> inline 
Point_<T>::operator Point_<T2>() const
{
	return Point_<T2>(static_cast<T2>(x), static_cast<T2>(y));
}
//
//template<class T> inline bool
//	Point_<T>::inside(const Rect_<T>& rect) const
//{
//	return rect.contains(*this);
//}
//
//template<class T> inline bool
//	Point_<T>::inside(const Circle_<T>& c) const
//{
//	return c.contains(*this);
//}

template<class T1, class T2> static inline
double distance(const Point_<T1>& pt1, const Point_<T2>& pt2)
{
	return std::sqrt((double)(pt1.x - pt2.x)*(pt1.x - pt2.x) + (double)(pt1.y - pt2.y)*(pt1.y - pt2.y));
}

template<class T> inline
	Point_<T> Point_<T>::at_direction(const int dir) const
{
	switch (dir)
	{
	case east:
		return Point_<T>(x + 1, y);
	case southeast:
		return Point_<T>(x + 1, y + 1);
	case south:
		return Point_<T>(x, y + 1);
	case southwest:
		return Point_<T>(x - 1, y + 1);
	case west:
		return Point_<T>(x - 1, y);
	case northwest:
		return Point_<T>(x - 1, y - 1);
	case north:
		return Point_<T>(x, y - 1);
	case northeast:
		return Point_<T>(x + 1, y - 1);
	default:
		return Point_<T>(-1, -1);
	}
}

template<class T> static inline
	T dot(const Point_<T>& pt1, const Point_<T>& pt2)
{
	return static_cast<T>(pt1.x*pt2.x + pt1.y*pt2.y);
}

template<class T> static inline
	double cross(const Point_<T>& pt1, const Point_<T>& pt2)
{
	return double(pt1.x*pt2.y - pt1.y*pt2.x);
}

template<class T> static inline
	Point_<T>& operator += (Point_<T>& a, const Point_<T>& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

template<class T> static inline
	Point_<T>& operator -= (Point_<T>& a, const Point_<T>& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}


template<class T> static inline
	bool operator == (const Point_<T>& a, const Point_<T>& b)
{
	return a.x == b.x && a.y == b.y;
}

template<class T> static inline
	bool operator != (const Point_<T>& a, const Point_<T>& b)
{
	return a.x != b.x || a.y != b.y;
}

template<class T> static inline
	Point_<T> operator + (const Point_<T>& a, const Point_<T>& b)
{
	return Point_<T>(static_cast<T>(a.x + b.x), static_cast<T>(a.y + b.y));
}

template<class T> static inline
	Point_<T> operator - (const Point_<T>& a, const Point_<T>& b)
{
	return Point_<T>(static_cast<T>(a.x - b.x), static_cast<T>(a.y - b.y));
}

template<class T> static inline
	Point_<T> operator - (const Point_<T>& a)
{
	return Point_<T>(static_cast<T>(-a.x), static_cast<T>(-a.y));
}


//////////////////////////////////////////////////////////////////Rect_////////////////////////////////////////////////////////

template<class T> inline
	Rect_<T>::Rect_()
	: x(0), y(0), width(0), height(0) {}

template<class T> inline
	Rect_<T>::Rect_(T _x, T _y, T _width, T _height)
	: x(_x), y(_y), width(_width), height(_height) {}

template<class T> template<class T2> inline
	Rect_<T>::operator Rect_<T2>() const
{
	return Rect_<T2>(static_cast<T2>(x), static_cast<T2>(y), static_cast<T2>(width), static_cast<T2>(height));
}

//template<class T> inline
//	Rect_<T>& Rect_<T>::operator = (const Rect_<T>& rect)
//{
//	x = rect.x; y = rect.y; width = rect.width; height = rect.height;
//	return *this;
//}
//
//template<class T> inline
//	bool Rect_<T>::contains(const Point_<T>& pt) const
//{
//	return x <= pt.x && pt.x < x + width && y <= pt.y && pt.y < y + height;
//}
//
//template<class T> static inline
//	Rect_<T>& operator += (Rect_<T>& a, const Point_<T>& b)
//{
//	a.x += b.x;
//	a.y += b.y;
//	return a;
//}
//
//template<class T> static inline
//	Rect_<T>& operator -= (Rect_<T>& a, const Point_<T>& b)
//{
//	a.x -= b.x;
//	a.y -= b.y;
//	return a;
//}


template<class T> static inline
	Rect_<T>& operator &= (Rect_<T>& a, const Rect_<T>& b)
{
	T x1 = (std::max)(a.x, b.x);
	T y1 = (std::max)(a.y, b.y);
	a.width = (std::min)(a.x + a.width, b.x + b.width) - x1;
	a.height = (std::min)(a.y + a.height, b.y + b.height) - y1;
	a.x = x1;
	a.y = y1;
	if (a.width <= 0 || a.height <= 0)
		a = Rect();
	return a;
}

template<class T> static inline
	Rect_<T>& operator |= (Rect_<T>& a, const Rect_<T>& b)
{
	if (a.width <= 0 || a.height <= 0) {
		a = b;
	}
	else if (b.width > 0 && b.height > 0) {
		T x1 = (std::min)(a.x, b.x);
		T y1 = (std::min)(a.y, b.y);
		a.width = (std::max)(a.x + a.width, b.x + b.width) - x1;
		a.height = (std::max)(a.y + a.height, b.y + b.height) - y1;
		a.x = x1;
		a.y = y1;
	}
	return a;
}


template<class T> static inline
	bool operator == (const Rect_<T>& a, const Rect_<T>& b)
{
	return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

template<class T> static inline
	bool operator != (const Rect_<T>& a, const Rect_<T>& b)
{
	return a.x != b.x || a.y != b.y || a.width != b.width || a.height != b.height;
}

//template<class T> static inline
//	Rect_<T> operator + (const Rect_<T>& a, const Point_<T>& b)
//{
//	return Rect_<T>(a.x + b.x, a.y + b.y, a.width, a.height);
//}
//
//template<class T> static inline
//	Rect_<T> operator - (const Rect_<T>& a, const Point_<T>& b)
//{
//	return Rect_<T>(a.x - b.x, a.y - b.y, a.width, a.height);
//}


template<class T> static inline
	Rect_<T> operator & (const Rect_<T>& a, const Rect_<T>& b)
{
	Rect_<T> c = a;
	return c &= b;
}

template<class T> static inline
	Rect_<T> operator | (const Rect_<T>& a, const Rect_<T>& b)
{
	Rect_<T> c = a;
	return c |= b;
}


/////////////////////////////////////////////////Circle_///////////////////////////////////////////////

template<class T> inline
	Circle_<T>::Circle_()
	: x(0), y(0), radius(0) {}

template<class T> inline
	Circle_<T>::Circle_(T _x, T _y, T r)
	: x(_x), y(_y), radius(r) {}

template<class T> inline
	Circle_<T>::Circle_(const Point_<T>& pt, T r)
	: x(pt.x), y(pt.y), radius(r) {}

template<class T> template<class T2> inline
	Circle_<T>::operator Circle_<T2>() const
{
	return Circle_<T2>(static_cast<T2>(x), static_cast<T2>(y), static_cast<T2>(radius));
}

//template<class T> inline
//	Circle_<T>& Circle_<T>::operator = (const Circle_<T>& c)
//{
//	x = c.x; y = c.y; radius = c.radius;
//	return *this;
//}
//
//template <class T> inline
//	bool Circle_<T>::contains(const Point_<T>& pt) const
//{
//	return distance(Point_<T>(x, y), pt) <= radius;
//}

template <class T> inline
	double Circle_<T>::area() const
{
	return 3.1416*radius*radius;
}


template<class T> static inline
	bool operator == (const Circle_<T>& a, const Circle_<T>& b)
{
	return a.x == b.x && a.y == b.y && a.radius == b.radius;
}

template<class T> static inline
	bool operator != (const Circle_<T>& a, const Circle_<T>& b)
{
	return a.x != b.x || a.y != b.y || a.radius != b.radius;
}



#endif //BASIC_SHAPE_H
