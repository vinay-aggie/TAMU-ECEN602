#include <iostream>
#include <string>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "httpUtils.h"

const int BUFFER_SIZE = 1024;

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead < 0) {
        std::cerr << "Error: Failed to read from client socket." << std::endl;
        close(clientSocket);
        return;
    }
    
    std::map<std::string, std::string> headers;
    std::string body;
    std::string httpRequest(buffer, bytesRead);

    if (!parseHttp(httpRequest, headers, body)) {
        std::cerr << "Error: Failed to parse HTTP request." << std::endl;
        close(clientSocket);
        return;
    }

    // Example Response
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!";
    if (send(clientSocket, response.c_str(), response.size(), 0) < 0) {
        std::cerr << "Error: Failed to send response to client." << std::endl;
    }

    close(clientSocket);  // Free the socket resource
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error: Could not create server socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error: Bind failed." << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error: Listen failed." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);
        if (clientSocket < 0) {
            std::cerr << "Error: Accept failed." << std::endl;
            continue;
        }

        handleClient(clientSocket);
    }

    close(serverSocket);
    return 0;
}
