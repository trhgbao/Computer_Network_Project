#include "list_app.h"

void List_App() {
    HKEY hKey;
    const std::string uninstallKeys[] = {
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", // HKLM
        "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall" // HKLM for 32-bit apps on 64-bit Windows
    };

    // Mở file để ghi kết quả
    std::ofstream outFile("list_app.txt");
    outFile << "Danh sách các ứng dụng đã được cài đặt trên máy:\n";

    // Duyệt qua các đường dẫn trong Registry
    for (const auto& uninstallKey : uninstallKeys) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, uninstallKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char subKeyName[256];
            DWORD subKeyNameSize = 256;
            DWORD index = 0;

            // Duyệt qua từng subkey trong đường dẫn Uninstall
            while (RegEnumKeyExA(hKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                HKEY hSubKey;
                std::string subKeyPath = uninstallKey + "\\" + subKeyName;

                // Mở subkey
                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKeyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    char displayName[256];
                    DWORD displayNameSize = 256;

                    // Đọc giá trị "DisplayName"
                    if (RegQueryValueExA(hSubKey, "DisplayName", NULL, NULL, (LPBYTE)displayName, &displayNameSize) == ERROR_SUCCESS) {
                        outFile << displayName << std::endl;
                    }

                    RegCloseKey(hSubKey);
                }

                subKeyNameSize = 256;
                index++;
            }

            RegCloseKey(hKey);
        }
    }

    outFile.close();
    std::cout << "Đã lưu danh sách ứng dụng vào file list_app.txt" << std::endl;
}

void List_Services() {
    HRESULT hres;

    // Khởi tạo COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        std::cout << "COM initialization failed." << std::endl;
        return;
    }

    // Thiết lập bảo mật COM
    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
        EOAC_NONE, NULL);

    if (FAILED(hres)) {
        CoUninitialize();
        std::cout << "Security initialization failed." << std::endl;
        return;
    }

    IWbemLocator* pLoc = NULL;

    // Tạo đối tượng Locator WMI
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        CoUninitialize();
        std::cout << "WMI locator creation failed." << std::endl;
        return;
    }

    IWbemServices* pSvc = NULL;

    // Kết nối tới namespace WMI
    hres = pLoc->ConnectServer(
        BSTR(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL, 0, 0, &pSvc);

    if (FAILED(hres)) {
        pLoc->Release();
        pLoc = nullptr;
        CoUninitialize();
        std::cout << "WMI connection failed." << std::endl;
        return;
    }

    // Thiết lập bảo mật WMI
    hres = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);

    if (FAILED(hres)) {
        pSvc->Release();
        pSvc = nullptr;
        pLoc->Release();
        pLoc = nullptr;
        CoUninitialize();
        std::cout << "Proxy blanket failed." << std::endl;
        return;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        BSTR(L"WQL"),
        BSTR(L"SELECT * FROM Win32_Service"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator);

    if (FAILED(hres)) {
        pSvc->Release();
        pSvc = nullptr;
        pLoc->Release();
        pLoc = nullptr;
        CoUninitialize();
        std::cout << "Query for services failed." << std::endl;
        return;
    }

    // Mở file để ghi danh sách dịch vụ
    std::ofstream outFile("list_service.txt");

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    // Ghi dữ liệu ra file
    outFile << "Danh sách các dịch vụ trên hệ thống:\n";
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (FAILED(hr) || uReturn == 0) {
            std::cout << "Hoàn thành việc liệt kê dịch vụ." << std::endl;
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
            outFile << "Service: " << (char*)_bstr_t(vtProp.bstrVal) << std::endl;
        }
        VariantClear(&vtProp);

        pclsObj->Release();
        pclsObj = nullptr;
    }

    // Đóng file
    outFile.close();

    // Dọn dẹp
    if (pEnumerator) {
        pEnumerator->Release();
        pEnumerator = nullptr;
    }
    if (pSvc) {
        pSvc->Release();
        pSvc = nullptr;
    }
    if (pLoc) {
        pLoc->Release();
        pLoc = nullptr;
    }

    // Hủy COM
    CoUninitialize();
}

std::string GetAppPathFromRegistry(const std::string& appName) {
    HKEY hKey;
    wchar_t path[MAX_PATH];
    DWORD pathSize = MAX_PATH;

    // Chuyển đổi std::string (ASCII) thành std::wstring (Unicode)
    std::wstring appNameW = std::wstring(appName.begin(), appName.end());

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, appNameW.c_str(), NULL, NULL, (LPBYTE)path, &pathSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::string(path, path + wcslen(path)); // Chuyển ngược lại sang std::string
        }
        RegCloseKey(hKey);
    }
    return ""; // Không tìm thấy
}

DWORD GetProcessIdByName(const std::string& processName) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Chuyển `processName` sang `std::wstring`
    std::wstring wideProcessName(processName.begin(), processName.end());

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            // So sánh chuỗi Unicode
            if (_wcsicmp(pe32.szExeFile, wideProcessName.c_str()) == 0) {
                DWORD pid = pe32.th32ProcessID;
                CloseHandle(hProcessSnap);
                return pid;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
    return 0;
}

void stop_app(const std::string& processName) {
    DWORD pid = GetProcessIdByName(processName);
    if (pid == 0) {
        std::cout << "Không tìm thấy ứng dụng: " << processName << std::endl;
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess) {
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
        std::cout << "Đã đóng ứng dụng: " << processName << std::endl;
    }
    else {
        std::cout << "Không thể mở tiến trình để kết thúc." << std::endl;
    }
}


void send_txt(SOCKET clientSocket, std::string a) {
    std::fstream file;
    if (a == "list_app") {
        file.open("list_app.txt", std::ios::in | std::ios::binary);
    }
    else if (a == "list_service") {
        file.open("list_service.txt", std::ios::in | std::ios::binary);
    }
	char buffer[1024];
	while (!file.eof()) {
		file.read(buffer, 1024);
		int bytesRead = file.gcount();
		send(clientSocket, buffer, bytesRead, 0);
	}
	file.close();
}