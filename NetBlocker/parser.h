#pragma once
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <iostream>
#include <sstream>
#include <regex>

#pragma comment(lib, "winhttp.lib")

inline float getVersionFromURL(const std::wstring& url) {

    std::wregex urlRegex(LR"(https://([^/]+)(/.*))");
    std::wsmatch matches;
    if (!std::regex_match(url, matches, urlRegex) || matches.size() < 3) {
        std::wcerr << L"Invalid URL format\n";
        return 0.0f;
    }
    std::wstring host = matches[1];
    std::wstring path = matches[2];

    // Initialize WinHTTP session
    HINTERNET hSession = WinHttpOpen(L"NetBlocker Version Fetcher/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        std::wcerr << L"Failed WinHttpOpen\n";
        return 0.0f;
    }

    // Connect to host
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        std::wcerr << L"Failed WinHttpConnect\n";
        WinHttpCloseHandle(hSession);
        return 0.0f;
    }

    // Open HTTPS GET request
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        std::wcerr << L"Failed WinHttpOpenRequest\n";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0.0f;
    }

    // Send request
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        std::wcerr << L"Failed WinHttpSendRequest\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0.0f;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        std::wcerr << L"Failed WinHttpReceiveResponse\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0.0f;
    }

    // Read response data
    std::string content;
    DWORD dwSize = 0;
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            std::wcerr << L"Error in WinHttpQueryDataAvailable\n";
            break;
        }
        if (dwSize == 0) break;

        char* buffer = new char[dwSize + 1];
        ZeroMemory(buffer, dwSize + 1);

        DWORD dwDownloaded = 0;
        if (!WinHttpReadData(hRequest, (LPVOID)buffer, dwSize, &dwDownloaded)) {
            std::wcerr << L"Error in WinHttpReadData\n";
            delete[] buffer;
            break;
        }

        content.append(buffer, dwDownloaded);
        delete[] buffer;
    } while (dwSize > 0);

    // Cleanup WinHTTP handles
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // Parse content line by line to find the version float
    std::istringstream iss(content);
    std::string line;
    std::regex versionRegex(R"(inline\s+float\s+version\s*=\s*([0-9]*\.?[0-9]+)f?;)");
    while (std::getline(iss, line)) {
        std::smatch match;
        if (std::regex_search(line, match, versionRegex)) {
            if (match.size() > 1) {
                try {
                    float version = std::stof(match[1].str());
                    return version;
                }
                catch (...) {
                    std::cerr << "Failed to parse version float\n";
                    return 0.0f;
                }
            }
        }
    }

    std::cerr << "Version not found in file\n";
    return 0.0f;
}
