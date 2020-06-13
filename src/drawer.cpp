#include "imports.hpp"
#include "drawer.hpp"
#include "utility.hpp"

#include <cairo.h>


//#define DRAWTYPE (it_type::STORED)
#define DRAWTYPE //(it_type::STORED)
#define CELLS 1
#define START 0
#define SCALE (1024 * 16.0)
#define DOT_RADIUS 0.25
#define LINE_THICKNESS 2.0


static void draw_triangles(const PDT &trig, cairo_t *cr)
{
	auto tb = trig.faces_begin();
	auto te = trig.faces_end();
	// auto tb = trig.periodic_triangles_begin(DRAWTYPE);
	// auto te = trig.periodic_triangles_end(DRAWTYPE);
	// auto tb = trig.periodic_triangles_begin();
	// auto te = trig.periodic_triangles_end();

	// auto w = trig.domain().xmax() - trig.domain().xmin();
	// auto h = trig.domain().ymax() - trig.domain().ymin();

	// auto inw = [=] (double x) { return w <= x && x <= 2*w; };
	// auto inh = [=] (double x) { return w <= x && x <= 2*h; };

	int i = 0;
	
	for (auto it = tb; it != te; i++, it++) {
		// auto p0 = point(trig, (*it)[0]);
		// auto p1 = point(trig, (*it)[1]);
		// auto p2 = point(trig, (*it)[2]);

		auto p0 = point(trig, trig.periodic_triangle(it)[0]);
		auto p1 = point(trig, trig.periodic_triangle(it)[1]);
		auto p2 = point(trig, trig.periodic_triangle(it)[2]);


		if (true || signed_area(p0, p1, p2) > 0) {

			// if (inw(x0) && inw(x1) && inw(x2) && inh(y0) && inh(y1) && inh(y2))
			// {
			cairo_new_sub_path(cr);
			cairo_move_to(cr, p0.x, p0.y);
			cairo_line_to(cr, p1.x, p1.y);
			cairo_line_to(cr, p2.x, p2.y);
			cairo_close_path(cr);
			cairo_stroke(cr);
			//cairo_line_to(cr, p0.x, p0.y);
			// }

			// std::cout
			// 	<< "(" << x0 << "," << y0 << ") "
			// 	<< "(" << x1 << "," << y1 << ") "
			// 	<< "(" << x2 << "," << y2 << ") "
			// 	<< std::endl;
			// char buf[100];
			// std::string file("pngs/out-stepped-");
			// sprintf(buf, "%02d", i);
			// file.append(buf);
			// file.append(".png");
			// std::cout << file << std::endl;
			// cairo_surface_write_to_png(surface, file.c_str());
		}
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


static void number_faces(const PDT &trig, cairo_t *cr)
{
	auto fb = trig.faces_begin();
	auto fe = trig.faces_end();
	// auto tb = trig.periodic_triangles_begin();
	// auto te = trig.periodic_triangles_end();

	//print = true;
	int i = 0;
	for (auto it = fb; it != fe; it++) {
		if (it == fb) continue;
		std::cout << i << ": ";
		auto p0 = point(trig, it->vertex(0));
		auto p1 = point(trig, it->vertex(1));
		auto p2 = point(trig, it->vertex(2));


		std::cout << signed_area(p0, p1, p2) << std::endl;
		auto avg = (p0 + p1 + p2) / 3.0;

		std::string n = std::to_string(i++);

		if (signed_area(p0, p1, p2) > 0.0) {
			std::cout << "Drawing " << i-1 << " at " << avg.x << ", " << avg.y << std::endl;
			cairo_move_to(cr, avg.x, avg.y);
			cairo_show_text(cr, n.c_str());
		}
	}
	//print = false;
}

static void draw_incident_triangles(const PDT &trig, const vertex_handle vertex, cairo_t *cr)
{
	auto fb = trig.incident_faces(vertex);

	auto it = fb;

	auto i = 0;

	std::vector<vec4> colors {
		vec4 { 0, 0, 0, 0.2 },
		vec4 { 0, 0, 0, 0.3 }
	};

	do {
		auto tris = trig.periodic_triangle(it);
		auto p0 = point(trig, tris[0]);
		auto p1 = point(trig, tris[1]);
		auto p2 = point(trig, tris[2]);

		auto color = colors[i % colors.size()];
		
		cairo_set_source_rgba(cr, color.r, color.b, color.g, color.a);
		
		cairo_new_sub_path(cr);
		cairo_move_to(cr, p0.x, p0.y);
		cairo_line_to(cr, p1.x, p1.y);
		cairo_line_to(cr, p2.x, p2.y);
		cairo_close_path(cr);

		cairo_fill(cr);
		
		i++;
	} while (++it != fb);
}

static void draw_voronoi(const PDT &trig, cairo_t *cr)
{
	auto fb = trig.faces_begin();
	auto fe = trig.faces_end();

	auto eb = trig.edges_begin();
	auto ee = trig.edges_end();

	auto vb = trig.vertices_begin();
	auto ve = trig.vertices_end();

	for (auto it = eb; it != ee; it++) {
		auto segment = trig.dual(it);

		auto p0 = segment.source();
		auto p1 = segment.target();

		cairo_move_to(cr, p0.x(), p0.y());
		cairo_line_to(cr, p1.x(), p1.y());
	}

	cairo_stroke(cr);

	// for (auto it = fb; it != fe; it++) {
	// 	auto p = trig.dual(it);

	// 	cairo_new_sub_path(cr);
	// 	cairo_move_to(cr, p.x() + DOT_RADIUS, p.y());
	// 	cairo_arc(cr, p.x(), p.y(), DOT_RADIUS, 0, TAU);
	// 	cairo_close_path(cr);
	// }

	for (auto it = vb; it != ve; it++) {
		//auto p = trig.dual(it);

		cairo_new_sub_path(cr);
		cairo_move_to(cr, it->point().x() + DOT_RADIUS, it->point().y());
		cairo_arc(cr, it->point().x(), it->point().y(), DOT_RADIUS, 0, TAU);
		cairo_close_path(cr);
	}

	cairo_fill(cr);
}

static void draw_sites(const PDT &trig, cairo_t *cr) {
	auto vb = trig.vertices_begin();
	auto ve = trig.vertices_end();

	for (auto it = vb; it != ve; it++) {
		auto p = point(trig, trig.periodic_point(it));

		cairo_new_sub_path(cr);
		cairo_move_to(cr, p.x + DOT_RADIUS, p.y);
		cairo_arc(cr, p.x, p.y, DOT_RADIUS, 0, TAU);
		cairo_close_path(cr);
	}
}

void draw_trig(const char *file, const PDT &trig, const vertex_handle last_inserted)
{
	auto domain = trig.domain();

	auto dw = (domain.xmax() - domain.xmin());
	auto dh = (domain.ymax() - domain.ymin());

	auto iw = SCALE * dw * CELLS;
	auto ih = SCALE * dh * CELLS;

	auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, iw, ih);
	auto cr = cairo_create(surface);

	//cairo_scale(cr, 1, -1);
	//cairo_translate(cr, 0, -ih);
	cairo_scale(cr, SCALE, SCALE),
	cairo_translate(cr, -START*dw, -START*dh);
	// cairo_translate(cr, 100, 0);
	
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
	cairo_paint(cr);

	cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
	cairo_set_line_width(cr, 1.0 / SCALE);


	for (int tx = START; tx < CELLS + START; tx++) {
		cairo_move_to(cr, tx*dw, START*dh);
		cairo_line_to(cr, tx*dw, (CELLS+START)*dh);
	}

	for (int ty = START; ty < CELLS + START; ty++) {
		cairo_move_to(cr, START * dw, ty*dh);
		cairo_line_to(cr, (CELLS+START)*dw, ty*dh);
	}

	cairo_stroke(cr);
	cairo_set_line_width(cr, LINE_THICKNESS / SCALE);

	{
		//draw_incident_triangles(trig, last_inserted, cr);
	}

	{
		// cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		// draw_voronoi(trig, cr);
	}

	{
		// cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.3);
		// draw_circumcircles(trig, cr);
		// cairo_stroke(cr);
	}

	// {
	// 	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	// 	draw_triangles(trig, cr);
	// 	cairo_stroke(cr);
	// }

	{
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
		draw_sites(trig, cr);
		cairo_fill(cr);
	}


	// {
	// 	number_faces(trig, cr);
	// 	cairo_stroke(cr);
	// }

	std::cout << "Writing " << file << std::endl;
	cairo_surface_write_to_png(surface, file);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}
