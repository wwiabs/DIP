#include "basic_shape.h"
#include <random>

using std::vector;

static std::random_device rd;

static std::default_random_engine randGen(rd());

Circle2d makeDiameter(const Point &a, const Point &b) {
	Point2d c(double(a.x + b.x) / 2, double(a.y + b.y) / 2);
	return Circle2d(c, distance(a, b) / 2);
}

Circle2d makeCircumcircle(const Point &a, const Point &b, const Point &c) {
	// Mathematical algorithm from Wikipedia: Circumscribed circle
	double ox = ((std::min)((std::min)(a.x, b.x), c.x) + (std::max)((std::min)(a.x, b.x), c.x)) / 2;
	double oy = ((std::min)((std::min)(a.y, b.y), c.y) + (std::max)((std::min)(a.y, b.y), c.y)) / 2;
	double ax = a.x - ox, ay = a.y - oy;
	double bx = b.x - ox, by = b.y - oy;
	double cx = c.x - ox, cy = c.y - oy;
	double d = (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) * 2;
	if (d == 0)
		return Circle2d(0, 0, -1);
	double x = ((ax * ax + ay * ay) * (by - cy) + (bx * bx + by * by) * (cy - ay) + (cx * cx + cy * cy) * (ay - by)) / d;
	double y = ((ax * ax + ay * ay) * (cx - bx) + (bx * bx + by * by) * (ax - cx) + (cx * cx + cy * cy) * (bx - ax)) / d;
	Point2d p(ox + x, oy + y);
	double r = (std::max)((std::max)(distance(p, Point_<double>(a.x, a.y)), distance(p, Point_<double>(b.x, b.y))), distance(p, Point_<double>(c.x, c.y)));
	return Circle2d(p, r);
}

// Two boundary points known
static Circle2d makeSmallestEnclosingCircleTwoPoints(const vector<Point> &points, size_t end, const Point &p, const Point &q) {
	Circle2d circ = makeDiameter(p, q);
	Circle2d left(0, 0, -1);
	Circle2d right(0, 0, -1);

	// For each point not in the two-point circle
	Point pq = q - p;
	for (size_t i = 0; i < end; i++) {
		const Point &r(points.at(i));
		if (circ.contains(r))
			continue;

		// Form a circumcircle and classify it on left or right side
		double cross_value = cross(pq, r - p);
		Circle2d c = makeCircumcircle(p, q, r);
		if (c.radius < 0)
			continue;
		else if (cross_value > 0 && (left.radius < 0 || cross(Point_<double>(pq.x, pq.y), Point2d(c.x, c.y) - Point_<double>(p.x, p.y)) > cross(Point_<double>(pq.x, pq.y), Point2d(left.x, left.y) - Point_<double>(p.x, p.y))))
			left = c;
		else if (cross_value < 0 && (right.radius < 0 || cross(Point_<double>(pq.x, pq.y), Point2d(c.x, c.y) - Point_<double>(p.x, p.y)) < cross(Point_<double>(pq.x, pq.y), Point2d(right.x, right.y) - Point_<double>(p.x, p.y))))
			right = c;
	}

	// Select which circle to return
	if (left.radius < 0 && right.radius < 0)
		return circ;
	else if (left.radius < 0)
		return right;
	else if (right.radius < 0)
		return left;
	else
		return left.radius <= right.radius ? left : right;
}

// One boundary point known
static Circle2d makeSmallestEnclosingCircleOnePoint(const vector<Point> &points, size_t end, const Point &p) {
	Circle2d c(p, 0);
	for (size_t i = 0; i < end; i++) {
		const Point &q(points.at(i));
		if (!c.contains(q)) {
			if (c.radius == 0)
				c = makeDiameter(p, q);
			else
				c = makeSmallestEnclosingCircleTwoPoints(points, i + 1, p, q);
		}
	}
	return c;
}

// Initially: No boundary points known
Circle2d makeSmallestEnclosingCircle(const vector<Point> &points) {
	// Clone list to preserve the caller's data, randomize order
	vector<Point> shuffled(points);
	std::shuffle(shuffled.begin(), shuffled.end(), randGen);

	// Progressively add points to circle or recompute circle
	Circle2d c(0, 0, -1);
	for (size_t i = 0; i < shuffled.size(); i++) {
		const Point &p(shuffled.at(i));
		if (c.radius < 0 || !c.contains(p))
			c = makeSmallestEnclosingCircleOnePoint(shuffled, i + 1, p);
	}

	return c;
}