#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const int BUFFER_SIZE = 1024;

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Could not create socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);

    // Convert localhost IP to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        std::cerr << "Error: Invalid address or address not supported." << std::endl;
        close(clientSocket);
        return 1;
    }

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error: Connection failed." << std::endl;
        close(clientSocket);
        return 1;
    }

    // Send HTTP GET request
    std::string request = "GET / HTTP/1.1\\r\\nHost: localhost\\r\\nConnection: close\\r\\n\\r\\n";
    if (send(clientSocket, request.c_str(), request.size(), 0) < 0) {
        std::cerr << "Error: Failed to send request." << std::endl;
        close(clientSocket);
        return 1;
    }

    // Receive and print the response
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = 0;
    while ((bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesRead] = '\\0';  // Null-terminate the buffer
        std::cout << buffer;
    }

    if (bytesRead < 0) {
        std::cerr << "Error: Failed to read response from server." << std::endl;
    }

    close(clientSocket);  // Close the socket
    return 0;
}
