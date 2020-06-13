#pragma once
#include <stdint.h>
#include <fstream>

struct options {
	bool verbose;
	bool print_help;

	bool draw_final;
	bool draw_intermediate;
	bool draw_sites;
	bool draw_voronoi;
	bool draw_triangulation;
	bool draw_circumcircles;

	std::string final_name;
	std::string inter_format;
	
	uint64_t rng_seed;
	int seed_count;

	std::ostream output;
	std::ostream log;

	float point_size;
	float line_width;
	uint32_t img_size;

	options()
		: verbose    { false }
		, print_help { false }

		, draw_final         { false }
		, draw_intermediate  { false }
		, draw_sites         { false }
		, draw_voronoi       { false }
		, draw_triangulation { false }
		, draw_circumcircles { false }

		, final_name   { "ivs.png" }
		, inter_format { "ivs_%07d.png" }

		, rng_seed   { 42 }
		, seed_count { 3 }

		, output { std::cout.rdbuf() }
		, log    { std::cerr.rdbuf() }

		, point_size { 3.0f }
		, line_width { 1.0f }
		, img_size   { 1024 }
	{
	}
};

void print_help();
options parse_options(int argc, char **argv);
