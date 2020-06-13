#include "imports.hpp"
#include "drawer.hpp"
#include "utility.hpp"
#include "options.hpp"

#define SIZE 1
#define COUNT 1048576

struct tris
{
	vec2 p0;
	vec2 p1;
	vec2 p2;
	vertex_handle v0;
	vertex_handle v1;
	vertex_handle v2;
	
	double size;

	tris(){}
	
	tris(vec2 p0,
		 vec2 p1,


		 vec2 p2,
		 vertex_handle v0,
		 vertex_handle v1,
		 vertex_handle v2)
		: p0(p0), p1(p1), p2(p2)
		, v0(v0), v1(v1), v2(v2)
		, size(glm::length(circumcircle_center(p0, p1, p2) - p0))
	{
	}
};

constexpr bool operator<(const tris &t0, const tris &t1)
{
	return t0.size < t1.size;
}

static void log_progress(int curr, int total, bool force = false)
{
	using namespace std::chrono;

	using duration = duration<double, std::milli>;

	static time_point<steady_clock> last_logged { steady_clock::now() };

	auto now = steady_clock::now();

	if (force || duration_cast<duration>(now - last_logged).count() > 16.666)
	{
		std::cerr << "\rPoint " << curr << "/" << total;
		fprintf(stderr, " (%.2f%%)", 100.0 * (double)curr/(double)total);
		last_logged = now;
	}
}

static vertex_handle add_point(PDT &trig, vec2 point)
{
	std::cout << point.x << "," << point.y << std::endl;
	return trig.insert(Point { point.x, point.y });
}

static int with_queue(vec2 p0, vec2 p1)
{
	PDT trig(rect(0,0,SIZE,SIZE));

	//std::cerr << std::endl;
	std::priority_queue<tris> pq;

	add_point(trig, p0);
	add_point(trig, p1);

	bool one_sheet = false;

	for (int i = 2; i < COUNT; i++) {

		vec2 new_point;
		
		if (one_sheet) {
			if (pq.size() == 0) {
				std::cerr << "Empty priority queue" << std::endl;
				return 1;
			}

			tris t;
			bool is_face = false;

			do {
				t = pq.top();
				pq.pop();
				is_face = trig.is_face(t.v0, t.v1, t.v2);
			} while(pq.size() > 0 && !is_face);

			if (!is_face) {
				std::cerr << "Found no valid triangle in priority queue" << std::endl;
				return 1;
			}

			new_point = circumcircle_center(t.p0, t.p1, t.p2);
		} else {
			auto fb = trig.faces_begin();
			auto fe = trig.faces_end();

			double largest = 0;

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

			if (largest == 0) {
				std::cerr << "Couldn't find largest" << std::endl;
				return 1;
			}
		}

		vertex_handle inserted;

		while (new_point.x < 0) new_point.x += SIZE;
		while (new_point.x >= SIZE) new_point.x -= SIZE;
		while (new_point.y < 0) new_point.y += SIZE;
		while (new_point.y >= SIZE) new_point.y -= SIZE;

		assert(new_point.x >= 0);
		assert(new_point.x < SIZE);
		assert(new_point.y >= 0);
		assert(new_point.y < SIZE);

		inserted = add_point(trig, new_point);

		auto sheets = trig.number_of_sheets();

		if (one_sheet && sheets[0]*sheets[1] != 1) {
			one_sheet = false;
			pq = std::priority_queue<tris>{};
		} else if (!one_sheet && sheets[0]*sheets[1] == 1) {
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
			auto fb = trig.incident_faces(inserted);
			auto it = fb;

			do {
				auto triangle = trig.periodic_triangle(it);
				auto p0 = point(trig, triangle[0]);
				auto p1 = point(trig, triangle[1]);
				auto p2 = point(trig, triangle[2]);

				pq.emplace(p0, p1, p2, it->vertex(0), it->vertex(1), it->vertex(2));
			} while (++it != fb);
		}

		// std::ostringstream ss;
		// ss  << "pngs/out-"
		// 	<< std::setw(2) << std::setfill('0') << i
		// 	<< ".png";
		
		// std::string file(ss.str());
	 	// draw_trig(file.c_str(), trig, inserted);
		//std::cout << "\rAdded point " << i << " of " << COUNT << "                   ";
		log_progress(i, COUNT);
	}

	log_progress(COUNT, COUNT, true);
	std::cerr << std::endl;
	draw_trig("pngs/queue.png", trig, vertex_handle{});

	return 0;
}


void print_help() {
	std::cout << R"HELP(Incremental voronoi set generator

Usage:
	ivs -n <count> [options] [file]

General options:
    -h, --help                  Print this help text
    -v, --verbose               Be verbose
    -s, --silent                Don't print progress information

Generation options:
    -n, --number                Number of total points to generate                    
    -s, --seed <n>              Seed for RNG
    -c, --seed-count <n>        Number of initial seed points (default = 2)

Drawing options:
	-f, --draw-final <file>     Save final image to file
	-i, --draw-inter <files>    Save intermediate images to file
                                Example: "-i IMG_%05d.png"

    --draw-sites                Draw the voronoi sites
    --draw-voronoi              Draw the voronoi diagram
    --draw-delaunay             Draw the Delaunay triangulation 
    --draw-circumcircles        Draw the circumcircles of the triangulation
    
    -p, --point-size <n>        Size of a drawn point
    -l, --line-width <n>        Width a drawn line 
    -i, --img-size <n>          Saved images size (output images are square, this 
                                is the size of one side)
)HELP";
}

int main(int argc, char **argv)
{
	std::mt19937 engine { 15 } ;
	std::uniform_real_distribution dist;

	auto rand = [&] { return SIZE * dist(engine); };

	vec2 p0 { rand(), rand() };
	vec2 p1 { rand(), rand() };

	if(with_queue(p0, p1)) return 1;

	return 0;
}
