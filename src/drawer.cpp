#include "imports.hpp"
#include "drawer.hpp"

#include <cairo.h>

using vec2 = glm::vec2;
	
typedef PDT::Iterator_type it_type;
typedef std::pair<PDT::Point, PDT::Offset> period_point;

#define DRAWTYPE (it_type::STORED)
#define CELLS 5
#define SCALE 2.0

static bool line_line_intersection(
	vec2 p0,
	vec2 v0,
	vec2 p1,
	vec2 v1,
	float &m0,
	float &m1)
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

static vec2 rotate(vec2 v) {
	auto x = v.x;
	v.x = -v.y;
	v.y = x;

	return v;
}

static vec2 circumcircle_center(vec2 c0, vec2 c1, vec2 c2) {
	auto mp0 = 0.5f * (c0 + c1);
	auto mp1 = 0.5f * (c1 + c2);

	auto v0 = rotate(c0 - c1);
	auto v1 = rotate(c1 - c2);

	float m0, m1;

	line_line_intersection(mp0, v0, mp1, v1, m0, m1);

	return mp0 + m0 * v0;
}

static vec2 point(const PDT &trig, period_point pnt)
{
	auto domain = trig.domain();
	auto width = domain.xmax() - domain.xmin();
	auto height = domain.ymax() - domain.ymin();
	
	return vec2 {
		pnt.first.x() + pnt.second.x() * width,
		pnt.first.y() + pnt.second.y() * height };
}

static void draw_triangles(const PDT &trig, cairo_t *cr)
{
	auto tb = trig.periodic_triangles_begin(DRAWTYPE);
	auto te = trig.periodic_triangles_end(DRAWTYPE);
	// auto tb = trig.periodic_triangles_begin();
	// auto te = trig.periodic_triangles_end();

	auto w = trig.domain().xmax() - trig.domain().xmin();
	auto h = trig.domain().ymax() - trig.domain().ymin();

	auto inw = [=] (double x) { return w <= x && x <= 2*w; };
	auto inh = [=] (double x) { return w <= x && x <= 2*h; };

	for (auto it = tb; it != te; it++) {
		auto p0 = point(trig, (*it)[0]);
		auto p1 = point(trig, (*it)[1]);
		auto p2 = point(trig, (*it)[2]);

		// if (inw(x0) && inw(x1) && inw(x2) && inh(y0) && inh(y1) && inh(y2))
		// {
		cairo_new_sub_path(cr);
		cairo_move_to(cr, p0.x, p0.y);
		cairo_line_to(cr, p1.x, p1.y);
		cairo_line_to(cr, p2.x, p2.y);
		cairo_close_path(cr);
		//cairo_line_to(cr, p0.x, p0.y);
		// }
		
		// std::cout
		// 	<< "(" << x0 << "," << y0 << ") "
		// 	<< "(" << x1 << "," << y1 << ") "
		// 	<< "(" << x2 << "," << y2 << ") "
		// 	<< std::endl;
	}
}

static void draw_circumcircles(const PDT &trig, cairo_t *cr)
{
	auto tb = trig.periodic_triangles_begin(DRAWTYPE);
	auto te = trig.periodic_triangles_end(DRAWTYPE);
	// auto tb = trig.periodic_triangles_begin();
	// auto te = trig.periodic_triangles_end();

	for (auto it = tb; it != te; it++) {
		auto p0 = point(trig, (*it)[0]);
		auto p1 = point(trig, (*it)[1]);
		auto p2 = point(trig, (*it)[2]);

		auto c = circumcircle_center(p0, p1, p2);
		auto r = glm::length(c - p0);

		cairo_move_to(cr, c.x + r, c.y);
		cairo_arc(cr, c.x, c.y, r, 0, TAU);
	}
}

void draw_trig(const PDT &trig)
{
	auto domain = trig.domain();
	auto ctx = CELLS > 1 ? 1 : 0;

	auto dw = (domain.xmax() - domain.xmin());
	auto dh = (domain.ymax() - domain.ymin());

	auto iw = SCALE * dw * CELLS;
	auto ih = SCALE * dh * CELLS;

	auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, iw, ih);
	auto cr = cairo_create(surface);

	cairo_scale(cr, 1, -1);
	cairo_translate(cr, 0, -ih);
	cairo_scale(cr, SCALE, SCALE),
	cairo_translate(cr, ctx*dw, ctx*dh);
	// cairo_translate(cr, 100, 0);
	
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
	cairo_paint(cr);

	cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
	cairo_set_line_width(cr, 1.0 / SCALE);


	for (int tx = 0; tx < CELLS - 1; tx++) {
		cairo_move_to(cr, tx*dw, -dh);
		cairo_line_to(cr, tx*dw, (CELLS-1)*dh);
	}

	for (int ty = 0; ty < CELLS; ty++) {
		cairo_move_to(cr, -dw, ty*dh);
		cairo_line_to(cr, (CELLS-1)*dw, ty*dh);
	}

	cairo_stroke(cr);

	{
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.3);
		cairo_set_line_width(cr, 2.0 / SCALE);
		draw_circumcircles(trig, cr);
		cairo_stroke(cr);
	}

	{
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
		cairo_set_line_width(cr, 2.0 / SCALE);
		draw_triangles(trig, cr);
		cairo_stroke(cr);
	}


	cairo_surface_write_to_png(surface, "out.png");
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}
