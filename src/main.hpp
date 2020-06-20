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

/**
 * Main and only header file for the project.
 *
 * I chucked in as many includes as I could here, because CGAL is header-only
 * and *massive*, so compiling any file with it included took like 400 years (i
 * mean, not really, but enough time that rapid recompile became super
 * annoying). Therefore I chucked CGAL and as many other header-only libraries
 * (glm, and <chrono> and <random> are the other big ones I guess) as I could in
 * this file and told CMake to make it a precompiled header. That actually more
 * or less works with clang, compile times where much reduced. Fun times!
 *
 * It's maybe weird that I use glm here, but I'm so used to using that for
 * vectors in C++ (and in GLSL obviously), so I tend just to default for that
 * for my vector structs. I suppose I could have used something from CGAL, but
 * I'd like to avoid CGAL for as much as humanly possible. 
 * 
 */

#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Periodic_2_Delaunay_triangulation_2.h>
#include <CGAL/Periodic_2_Delaunay_triangulation_traits_2.h>
#include <fstream>
#include <cassert>
#include <list>
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <cmath>
#include <sstream>
#include <queue>
#include <chrono>

#define TAU (2*M_PI)

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Periodic_2_Delaunay_triangulation_traits_2<K> GT;
typedef CGAL::Periodic_2_Delaunay_triangulation_2<GT>       PDT;

using vec2 = glm::dvec2;
using vec3 = glm::dvec3;
using vec4 = glm::dvec4;
	
/**
 * Struct to contain the various command line options. 
 */
struct options {
	// ints instead of bools to integrate nicer with getopt
	int print_help;

	uint32_t point_count;

	int draw_voronoi;
	int draw_triangulation;
	int draw_circumcircles;

	std::string final_name;
	std::string inter_format;
	
	uint32_t rng_seed;
	uint32_t seed_count;

	float point_size;
	float line_width;
	uint32_t img_size;

    std::unique_ptr<std::ostream> output; 

	options()
		: print_help { false }
		, point_count { 4096 }
		, draw_voronoi       { false }
		, draw_triangulation { false }
		, draw_circumcircles { false }
		, final_name   { "" }
		, inter_format { "" }
		, rng_seed   { 42 }
		, seed_count { 3 }
		, point_size { 3.0f }
		, line_width { 1.0f }
		, img_size   { 1024 }

        , output { nullptr }
	{
	}
};

/**
 * The command line options. I guess it's better and more functional/modern to
 * pass this around as an argument, but you'd need to do that *everywhere*, and
 * this is such a small project. If i made this into a library, i definitely
 * would :) 
 */
extern options opts;

/**
 * Prints usage help text (i.e. --help)
 */
void print_help();

/**
 * Parse command line options
 */
bool parse_options(int argc, char **argv);

/**
 * The main IVS algorithm, with a vector of seeds. Prints out the results to
 * output file or stdout.
 *
 * TODO: this should probably return the list of points. 
 */
void generate_ivs(const std::vector<vec2> &seeds);

/**
 * Return the signed area of a triangle with points a, b, c
 */
double signed_area(vec2 a, vec2 b, vec2 c);

/**
 * Find the intersection of two lines defined by a position and direction
 * vector, returning the required coefficiant of the direction vectors. That is,
 * if the two lines are defined as:
 *
 *   l0(t) = p0 + t * v0
 *   l1(t) = p1 + t * v1
 *
 * Then the intersection is at l0(m0) and l1(m1).
 *
 * Returns false if there is no intersection (and m0 and m1 are NaN).
 */
bool line_line_intersection(vec2 p0, vec2 v0, vec2 p1, vec2 v1, double &m0, double &m1);

/**
 * Rotate a vector v CCW 90 degrees (i.e. if v was a complex number, the
 * equivalent of multiplying with i).
 */
vec2 rotate(vec2 v);

/**
 * Returns the center of the circumcircle of three given points. Will return
 * NaN's if there isn't any.
 *
 * TODO: error-checking by returning NaN's is dumb, this should return a bool
 * and have the result in an argument passed by ref. Or, if you wanna look cool
 * at all the C++ parties, return a std::optional. 
 */
vec2 circumcircle_center(vec2 c0, vec2 c1, vec2 c2);

/**
 * Utility functions to turn points from the internal Delaunay triangulation
 * structure into regular vec2's.
 */
vec2 point(const PDT &trig, const PDT::Periodic_point pnt);
vec2 point(const PDT &trig, const PDT::Vertex_handle pnt);

/**
 * Draw a triangulation using the options in opts and save the drawing to a
 * file. Mostly used for debugging and fancy GitHub gifs. 
 */
void draw_trig(const char *file, const PDT &trig);
