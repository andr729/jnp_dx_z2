﻿#include <d2d1_3.h>
#include <utility>
#include <tuple>
#include <cmath>
#include <vector>
#include <array>
#include "app.h"

// This line is needed on my local computer for some reason
// I suspose I have errors in linker configuration
#pragma comment(lib, "d2d1")

namespace {
	ID2D1Factory7* d2d_factory = nullptr;
	ID2D1HwndRenderTarget* d2d_render_target = nullptr;
	ID2D1SolidColorBrush* brush = nullptr;
	ID2D1SolidColorBrush* fill_brush = nullptr;

	// Radial gradient brush
	ID2D1RadialGradientBrush* rad_brush = nullptr;
	ID2D1GradientStopCollection* rad_stops = nullptr;
	UINT const NUM_RAD_STOPS = 2;
	D2D1_GRADIENT_STOP rad_stops_data[NUM_RAD_STOPS] = {
		{.position = 0.0f, .color = D2D1::ColorF(0.4f, 0.9f, 0.4f, 1.0f) },
		{.position = 1.0f, .color = D2D1::ColorF(0.0f, 0.3f, 0.0f, 1.0f) },
	};

	ID2D1RadialGradientBrush* eye_brush = nullptr;
	ID2D1GradientStopCollection* eye_rad_stops = nullptr;
	UINT const NUM_EYE_RAD_STOPS = 3;
	D2D1_GRADIENT_STOP rad_eye_stops_data[NUM_EYE_RAD_STOPS] = {
		{.position = 0.0f, .color = D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f) },
		{.position = 0.8f, .color = D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f) },
		{.position = 1.0f, .color = D2D1::ColorF(0.2f, 0.2f, 0.2f, 1.0f) },
	};

	// - Macierz do połączenia transformacji
	D2D1::Matrix3x2F base_transformation;
	D2D1::Matrix3x2F transformation;

	const D2D1_COLOR_F background_color =
	{ .r = 0.6f, .g = 0.2f, .b = 0.2f, .a = 1.0f };
	const D2D1_COLOR_F main_color =
	{ .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
	const D2D1_COLOR_F fill_color =
	{ .r = 0.1, .g = 0.6f, .b = 0.1f, .a = 1.0f };


	ID2D1PathGeometry* bear_geometry = nullptr;

	FLOAT window_size_x;
	FLOAT window_size_y;

	FLOAT mouse_x = 0;
	FLOAT mouse_y = 0;

	FLOAT base_x_offset = 400;
	FLOAT base_y_offset = 600;
}

D2D1_POINT_2F normalize(D2D1_POINT_2F p) {
	FLOAT len2 = p.x * p.x + p.y * p.y;
	FLOAT len = std::sqrtf(len2 + 1e-10);
	return { p.x / len, p.y / len };
}

void init(HWND hwnd) {
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
	if (d2d_factory == nullptr) {
		exit(1);
	}
	d2d_factory->CreatePathGeometry(&bear_geometry);
	if (bear_geometry == nullptr) {
		exit(1);
	}

	// x, y, control_point distance from previus point
	std::array<std::tuple<FLOAT, FLOAT, FLOAT>, 9> pre_points = { {
		{270, -20, 150},
		{100, -100, 120},
		{-130, -300, 100},
		{2, -2, 1},
		{70, -80, 80},
		{-30, -50, 40},
		{-110, 50, 70},
		{-3, -1, 2},
		{-169, -70, 80},
	} };

	std::vector<std::pair<D2D1_POINT_2F, D2D1_POINT_2F>> points;

	D2D1_POINT_2F pre_control_point = { -100, 0 };
	D2D1_POINT_2F control_point = pre_control_point;
	D2D1_POINT_2F previous_point = { 0, 0 };
	for (auto [x, y, b_scale] : pre_points) {
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

		points.push_back({ {control_point.x, control_point.y}, { current_point.x, current_point.y } });
		
		// evil version:
		//points.push_back({ {current_point.x, current_point.y}, { control_point.x, control_point.y } });

		previous_point = current_point;
	}

	ID2D1GeometrySink* g_sink;
	bear_geometry->Open(&g_sink);
	g_sink->BeginFigure({0, 0}, D2D1_FIGURE_BEGIN_FILLED);
	
	for (auto [control_point, point]: points) {
		g_sink->AddQuadraticBezier(
			{ {control_point.x, control_point.y}, { point.x, point.y } }
		);
	}
	for (int i = points.size() - 2; i >= 0; i--) {
		auto [control_point, _1] = points[i + 1];
		auto [_2, point] = points[i];
		g_sink->AddQuadraticBezier(
			{ {-control_point.x, control_point.y}, { -point.x, point.y } }
		);
	}
	g_sink->AddQuadraticBezier(
		{ {-points[0].first.x, points[0].first.y}, {0, 0}}
	);
	
	g_sink->EndFigure(D2D1_FIGURE_END_OPEN);
	g_sink->Close();

	recreateRenderTarget(hwnd);
}

void recreateRenderTarget(HWND hwnd) {
	RECT rc;
	GetClientRect(hwnd, &rc);

	window_size_x = static_cast<FLOAT>(rc.right - rc.left);
	window_size_y = static_cast<FLOAT>(rc.bottom - rc.top);

	d2d_factory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd,
			D2D1::SizeU(static_cast<UINT32>(rc.right) -
				static_cast<UINT32>(rc.left),
				static_cast<UINT32>(rc.bottom) -
				static_cast<UINT32>(rc.top))),
		&d2d_render_target);

	if (d2d_render_target == NULL) {
		exit(1);
	}

	if (brush) {
		brush->Release();
	}
	if (fill_brush) {
		fill_brush->Release();
	}
	d2d_render_target->CreateSolidColorBrush(main_color, &brush);
	d2d_render_target->CreateSolidColorBrush(fill_color, &fill_brush);

	// Utworzenie gradientu promienistego
	d2d_render_target->CreateGradientStopCollection(
		rad_stops_data, NUM_RAD_STOPS, &rad_stops);

	// Utworzenie gradientu promienistego
	d2d_render_target->CreateGradientStopCollection(
		rad_eye_stops_data, NUM_EYE_RAD_STOPS, &eye_rad_stops);

	if (rad_stops) {
		d2d_render_target->CreateRadialGradientBrush(
			D2D1::RadialGradientBrushProperties({0, -230}, {0, 0}, 500, 500),
			rad_stops, &rad_brush);
	}
	else {
		exit(1);
	}

	if (eye_rad_stops) {
		d2d_render_target->CreateRadialGradientBrush(
			D2D1::RadialGradientBrushProperties({ 0, 0 }, { 0, 0 }, 100, 100),
			eye_rad_stops, &eye_brush);
	}
	else {
		exit(1);
	}

}

void destroyRenderTarget() {
	if (d2d_render_target) {
		d2d_render_target->Release();
		d2d_render_target = nullptr;
	}
}

void destroy() {
	if (d2d_render_target) d2d_render_target->Release();
	if (d2d_factory) d2d_factory->Release();
}

void onMouseMove(FLOAT x, FLOAT y)  {
	mouse_x = x - base_x_offset;
	mouse_y = y - base_y_offset;
}

// changed transforamtion matrix
void draw_eye(ID2D1HwndRenderTarget* drt, FLOAT x, FLOAT y) {
	auto trn = base_transformation;
	trn.SetProduct(trn, D2D1::Matrix3x2F::Translation({ x, y }));

	d2d_render_target->SetTransform(trn);
	d2d_render_target->DrawEllipse(D2D1::Ellipse({ 0, 0 }, 100, 100), brush);
	d2d_render_target->FillEllipse(D2D1::Ellipse({ 0, 0 }, 100, 100), eye_brush);

	D2D1_POINT_2F mouse_vec = {mouse_x - x, mouse_y - y};
	mouse_vec = normalize(mouse_vec);
	mouse_vec.x *= 70;
	mouse_vec.y *= 70;

	d2d_render_target->FillEllipse(D2D1::Ellipse(mouse_vec, 30, 30), brush);
}

void onPaint(HWND hwnd) {
	if (!d2d_render_target) recreateRenderTarget(hwnd);

	d2d_render_target->BeginDraw();
	d2d_render_target->Clear(background_color);
	
	base_transformation = D2D1::Matrix3x2F::Translation({ base_x_offset, base_y_offset });

	d2d_render_target->SetTransform(base_transformation);

	d2d_render_target->DrawGeometry(bear_geometry, brush, 5);
	d2d_render_target->FillGeometry(bear_geometry, rad_brush);

	draw_eye(d2d_render_target, -150, -350);
	draw_eye(d2d_render_target, 150, -350);

	if (d2d_render_target->EndDraw() == D2DERR_RECREATE_TARGET) {
		destroyRenderTarget();
		onPaint(hwnd);
	}
}