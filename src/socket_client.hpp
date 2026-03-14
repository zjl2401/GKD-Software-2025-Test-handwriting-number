#pragma once

#include <vector>
#include <string>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

inline std::vector<float> socketForward(const std::string& host, int port,
                                        const std::vector<float>& input) {
    if (input.size() != 784) return {};
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return {};
#endif
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET) return {};
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#ifdef _WIN32
        closesocket(fd);
        WSACleanup();
#endif
        return {};
    }
    
    uint32_t dims[2] = {1, 784};
    send(fd, (const char*)dims, 8, 0);
    send(fd, (const char*)input.data(), 784 * 4, 0);
    
    uint32_t outLen;
    if (recv(fd, (char*)&outLen, 4, 0) != 4) {
#ifdef _WIN32
        closesocket(fd);
        WSACleanup();
#endif
        return {};
    }
    std::vector<float> result(outLen);
    recv(fd, (char*)result.data(), outLen * 4, 0);
#ifdef _WIN32
    closesocket(fd);
    WSACleanup();
#else
    close(fd);
#endif
    return result;
}
