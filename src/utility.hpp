#pragma once

#include "imports.hpp"
#include "utility.hpp"

double signed_area(vec2 a, vec2 b, vec2 c);
bool line_line_intersection(vec2 p0, vec2 v0, vec2 p1, vec2 v1, double &m0, double &m1);
vec2 rotate(vec2 v);
vec2 circumcircle_center(vec2 c0, vec2 c1, vec2 c2);
vec2 point(const PDT &trig, const period_point pnt);
vec2 point(const PDT &trig, const vertex_handle pnt);
