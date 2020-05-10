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

#define TAU (2*M_PI)

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Periodic_2_Delaunay_triangulation_traits_2<K> GT;
typedef CGAL::Periodic_2_Delaunay_triangulation_2<GT>       PDT;
