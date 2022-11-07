#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void init(HWND hwnd);
void recreateRenderTarget(HWND hwnd);
void destroyRenderTarget();
void destroy();
void onPaint(HWND hwnd);
void onMouseMove(FLOAT x, FLOAT y);
