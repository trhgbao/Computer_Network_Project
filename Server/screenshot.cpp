#include "screenshot.h"

#define _CRT_SECURE_NO_WARNINGS
using namespace std;

// Hàm lưu ảnh chụp màn hình thành file BMP
void SaveBitmap(HBITMAP hBitmap, HDC hdc, int width, int height, const char* filepath) {
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height; // Hình ảnh ngược theo trục y
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	int dataSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
	char* data = new char[dataSize];

	BITMAPINFO biInfo;
	biInfo.bmiHeader = bi;

	GetDIBits(hdc, hBitmap, 0, (UINT)height, data, &biInfo, DIB_RGB_COLORS);

	FILE* file;
	errno_t err = fopen_s(&file, filepath, "wb");
	if (file) {
		BITMAPFILEHEADER bf;
		bf.bfType = 0x4D42; // 'BM'
		bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataSize;
		bf.bfReserved1 = 0;
		bf.bfReserved2 = 0;
		bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, file);
		fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, file);
		fwrite(data, dataSize, 1, file);
		fclose(file);
	}

	delete[] data;
}

// Hàm chụp màn hình
void CaptureScreen(const char* filepath) {
	// Lấy kích thước màn hình
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Tạo thiết bị ngữ cảnh (device context)
	HDC hScreenDC = GetDC(NULL);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

	// Tạo bitmap tương thích
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
	SelectObject(hMemoryDC, hBitmap);

	// Copy dữ liệu từ màn hình vào bitmap
	BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

	// Lưu bitmap thành file BMP
	SaveBitmap(hBitmap, hMemoryDC, screenWidth, screenHeight, filepath);

	// Giải phóng tài nguyên
	DeleteObject(hBitmap);
	DeleteDC(hMemoryDC);
	ReleaseDC(NULL, hScreenDC);
}

void send_imp(SOCKET client_sock) {
	// Đọc file ảnh
	ifstream imageFile("screenshot.bmp", ios::binary | ios::ate);

	// Lấy kích thước file
	streamsize imageSize = imageFile.tellg();
	imageFile.seekg(0, ios::beg);
	char* imageData = new char[imageSize];

	// Đọc dữ liệu file vào buffer
	if (!imageFile.read(imageData, imageSize)) {
		cout << "Could not read image file." << endl;
		delete[] imageData;
		return;
	}

	// Gửi kích thước file ảnh
	send(client_sock, (char*)&imageSize, sizeof(imageSize), 0);
	// Gửi dữ liệu ảnh
	send(client_sock, imageData, imageSize, 0);

	cout << "Image sent successfully!" << endl;
	return;
}