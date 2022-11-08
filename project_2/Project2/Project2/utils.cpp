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


namespace detail{
	void auxMakeRadBrush(ID2D1RadialGradientBrush * *brush,
					  ID2D1HwndRenderTarget * drt, D2D1_GRADIENT_STOP * stop_data,
					  int num_rad_stops, ID2D1GradientStopCollection * *stops,
					  D2D1_POINT_2F center, D2D1_POINT_2F offset, FLOAT rx, FLOAT ry) {
		drt->CreateGradientStopCollection(
			stop_data, num_rad_stops, stops);

		if (stops) {
			drt->CreateRadialGradientBrush(
				D2D1::RadialGradientBrushProperties(center, offset, rx, ry),
				*stops, brush);
		}
		else {
			exit(1);
		}
	}
}

