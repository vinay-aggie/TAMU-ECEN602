#include "ProxyServer.h"
#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

ProxyServer::ProxyServer(const std::string& ip, int port)
    : ipAddress(ip), port(port) {}

void ProxyServer::run() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    serverAddr.sin_port = htons(port);

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 10);
    std::cout << "Proxy server running on " << ipAddress << ":" << port << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        std::thread(&ProxyServer::handleClient, this, clientSocket).detach();
    }
}

void ProxyServer::handleClient(int clientSocket) {
    // Logic to handle HTTP request, retrieve from cache or forward to web server
    close(clientSocket);
}
