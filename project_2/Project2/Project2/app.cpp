﻿#include <d2d1_3.h>
#include "app.h"
#include "utils.h"
#include "matrix.h"

// This line is needed on my local computer for some reason
// I suspose I have errors in linker configuration
#pragma comment(lib, "d2d1")

namespace {
	ID2D1Factory7* d2d_factory = nullptr;
	ID2D1HwndRenderTarget* d2d_render_target = nullptr;
	ID2D1SolidColorBrush* brush = nullptr;
	ID2D1SolidColorBrush* fill_brush = nullptr;
	ID2D1SolidColorBrush* nouse_brush = nullptr;

	RadBrushState<2> bear_brush = {
		nullptr, nullptr,
		{
			{.position = 0.0f, .color = D2D1::ColorF(0.4f, 0.9f, 0.4f, 1.0f) },
			{.position = 1.0f, .color = D2D1::ColorF(0.0f, 0.3f, 0.0f, 1.0f) },
		}
	};

	RadBrushState<3> eye_brush = {
		nullptr, nullptr,
		{
			{.position = 0.0f, .color = D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f) },
			{.position = 0.8f, .color = D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f) },
			{.position = 1.0f, .color = D2D1::ColorF(0.2f, 0.2f, 0.2f, 1.0f) },
		}
	};

	constexpr D2D1_COLOR_F background_color =
		{ .r = 0.6f, .g = 0.2f, .b = 0.2f, .a = 1.0f };
	constexpr D2D1_COLOR_F main_color =
		{ .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
	constexpr D2D1_COLOR_F fill_color =
		{ .r = 0.1, .g = 0.6f, .b = 0.1f, .a = 1.0f };
	constexpr D2D1_COLOR_F nouse_color =
		{ .r = 0.2, .g = 0.2f, .b = 0.2f, .a = 1.0f };


	ID2D1PathGeometry* bear_geometry = nullptr;
	ID2D1PathGeometry* nouse_geometry = nullptr;

	Matrix3 base_transformation;

	FLOAT window_size_x;
	FLOAT window_size_y;

	FLOAT mouse_x = 0;
	FLOAT mouse_y = 0;

	FLOAT base_x_offset = 400;
	FLOAT base_y_offset = 600;
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
	d2d_factory->CreatePathGeometry(&nouse_geometry);
	if (nouse_geometry == nullptr) {
		exit(1);
	}

	// x, y, control_point distance from previus point
	BezierDefinition<9> bear_pre_points = { {
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
	BezierPoints bear_points = makeBezierPoints(bear_pre_points, true, false);
	makeID2D1PathGeometry(&bear_geometry, bear_points);

	BezierDefinition<3> nouse_pre_points = { {
		{60, -10, 40},
		{-20, -50, 20},
		{-40, -20, 27},
	} };
	BezierPoints nouse_points = makeBezierPoints(nouse_pre_points, true, false);
	makeID2D1PathGeometry(&nouse_geometry, nouse_points);


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
			D2D1::SizeU(
				static_cast<UINT32>(rc.right) -
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
	if (nouse_brush) {
		nouse_brush->Release();
	}
	d2d_render_target->CreateSolidColorBrush(main_color, &brush);
	d2d_render_target->CreateSolidColorBrush(fill_color, &fill_brush);
	d2d_render_target->CreateSolidColorBrush(nouse_color, &nouse_brush);

	makeRadBrush<2>(
		bear_brush, d2d_render_target,
		{ 0, -230 }, 500, 500
	);

	makeRadBrush<3>(
		eye_brush, d2d_render_target,
		{ 0, 0 }, 100, 100
	);

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

// changes transforamtion matrix
void draw_eye(ID2D1HwndRenderTarget* drt, FLOAT x, FLOAT y) {
	auto trn = base_transformation;
	trn *= D2D1::Matrix3x2F::Translation({ x, y });

	d2d_render_target->SetTransform(trn.getInner());
	d2d_render_target->DrawEllipse(D2D1::Ellipse({ 0, 0 }, 100, 100), brush);
	d2d_render_target->FillEllipse(D2D1::Ellipse({ 0, 0 }, 100, 100), eye_brush.brush);

	D2D1_POINT_2F mouse_vec = {mouse_x - x, mouse_y - y};
	auto dist = vecLen(mouse_vec);
	if (dist > 70) {
		mouse_vec = normalize(mouse_vec);
		mouse_vec.x *= 70;
		mouse_vec.y *= 70;
	}

	d2d_render_target->FillEllipse(D2D1::Ellipse(mouse_vec, 30, 30), brush);
}

void onPaint(HWND hwnd) {
	if (!d2d_render_target) recreateRenderTarget(hwnd);

	d2d_render_target->BeginDraw();
	d2d_render_target->Clear(background_color);
	
	base_transformation = D2D1::Matrix3x2F::Translation({ base_x_offset, base_y_offset });

	// bear:
	d2d_render_target->SetTransform(base_transformation.getInner());
	d2d_render_target->DrawGeometry(bear_geometry, brush, 5);
	d2d_render_target->FillGeometry(bear_geometry, bear_brush.brush);

	// eyes:
	draw_eye(d2d_render_target, -150, -350);
	draw_eye(d2d_render_target, 150, -350);

	// nouse:
	Matrix3 trn = D2D1::Matrix3x2F::Translation({ 0, -200 });
	trn *= base_transformation;
	d2d_render_target->SetTransform(trn.getInner());
	
	d2d_render_target->DrawGeometry(nouse_geometry, brush, 5);
	d2d_render_target->FillGeometry(nouse_geometry, nouse_brush);

	if (d2d_render_target->EndDraw() == D2DERR_RECREATE_TARGET) {
		destroyRenderTarget();
		onPaint(hwnd);
	}
}
