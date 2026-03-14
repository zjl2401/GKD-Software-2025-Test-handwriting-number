#ifndef SOCKET_SERVER_HPP
#define SOCKET_SERVER_HPP

#include "model.h"
#include <memory>
#include <vector>
#include <cstdint>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <stdexcept>
#include <iostream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET sock_t;
    #define INVALID_SOCK INVALID_SOCKET
    #define sock_close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int sock_t;
    #define INVALID_SOCK (-1)
    #define sock_close close
#endif

// Socket 服务端：接收 [rows, cols, data] 格式的矩阵，调用 model.forward，返回 10 维概率
class SocketServer {
public:
    explicit SocketServer(ModelBase* model, int port = 12345)
        : model_(model), port_(port) {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
    }
    
    ~SocketServer() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    void run() {
        sock_t listenSock = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSock == INVALID_SOCK) {
            throw std::runtime_error("socket() failed");
        }
        
        int opt = 1;
#ifdef _WIN32
        setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
        setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
        
        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(static_cast<uint16_t>(port_));
        
        if (bind(listenSock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            sock_close(listenSock);
            throw std::runtime_error("bind() failed on port " + std::to_string(port_));
        }
        
        if (listen(listenSock, 5) != 0) {
            sock_close(listenSock);
            throw std::runtime_error("listen() failed");
        }
        
        std::cout << "Socket server listening on port " << port_ << std::endl;
        
        while (true) {
            sock_t client = accept(listenSock, nullptr, nullptr);
            if (client == INVALID_SOCK) {
                std::cerr << "accept() failed" << std::endl;
                continue;
            }
            handleClient(client);
            sock_close(client);
        }
        
        sock_close(listenSock);
    }

private:
    ModelBase* model_;
    int port_;
    
    static bool recvAll(sock_t sock, void* buf, size_t len) {
        char* p = static_cast<char*>(buf);
        while (len > 0) {
#ifdef _WIN32
            int n = recv(sock, p, static_cast<int>(len), 0);
#else
            ssize_t n = recv(sock, p, len, 0);
#endif
            if (n <= 0) return false;
            p += n;
            len -= static_cast<size_t>(n);
        }
        return true;
    }
    
    static bool sendAll(sock_t sock, const void* buf, size_t len) {
        const char* p = static_cast<const char*>(buf);
        while (len > 0) {
#ifdef _WIN32
            int n = send(sock, p, static_cast<int>(len), 0);
#else
            ssize_t n = send(sock, p, len, 0);
#endif
            if (n <= 0) return false;
            p += n;
            len -= static_cast<size_t>(n);
        }
        return true;
    }
    
    void handleClient(sock_t client) {
        int32_t rows, cols;
        if (!recvAll(client, &rows, 4) || !recvAll(client, &cols, 4)) {
            std::cerr << "Failed to receive rows/cols" << std::endl;
            return;
        }
        
        if (rows <= 0 || cols <= 0 || static_cast<size_t>(rows) * cols > 1000000) {
            std::cerr << "Invalid dimensions: " << rows << " x " << cols << std::endl;
            return;
        }
        
        size_t count = static_cast<size_t>(rows) * cols;
        std::vector<float> input(count);
        if (!recvAll(client, input.data(), count * sizeof(float))) {
            std::cerr << "Failed to receive matrix data" << std::endl;
            return;
        }
        
        try {
            auto t0 = std::chrono::high_resolution_clock::now();
            std::vector<float> output = model_->forward(input);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            std::cout << "forward: " << std::fixed << std::setprecision(2) << ms << " ms" << std::endl;
            
            int32_t outCount = static_cast<int32_t>(output.size());
            if (!sendAll(client, &outCount, 4)) {
                std::cerr << "Failed to send output count" << std::endl;
                return;
            }
            if (!sendAll(client, output.data(), output.size() * sizeof(float))) {
                std::cerr << "Failed to send output" << std::endl;
                return;
            }
        } catch (const std::exception& e) {
            std::cerr << "Forward error: " << e.what() << std::endl;
        }
    }
};

// Socket 客户端：发送 1x784 向量，接收 10 维概率
inline std::vector<float> socketForward(const std::string& host, int port,
                                        const std::vector<float>& input) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
    
    sock_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCK) {
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("socket() failed");
    }
    
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        sock_close(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("Invalid address: " + host);
    }
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        sock_close(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("connect() failed to " + host + ":" + std::to_string(port));
    }
    
    auto sendAll = [sock](const void* buf, size_t len) {
        const char* p = static_cast<const char*>(buf);
        while (len > 0) {
#ifdef _WIN32
            int n = send(sock, p, static_cast<int>(len), 0);
#else
            ssize_t n = send(sock, p, len, 0);
#endif
            if (n <= 0) return false;
            p += n;
            len -= static_cast<size_t>(n);
        }
        return true;
    };
    
    auto recvAll = [sock](void* buf, size_t len) {
        char* p = static_cast<char*>(buf);
        while (len > 0) {
#ifdef _WIN32
            int n = recv(sock, p, static_cast<int>(len), 0);
#else
            ssize_t n = recv(sock, p, len, 0);
#endif
            if (n <= 0) return false;
            p += n;
            len -= static_cast<size_t>(n);
        }
        return true;
    };
    
    int32_t rows = 1;
    int32_t cols = static_cast<int32_t>(input.size());
    if (!sendAll(&rows, 4) || !sendAll(&cols, 4) || !sendAll(input.data(), input.size() * sizeof(float))) {
        sock_close(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("send failed");
    }
    
    int32_t outCount;
    if (!recvAll(&outCount, 4) || outCount <= 0 || outCount > 100) {
        sock_close(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("recv count failed");
    }
    
    std::vector<float> result(outCount);
    if (!recvAll(result.data(), result.size() * sizeof(float))) {
        sock_close(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("recv data failed");
    }
    
    sock_close(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return result;
}

#endif // SOCKET_SERVER_HPP
