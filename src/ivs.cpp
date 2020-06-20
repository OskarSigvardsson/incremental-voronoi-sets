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
 * Main file containing the IVS algorithm implementation. Most of the heavy
 * lifting is done by CGAL, but there's some stuff going on here as well.
 *
 * Generally speaking, the algorithm is as follows:
 *
 *  1. Begin with some small number of seed points and create a Delaunay
 *     triangulation from the seeds.
 *
 *  2. Find the the largest circumcircle in the triangulation.
 *
 *  3. Add the center of the circumcircle from step 2 as a new point
 *
 *  4. Loop back to 2 until you have as many points as you want.
 *
 * A brute force implementation of that will obviously be quite slow (being
 * O(n^2) or whatever) for large number of points (and the goal is to create an
 * IVS with points in the millions). So what you actually want to do for step 2
 * is to use a priority queue that stores all the circumcircles in the
 * triangulation instead of looping through all of them. Triangles in the queue
 * might not be valid any more (because new points have come in and broke them
 * up), so you might have to pop a couple.
 *
 * This works quite well: generating an IVS with a million points takes about
 * 2.5-3 minutes on my computer. In the original paper they say it took them
 * more than an hour, so that compares very favourably (obviously the credit
 * mostly goes to CGAL).
 *
 * Another complication is that CGAL's periodic delaunay triangulation works in
 * two "modes": single sheet and nine-sheet. If there aren't enough points in
 * the triangulation to cover it properly, it will maintain 9 copies of the
 * triangulation (i.e, a 3x3 grid where the center "square" is the actual one).
 * When enough points have been filled in in the triangulation, it switches to
 * "single-sheet" mode, where only one copy is maintained.
 *
 * Because this switch happens pretty early (usually within a few dozen points),
 * we don't start using the priority queue until we've switched to single-sheet
 * mode. There's just no reason for it, and it complicates the logic. That does
 * mean however that there's an annoying if statement in the middle of the loop,
 * but c'est la vie. 
 */
#include "main.hpp"

/**
 * This structure is the thing that gets put into the priority queue. It
 * maintains the coordinates as well as the vertex handles for the points of the
 * triangle so that we can easily check if it's still valid. It's sorted on the
 * "size" field (which is the radius of the circumcircle of the three points).
 */
struct tris
{
	vec2 p0;
	vec2 p1;
	vec2 p2;
	PDT::Vertex_handle v0;
	PDT::Vertex_handle v1;
	PDT::Vertex_handle v2;
	
	double size;

	tris(){}
	
    tris(vec2 p0, vec2 p1, vec2 p2,
        PDT::Vertex_handle v0,
        PDT::Vertex_handle v1,
        PDT::Vertex_handle v2)

        : p0(p0), p1(p1), p2(p2)
        , v0(v0), v1(v1), v2(v2)
        , size(glm::length(circumcircle_center(p0, p1, p2) - p0))
	{
	}
};

/**
 * Comparison function for the priority queue struct. 
 */
constexpr bool operator<(const tris &t0, const tris &t1)
{
	return t0.size < t1.size;
}

/**
 * Function used to log progress for debug purposes. Only logs at most once
 * every 16 ms, unless forced. 
 */
static void log_progress(int curr, int total, bool force = false)
{
	using namespace std::chrono;

    // I hate <chrono>
	using duration = duration<double, std::milli>;

    // Have I mentioned I *hate* <chrono>
	static time_point<steady_clock> last_logged { steady_clock::now() };

	auto now = steady_clock::now();

    // this library was written by psychopaths. Why am i doing this to myself?
	if (force || duration_cast<duration>(now - last_logged).count() > 16.666)
	{
        // i don't know why i bother with iostream, i should just use fprintf
		std::cerr << "\rPoint " << curr << "/" << total;
		fprintf(stderr, " (%.2f%%)", 100.0 * (double)curr/(double)total);
		last_logged = now;
	}

    std::chrono::high_resolution_clock::now();
}

/**
 * Convenience function for adding a point to the triangulation, as well as
 * recording it's position for the output. 
 */
static PDT::Vertex_handle add_point(PDT &trig, vec2 point)
{
    if (opts.output) {
        *opts.output << std::setprecision(16)
                     << point.x << "," << point.y << std::endl;
    }

	return trig.insert(PDT::Point { point.x, point.y });
}

/**
 * Convenience function for drawing the intermediate triangulations. 
 */
static void draw_inter(const PDT &trig, int i) {
	if (opts.inter_format != "") {
		std::vector<char> buf;

		auto strsz = (size_t)snprintf(nullptr, 0, opts.inter_format.c_str(), i);
		buf.resize(strsz + 1, 0);

		sprintf(buf.data(), opts.inter_format.c_str(), i);

		draw_trig(buf.data(), trig);
	}
}

/**
 * Main procedure for the algorithm.
 */
void generate_ivs(const std::vector<vec2> &seeds)
{
	PDT trig { PDT::Iso_rectangle { 0, 0, 1, 1 } };

	std::priority_queue<tris> pq;

    // Add the seeds
	for (uint32_t i = 0; i < seeds.size(); i++) {
		add_point(trig, seeds[i]);
		draw_inter(trig, i);
	}

    // Are we in one-sheet mode or nine-sheet mode?
	bool one_sheet = false;

	for (uint32_t i = seeds.size(); i < opts.point_count; i++) {
		vec2 new_point;
		
		if (one_sheet) {
            assert(pq.size() > 0 && "Priority queue should not be empty");

			tris t;
			bool is_face = false;

			do {
				t = pq.top();
				pq.pop();

				// Adding new points will split up the triangles, so have this
				// check here to make sure that whatever was popped of the
				// priority queue is still a valid face.
				is_face = trig.is_face(t.v0, t.v1, t.v2);
			} while(pq.size() > 0 && !is_face);

            assert(is_face > 0);

			new_point = circumcircle_center(t.p0, t.p1, t.p2);
		} else {
			auto fb = trig.faces_begin();
			auto fe = trig.faces_end();

			double largest = 0;

            // In nine-sheet mode, we just loop through the triangles to find
            // the largest circumcircle. 
			for (auto it = fb; it != fe; it++) {
				auto triangle = trig.periodic_triangle(it);

				auto p0 = point(trig, triangle[0]);
				auto p1 = point(trig, triangle[1]);
				auto p2 = point(trig, triangle[2]);

				auto c = circumcircle_center(p0, p1, p2);
				auto curr = glm::length(c - p0);

				if (curr > largest) {
					largest = curr;
					new_point = c;
				}
			}

            assert(largest > 0);
		}


		// This is a little bit silly, i could just use fract or whatever, but
		// it used to be that the domain could be anything, not just [0,1). But
		// this is robust and fast enough
		while (new_point.x <  0) new_point.x += 1;
		while (new_point.x >= 1) new_point.x -= 1;
		while (new_point.y <  0) new_point.y += 1;
		while (new_point.y >= 1) new_point.y -= 1;

		assert(new_point.x >= 0);
		assert(new_point.x <  1);
		assert(new_point.y >= 0);
		assert(new_point.y <  1);

        // This insert here might trigger the switch from nine-sheet to
        // one-sheet, so we have to deal with that switch in the next section.
        // If we're in one-sheet mode, inserting this point means we need to add
        // the newly created triangles to the priority queue.  
		auto inserted = add_point(trig, new_point);
		auto sheets = trig.number_of_sheets();

		if (one_sheet && sheets[0]*sheets[1] != 1) {

            // We get here if we've switched from one-sheet to nine-sheet. I
            // don't think this ever happens, I think it only ever goes the
            // other way. But if it does happen, this takes care of it:
            // essentially we just clear the priority queue so that we have to
            // fill it manually when it switches back to one-sheet.

			one_sheet = false;
			pq = std::priority_queue<tris>{};

		} else if (!one_sheet && sheets[0]*sheets[1] == 1) {

            // We get here if we've just switched from nine-sheet to one-sheet.
            // When that happens, loop through all triangles and add them all to
            // the priority queue. 
			one_sheet = true;

			auto fb = trig.faces_begin();
			auto fe = trig.faces_end();

			for (auto it = fb; it != fe; it++) {
				auto triangle = trig.periodic_triangle(it);

				auto p0 = point(trig, triangle[0]);
				auto p1 = point(trig, triangle[1]);
				auto p2 = point(trig, triangle[2]);

				pq.emplace(p0, p1, p2, it->vertex(0), it->vertex(1), it->vertex(2));
			}

		} else if (one_sheet) {

            // If we're in one-sheet mode, some number of new triangles have
            // been created as a result of inserting the point. Those triangles
            // need to be added to the priority queue. 
			auto fb = trig.incident_faces(inserted);
			auto it = fb;

            // An interesting thing to note: adding a point iteratively to a
            // Delaunay triangulation can create any number of new triangles and
            // lead to all sorts of flipping in the structure, but all newly
            // created triangles will have the inserted point as a vertex. So it
            // is safe to just loop through the incident triangles of point and
            // add them, we don't need to do anything else. 
			do {
				auto triangle = trig.periodic_triangle(it);
				auto p0 = point(trig, triangle[0]);
				auto p1 = point(trig, triangle[1]);
				auto p2 = point(trig, triangle[2]);

				pq.emplace(p0, p1, p2, it->vertex(0), it->vertex(1), it->vertex(2));
			} while (++it != fb);

		}

        // Draw intermediate images and record the points
		draw_inter(trig, i);
		log_progress(i, opts.point_count);
	}

    // Log that we've finished
	log_progress(opts.point_count, opts.point_count, true);
	std::cerr << std::endl;

    if (opts.final_name != "") {
        std::cerr << "Drawing final result to " << opts.final_name << std::endl;
        draw_trig(opts.final_name.c_str(), trig);
    }
}
