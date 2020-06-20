/**
 * Copyright 2020 Oskar Sigvardsson
 *
 * This file is part of ivs-generator.
 * 
 * ivs-generator is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * ivs-generator is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ivs-generator. If not, see <https://www.gnu.org/licenses/>.
 */

#include "main.hpp"

#include <cairo.h>

static void draw_wrapped_line(cairo_t *cr, float x0, float y0, float x1, float y1)
{
	cairo_move_to(cr, x0, y0);
	cairo_line_to(cr, x1, y1);

	auto outside = [](float v) { return v < 0 || v >= 1; };
	
	if (outside(x0) || outside(y0) || outside(x1) || outside(y1)) {
		cairo_move_to(cr, x0-1, y0-1);
		cairo_line_to(cr, x1-1, y1-1);

		cairo_move_to(cr, x0  , y0-1);
		cairo_line_to(cr, x1  , y1-1);

		cairo_move_to(cr, x0+1, y0-1);
		cairo_line_to(cr, x1+1, y1-1);

		cairo_move_to(cr, x0-1, y0  );
		cairo_line_to(cr, x1-1, y1  );

		cairo_move_to(cr, x0+1, y0  );
		cairo_line_to(cr, x1+1, y1  );

		cairo_move_to(cr, x0-1, y0+1);
		cairo_line_to(cr, x1-1, y1+1);

		cairo_move_to(cr, x0  , y0+1);
		cairo_line_to(cr, x1  , y1+1);

		cairo_move_to(cr, x0+1, y0+1);
		cairo_line_to(cr, x1+1, y1+1);

	}
}

static void draw_triangles(const PDT &trig, cairo_t *cr)
{
	auto tb = trig.faces_begin();
	auto te = trig.faces_end();

	int i = 0;
	
	for (auto it = tb; it != te; i++, it++) {
		auto p0 = point(trig, trig.periodic_triangle(it)[0]);
		auto p1 = point(trig, trig.periodic_triangle(it)[1]);
		auto p2 = point(trig, trig.periodic_triangle(it)[2]);

        cairo_new_sub_path(cr);
        draw_wrapped_line(cr, p0.x, p0.y, p1.x, p1.y);
        draw_wrapped_line(cr, p1.x, p1.y, p2.x, p2.y);
        draw_wrapped_line(cr, p2.x, p2.y, p0.x, p0.y);
        cairo_close_path(cr);
	}

	cairo_stroke(cr);
}

static void draw_circumcircles(const PDT &trig, cairo_t *cr)
{
	auto tb = trig.periodic_triangles_begin();
	auto te = trig.periodic_triangles_end();

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

static void draw_voronoi(const PDT &trig, cairo_t *cr)
{
	auto eb = trig.edges_begin();
	auto ee = trig.edges_end();

	for (auto it = eb; it != ee; it++) {
		auto segment = trig.dual(it);

		auto p0 = segment.source();
		auto p1 = segment.target();

		draw_wrapped_line(cr, p0.x(), p0.y(), p1.x(), p1.y());
	}

	cairo_stroke(cr);
}

static void draw_sites(const PDT &trig, cairo_t *cr) {
	auto vb = trig.vertices_begin();
	auto ve = trig.vertices_end();

	for (auto it = vb; it != ve; it++) {
		auto r = opts.point_size / opts.img_size;
		
		cairo_new_sub_path(cr);
		cairo_move_to(cr, it->point().x() + r, it->point().y());
		cairo_arc(cr, it->point().x(), it->point().y(), r, 0, TAU);
		cairo_close_path(cr);
	}

	cairo_fill(cr);
}

void draw_trig(const char *file, const PDT &trig)
{
	auto size = opts.img_size;

	auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, opts.img_size, size);
	auto cr = cairo_create(surface);

	// Transform the canvas so that (0,0) is bottom left and (1,1) is top right
	cairo_scale(cr, 1, -1);
	cairo_translate(cr, 0, -size);
	cairo_scale(cr, size, size),
	
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
	cairo_paint(cr);

	cairo_set_line_width(cr, opts.line_width / size);

	if (opts.draw_circumcircles) 
	{
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.3);
		draw_circumcircles(trig, cr);
		cairo_stroke(cr);
	}

	if (opts.draw_triangulation) 
	{
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
		draw_triangles(trig, cr);
	}

	if (opts.draw_voronoi) 
	{
		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		draw_voronoi(trig, cr);
	}

	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
	draw_sites(trig, cr);

	cairo_surface_write_to_png(surface, file);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}
