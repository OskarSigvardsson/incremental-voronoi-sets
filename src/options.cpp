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

#include <getopt.h>

options opts;

void print_help()
{
    // this new type of string literal is pretty rad. 
    std::cout << R"HELP(Incremental voronoi set generator

Usage: ivs [options] [<output-file>]
	
The <output-file> option is a file to save the finished IVS set into. Each line
will have the X and Y coordinates of the dot (in the range [0,1)) separated by a
comma. If <output-file> is a "-", then print to stdout instead. 

Options:
    -h, --help                  Print this help text

    -n, --number                Number of total points to generate
        --seed <n>              Seed for RNG
    -c, --seed-count <n>        Number of initial seed points (default = 2)

    -f, --draw-final <file>     Save final image to file
    -i, --draw-inter <files>    Save intermediate images to file
                                Example: ivs_%07d.png

        --draw-voronoi          Draw the voronoi diagram
        --draw-delaunay         Draw the Delaunay triangulation
        --draw-circumcircles    Draw the circumcircles of the triangulation

    -p, --point-size <n>        Radius of a drawn point
    -l, --line-width <n>        Width a drawn line
    -o, --img-size <n>          Saved images size (output images are square, this
                                is the size of one side)
)HELP";
}

bool parse_options(int argc, char **argv) {
    // never used getopt before, and its not a bad little library! A bit wordy,
    // but basically pretty straightforward.
    static const struct option longopts[]
    {
        { "help",               no_argument,       &(opts.print_help), 1 },   
        { "number",             required_argument, 0, 'n' },
        { "seed",               required_argument, 0, 'e' },
        { "seed-count",         required_argument, 0, 'c' },
        { "draw-final",         required_argument, 0, 'f' },
        { "draw-inter",         required_argument, 0, 'i' },
        { "draw-voronoi",       no_argument,       &(opts.draw_voronoi), 1 },  
        { "draw-delaunay",      no_argument,       &(opts.draw_triangulation), 1 },  
        { "draw-circumcircles", no_argument,       &(opts.draw_circumcircles), 1 },  
        { "point-size",         required_argument, 0, 'p' },
        { "line-width",         required_argument, 0, 'l' },
        { "img-size",           required_argument, 0, 'o' },
        { 0, 0, 0, 0 }
    };

    const char *shortopts = "hvsn:c:f:i:p:l:o:";

    while(1) {
        int optindex;
        int c = getopt_long(argc, argv, shortopts, longopts, &optindex);

        if (c == -1) break;
        
        switch (c) {

        case 0:
            break;

        case 'n':
            try {
                opts.point_count = std::stoi(optarg);
            } catch (...) {
                std::cerr << "Failed to parse point-count" << std::endl;
                return false;
            }
            break;

        case 'e':
            try {
                opts.rng_seed = std::stoul(optarg);
            } catch (...) {
                std::cerr << "Failed to parse seed" << std::endl;
                return false;
            }
            break;

        case 'c':
            try {
                opts.seed_count = std::stoul(optarg);

                if (opts.seed_count < 2) {
                    std::cerr << "Seed count should be >= 2" << std::endl;
                    return false;
                }

            } catch (...) {
                std::cerr << "Failed to parse seed-count" << std::endl;
                return false;
            }
            break;

        case 'f':
            opts.final_name = std::string(optarg);
            break;

        case 'i':
            opts.inter_format = std::string(optarg);
            break;

        case 'p':
            try {
                opts.point_size = std::stof(optarg);
            } catch (...) {
                std::cerr << "Failed to parse point size" << std::endl;
                return false;
            }
            break;

        case 'l':
            try {
                opts.line_width = std::stof(optarg);
            } catch (...) {
                std::cerr << "Failed to parse line width" << std::endl;
                return false;
            }
            break;

        case 'o':
            try {
                opts.img_size = std::stoul(optarg);
            } catch (...) {
                std::cerr << "Failed to parse image size" << std::endl;
                return false;
            }
            break;
            
        case '?':
            return false;
        }
    }

    if (optind < argc) {
        if (strcmp("-", argv[optind]) == 0) {
            opts.output = std::make_unique<std::ostream>(std::cout.rdbuf());
        } else {
            opts.output = std::make_unique<std::ofstream>(argv[optind]);
        }
    }

    return true;
}
