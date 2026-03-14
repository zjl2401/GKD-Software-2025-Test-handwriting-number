#pragma once

#include "model.hpp"
#include <memory>
#include <string>
#include <vector>
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

class SocketServer {
public:
    SocketServer(std::unique_ptr<ModelBase> model, int port = 12345)
        : model_(std::move(model)), port_(port) {}
    
    void run() {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return;
#endif
        SOCKET listenFd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenFd == INVALID_SOCKET) return;
        
        int opt = 1;
        setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
        
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons((uint16_t)port_);
        
        if (bind(listenFd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) return;
        listen(listenFd, 5);
        
        while (true) {
            sockaddr_in clientAddr{};
            socklen_t len = sizeof(clientAddr);
            SOCKET clientFd = accept(listenFd, (sockaddr*)&clientAddr, &len);
            if (clientFd == INVALID_SOCKET) continue;
            
            uint32_t dims[2];
            if (recv(clientFd, (char*)dims, 8, 0) != 8) {
                closeSocket(clientFd);
                continue;
            }
            size_t rows = dims[0], cols = dims[1];
            if (rows * cols != 784) {
                closeSocket(clientFd);
                continue;
            }
            
            std::vector<float> data(784);
            if (recv(clientFd, (char*)data.data(), 784 * 4, 0) != 784 * 4) {
                closeSocket(clientFd);
                continue;
            }
            
            auto result = model_->forward(data);
            uint32_t outLen = (uint32_t)result.size();
            send(clientFd, (const char*)&outLen, 4, 0);
            send(clientFd, (const char*)result.data(), result.size() * 4, 0);
            closeSocket(clientFd);
        }
#ifdef _WIN32
        closesocket(listenFd);
        WSACleanup();
#else
        close(listenFd);
#endif
    }
    
private:
    void closeSocket(SOCKET fd) {
#ifdef _WIN32
        closesocket(fd);
#else
        close(fd);
#endif
    }
    std::unique_ptr<ModelBase> model_;
    int port_;
};
