#include "imports.hpp"
#include "drawer.hpp"

typedef PDT::Face_handle                                    Face_handle;
typedef PDT::Vertex_handle                                  Vertex_handle;
typedef PDT::Locate_type                                    Locate_type;
typedef PDT::Point                                          Point;
typedef PDT::Iso_rectangle                                  Iso_rectangle;
typedef PDT::Iterator_type                                  it_type;

int main()
{
	auto size = 100;
	auto count = 10;
	
	Iso_rectangle domain(0,0,size,size);

	std::vector<Point> ps;

	std::mt19937 engine { 15 } ;
	std::uniform_real_distribution dist;

	//auto rand = std::bind(dist, engine);
	auto rand = [&] { return size * dist(engine); };

	for (int i = 0; i < count; i++) {
		ps.emplace_back(rand(), rand());
	}

	PDT trig(domain);

	trig.insert(ps.begin(), ps.end());
	
	draw_trig(trig);

	return 0;
}
