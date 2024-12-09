#include "lib.h"
#include "screenshot.h"
#include "list_app.h"

#pragma comment(lib, "ws2_32.lib") // Link thư viện Winsock
using namespace std;


int main() {
	WSADATA wsa;

	// Khởi tạo Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "WSAStartup failed. Error: " << WSAGetLastError() << endl;
		return 1;
	}

	cout << "Winsock initialized successfully!" << endl;

	SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock == INVALID_SOCKET) {
		cout << "Socket creation failed. Error: " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY; // Chấp nhận kết nối từ mọi địa chỉ
	server.sin_port = htons(8080); // Cổng server

	// Gắn địa chỉ và cổng vào socket
	if (bind(server_sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		cout << "Bind failed. Error: " << WSAGetLastError() << endl;
		closesocket(server_sock);
		WSACleanup();
		return 1;
	}

	// Đặt socket vào trạng thái lắng nghe
	if (listen(server_sock, 3) == SOCKET_ERROR) {
		cout << "Listen failed. Error: " << WSAGetLastError() << endl;
		closesocket(server_sock);
		WSACleanup();
		return 1;
	}

	cout << "Waiting for incoming connections..." << endl;

	// Chấp nhận kết nối từ client
	SOCKET client_sock;
	struct sockaddr_in client;
	int c = sizeof(struct sockaddr_in);
	client_sock = accept(server_sock, (struct sockaddr*)&client, &c);
	if (client_sock == INVALID_SOCKET) {
		cout << "Accept failed. Error: " << WSAGetLastError() << endl;
		closesocket(server_sock);
		WSACleanup();
		return 1;
	}

	cout << "Connection accepted!" << endl;

	char buffer[1024];
	char mess[1024];
	string a;
	cout << "Chat with client (type 'exit' to quit):" << endl;

	// Vòng lặp nhận và gửi tin nhắn
	memset(buffer, 0, sizeof(buffer));
	int len = recv(client_sock, buffer, sizeof(buffer), 0);
	if (len <= 0) {
		cout << "Client disconnected or recv error." << endl;
	}
	buffer[len] = '\0';
	cout << "Client: " << buffer << endl;
	if (strcmp(buffer, "shutdown") == 0) {
		system("shutdown /s /t 1");
	}
	else if (strcmp(buffer, "screenshot") == 0) {
		CaptureScreen("screenshot.bmp");
		send(client_sock, "send_imp", 8, 0);
		send_imp(client_sock);
	}
	else if (strcmp(buffer, "turn_on_webcam") == 0) {
		system("powershell -Command \"Get-PnpDevice | Where-Object { $_.Class -eq 'Image' } | Enable-PnpDevice -Confirm:$false\"");
	}
	else if (strcmp(buffer, "turn_off_webcam") == 0) {
		system("powershell -Command \"Get-PnpDevice | Where-Object { $_.Class -eq 'Image' } | Disable-PnpDevice -Confirm:$false\"");
	}
	else if (strcmp(buffer, "list_app") == 0) {
		send(client_sock, "list_app", 8, 0);
		List_App();
		memset(buffer, 0, sizeof(buffer));
		len = recv(client_sock, buffer, sizeof(buffer), 0);
		buffer[len] = '\0';
		cout << buffer;
		string cm = string(buffer).substr(0, string(buffer).find(' '));
		string app_name = string(buffer).substr(string(buffer).find(' '));
		cout << cm << " " << app_name;
		a = GetAppPathFromRegistry(app_name);
		if (cm != "start" || cm != "stop" || a == "") {
			return 0;
		}
		if (cm == "start") {
			std::wstring wideA(a.begin(), a.end());
			ShellExecute(NULL, L"open", wideA.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		else if (cm == "stop") {
			stop_app(app_name);
		}
		send_txt(client_sock, "list_app");
	}
	else if (strcmp(buffer, "list_service") == 0) {
		send(client_sock, "list_service", 12, 0);
		List_Services();
		send_txt(client_sock, "list_service");
		memset(buffer, 0, sizeof(buffer));
		a = GetAppPathFromRegistry(buffer);

	}

	// Đóng socket và dọn dẹp Winsock
	closesocket(client_sock);
	closesocket(server_sock);
	WSACleanup();
	return 0;
}