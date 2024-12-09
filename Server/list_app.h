#pragma once
#include "lib.h"

void List_App();

void List_Services();

std::string GetAppPathFromRegistry(const std::string& appName);

DWORD GetProcessIdByName(const std::string& processName);

void stop_app(const std::string& processName);

void send_txt(SOCKET clientSocket, std::string a);