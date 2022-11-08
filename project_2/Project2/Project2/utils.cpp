#include "utils.h"
#include <cmath>

FLOAT vecLen(D2D1_POINT_2F p) {
	FLOAT len2 = p.x * p.x + p.y * p.y;
	FLOAT len = std::sqrtf(len2 + 1e-10);
	return len;
}

D2D1_POINT_2F normalize(D2D1_POINT_2F p) {
	auto len = vecLen(p);
	return { p.x / len, p.y / len };
}