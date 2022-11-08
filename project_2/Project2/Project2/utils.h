#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d2d1_3.h>
#include <array>
#include <tuple>
#include <vector>

FLOAT vecLen(D2D1_POINT_2F p);
D2D1_POINT_2F normalize(D2D1_POINT_2F p);

template<UINT NUM_STOPS>
struct RadBrushState {
	ID2D1RadialGradientBrush* brush = nullptr;
	ID2D1GradientStopCollection* stops = nullptr;
	D2D1_GRADIENT_STOP stops_data[NUM_STOPS];
};


namespace detail {
	void auxMakeRadBrush(ID2D1RadialGradientBrush** brush,
			   	  ID2D1HwndRenderTarget* drt, D2D1_GRADIENT_STOP* stop_data,
                  int num_rad_stops, ID2D1GradientStopCollection** stops,
                  D2D1_POINT_2F center, D2D1_POINT_2F offset, FLOAT rx, FLOAT ry);
}

template<UINT NUM_STOPS>
void makeRadBrush(RadBrushState<NUM_STOPS>& state, ID2D1HwndRenderTarget* drt,
                  D2D1_POINT_2F center, FLOAT rx, FLOAT ry) {
	detail::auxMakeRadBrush(
		&state.brush,
		drt, state.stops_data, NUM_STOPS, &state.stops,
		center, {0, 0},
		rx, ry
	);
}


template<int n>
using BezierDefinition = std::array<std::tuple<FLOAT, FLOAT, FLOAT>, n>;

using BezierPoints = std::vector<std::pair<D2D1_POINT_2F, D2D1_POINT_2F>>;

void makeID2D1PathGeometry(ID2D1PathGeometry** out, const BezierPoints& points);

/**
* Allways start from (0, 0), and control point parallel to y-axis.
* If y_symetry == true then curve symetric to x-axis will be added % point in between.
* 
* Evil should be between 0 and 1 and it continuously changes control points with normal points -- inspiration from a bug,
* can create interesting visual effects
*/
template<int n>
BezierPoints makeBezierPoints(BezierDefinition<n> definition, bool x_symetry, float evil = 0.f) {
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


		if (evil == 0.f) {
			out.push_back({ {control_point.x, control_point.y}, { current_point.x, current_point.y } });
		}
		else {
			auto cur_p_x = current_point.x + (control_point.x - current_point.x) * evil;
			auto cur_p_y = current_point.y + (control_point.y - current_point.y) * evil;
			
			auto cp_p_x = control_point.x + (current_point.x - control_point.x) * evil;
			auto cp_p_y = control_point.y + (current_point.y - control_point.y) * evil;
			out.push_back({ {cp_p_x, cp_p_y}, { cur_p_x, cur_p_y } });
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





