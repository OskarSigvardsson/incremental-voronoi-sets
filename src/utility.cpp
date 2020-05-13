#include "imports.hpp"
#include "utility.hpp"

double signed_area(vec2 a, vec2 b, vec2 c)
{
	vec3 a3 { a, 0.0 };
	vec3 b3 { b, 0.0 };
	vec3 c3 { c, 0.0 };

	vec3 ab = glm::cross(a3, b3);
	vec3 bc = glm::cross(b3, c3);
	vec3 ca = glm::cross(c3, a3);

	return 0.5 * (ab.z + bc.z + ca.z);
}

bool line_line_intersection(
	vec2 p0,
	vec2 v0,
	vec2 p1,
	vec2 v1,
	double &m0,
	double &m1)
{
	auto det = (v0.x * v1.y - v0.y * v1.x);

	if (abs(det) < 0.001f) {
		m0 = NAN;
		m1 = NAN;

		return false;
	} else {
		m0 = ((p0.y - p1.y) * v1.x - (p0.x - p1.x) * v1.y) / det;

		if (abs(v1.x) >= 0.001f) {
			m1 = (p0.x + m0*v0.x - p1.x) / v1.x;
		} else {
			m1 = (p0.y + m0*v0.y - p1.y) / v1.y;
		}

		return true;
	}
}

vec2 rotate(vec2 v) {
	auto x = v.x;
	v.x = -v.y;
	v.y = x;

	return v;
}

vec2 circumcircle_center(vec2 c0, vec2 c1, vec2 c2) {
	auto mp0 = 0.5 * (c0 + c1);
	auto mp1 = 0.5 * (c1 + c2);

	auto v0 = rotate(c0 - c1);
	auto v1 = rotate(c1 - c2);

	double m0, m1;

	line_line_intersection(mp0, v0, mp1, v1, m0, m1);

	return mp0 + m0 * v0;
}

vec2 point(const PDT &trig, const period_point pnt)
{
	auto domain = trig.domain();
	auto width = domain.xmax() - domain.xmin();
	auto height = domain.ymax() - domain.ymin();
	
	// if (print) {
	// 	std::cout << "("
	// 			  << pnt.first.x() << " " << pnt.second.x() << ", "
	// 			  << pnt.first.y() << " " << pnt.second.y() << ") ";
	// }

	return vec2 {
		pnt.first.x() + pnt.second.x() * width,
			pnt.first.y() + pnt.second.y() * height };
}


vec2 point(const PDT &trig, const vertex_handle pnt)
{
	//std::cout << "(" << pnt->point().x() << "," << pnt->point().y() << ") ";
	//return vec2 { pnt->point().x(), pnt->point().y() };
	return point(trig, trig.periodic_point(pnt));
}
