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

/**
 * Main function. Parses command line arguments, generates the seed points, runs
 * the algoritm.
 */
int main(int argc, char **argv)
{
	if (!parse_options(argc, argv)) {
		return 1;
	}

	if (opts.print_help) {
		print_help();
		return 0;
	}

	std::mt19937 engine { opts.rng_seed } ;
	std::uniform_real_distribution dist;

	std::vector<vec2> seeds { opts.seed_count };
	
	for (int i = 0; i < opts.seed_count; i++)
	{
		seeds[i] = { dist(engine), dist(engine) };
	}

    try {
        generate_ivs(seeds);
    } catch (...) {
        std::cerr << "Failed to generate IVS" << std::endl;
        return 1;
    }

	return 0;
}
