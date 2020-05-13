#include "imports.hpp"
#include "drawer.hpp"
#include "utility.hpp"

typedef PDT::Face_handle   Face_handle;
typedef PDT::Locate_type   Locate_type;
typedef PDT::Point         Point;
typedef PDT::Iso_rectangle rect;
typedef PDT::Iterator_type it_type;

#define SIZE 1024
//#define COUNT (1024*1024)
#define COUNT 1048576
//#define COUNT 100000

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

// static int brute_force(vec2 p0, vec2 p1)
// {
// 	std::cout << "------------------------------" << std::endl;
// 	std::cout << "brute_force" << std::endl;

// 	PDT trig(rect(0,0,SIZE,SIZE));

// 	std::cout << std::endl;

// 	trig.insert(Point { p0.x, p0.y });
// 	trig.insert(Point { p1.x, p1.y });

// 	for (int i = 2; i < COUNT; i++) {
// 		auto fb = trig.faces_begin();
// 		auto fe = trig.faces_end();

// 		double largest = 0;
// 		vec2 new_point;
		
// 		for (auto it = fb; it != fe; it++) {
// 			auto triangle = trig.periodic_triangle(it);

// 			auto p0 = point(trig, triangle[0]);
// 			auto p1 = point(trig, triangle[1]);
// 			auto p2 = point(trig, triangle[2]);

// 			auto c = circumcircle_center(p0, p1, p2);
// 			auto curr = glm::length(c - p0);

// 			if (curr > largest) {
// 				largest = curr;
// 				new_point = c;
// 			}
// 		}

// 		vertex_handle inserted;

// 		if (largest == 0) {
// 			std::cerr << "Couldn't find largest" << std::endl;
// 			return 1;
// 		} else {
// 			while (new_point.x < 0) new_point.x += SIZE;
// 			while (new_point.x >= SIZE) new_point.x -= SIZE;
// 			while (new_point.y < 0) new_point.y += SIZE;
// 			while (new_point.y >= SIZE) new_point.y -= SIZE;

// 			assert(new_point.x >= 0);
// 			assert(new_point.x < SIZE);
// 			assert(new_point.y >= 0);
// 			assert(new_point.y < SIZE);

// 			inserted = trig.insert(Point(new_point.x, new_point.y));
// 		}

// 		// std::ostringstream ss;
// 		// ss  << "pngs/out-"
// 		// 	<< std::setw(2) << std::setfill('0') << i
// 		// 	<< ".png";
		
// 		// std::string file(ss.str());
// 	 	// draw_trig(file.c_str(), trig, inserted);
// 		// auto sheets = trig.number_of_sheets();
// 		// std::cout << sheets[0] * sheets[1] << std::endl;
// 		//std::cout << "\rAdded point " << i << " of " << COUNT << "                   ";
// 		log_progress(i, COUNT);
// 	}

// 	log_progress(COUNT, COUNT, true);

// 	//draw_trig("pngs/brute.png", trig, vertex_handle{});

// 	return 0;
// }

int main()
{
	//Iso_rectangle domain(0,0,SIZE,SIZE);


	std::mt19937 engine { 15 } ;
	std::uniform_real_distribution dist;

	auto rand = [&] { return SIZE * dist(engine); };

	vec2 p0 { rand(), rand() };
	vec2 p1 { rand(), rand() };

	if(with_queue(p0, p1)) return 1;
	//if(brute_force(p0, p1)) return 1;
	
	// std::vector<Point> ps;
	// for (int i = 0; i < COUNT; i++) {
	// 	ps.emplace_back(rand(), rand());
	// }

	// std::vector<Point> ps {
	// 	{ 25, 25 },
	// 	{ 75, 25 },
	// 	{ 50, 75 },
	// };


	//trig.insert(ps.begin(), ps.end());
	
	// for (int i = 0; i < ps.SIZE(); i++) {
	// 	auto inserted = trig.insert(ps[i]);

	// 	std::ostringstream ss;
	// 	ss  << "pngs/out-"
	// 		<< std::setw(2) << std::setfill('0') << i
	// 		<< ".png";
		
	// 	std::string file(ss.str());
		
	// 	draw_trig(file.c_str(), trig, inserted);
	// }


	// auto fb = trig.faces_begin();
	// auto fe = trig.faces_end();

	// for (auto it = fb; it != fe; it++) {
	// 	auto v0 = it->vertex(0);
	// 	auto v1 = it->vertex(1);
	// 	auto v2 = it->vertex(2);

	// 	std::cout
	// 		<< "(" << *v0 << ") "
	// 		<< "(" << *v1 << ") "
	// 		<< "(" << *v2 << ") "
	// 		<< std::endl;
	// }


	return 0;
}
