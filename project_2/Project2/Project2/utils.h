#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d2d1_3.h>
#include <array>
#include <tuple>
#include <vector>

FLOAT vecLen(D2D1_POINT_2F p);
D2D1_POINT_2F normalize(D2D1_POINT_2F p);

template<int n>
using BezierDefinition = std::array<std::tuple<FLOAT, FLOAT, FLOAT>, n>;

using BezierPoints = std::vector<std::pair<D2D1_POINT_2F, D2D1_POINT_2F>>;

/**
* Allways start from (0, 0), and control point parallel to y-axis.
* If y_symetry == true then curve symetric to x-axis will be added % point in between.
* 
* Evil is flag that changes control points with normal points -- inspiration from a bug,
* can create interesting visual effects
*/
template<int n>
BezierPoints makeBezierPoints(BezierDefinition<n> definition, bool x_symetry, bool evil = false) {
	BezierPoints out;

	D2D1_POINT_2F pre_control_point = { -100, 0 };
	D2D1_POINT_2F control_point = pre_control_point;
	D2D1_POINT_2F previous_point = { 0, 0 };
	for (auto [x, y, b_scale] : definition) {
		D2D1_POINT_2F current_point = {
			x + previous_point.x,
			y + previous_point.y,
		};

		D2D1_POINT_2F control_vector = {
			previous_point.x - control_point.x,
			previous_point.y - control_point.y
		};

		// normalize vector:
		control_vector = normalize(control_vector);
		control_point = {
			previous_point.x + control_vector.x * b_scale,
			previous_point.y + control_vector.y * b_scale
		};


		if (!evil) {
			out.push_back({ {control_point.x, control_point.y}, { current_point.x, current_point.y } });
		}
		else {
			out.push_back({ {current_point.x, current_point.y}, { control_point.x, control_point.y } });
		}

		previous_point = current_point;
	}

	if (x_symetry) {
		auto org_size = out.size();
		for (int i = org_size - 2; i >= 0; i--) {
			auto [control_point, _1] = out[i + 1];
			auto [_2, point] = out[i];
			out.push_back(
				{ {-control_point.x, control_point.y}, { -point.x, point.y } }
			);
		}
		out.push_back(
			{ {-out[0].first.x, out[0].first.y}, {0, 0} }
		);
	}

	return out;
}





