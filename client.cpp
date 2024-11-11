#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

std::string getRequest(std::string url) {
    int length = url.size();
    std::string request = "GET ";

    int i = 0;
    if (url.find("http://") != std::string::npos) {
        i += 7;
    }

    int slash = url.find("/", i);

    std::string domain = url.substr(0, slash);
    std::string path = url.substr(slash);

    request += path + " HTTP/1.0\r\n";
    request += "Host: " + domain + "\r\n\r\n";

    std::cout << request << std::endl;

    return request;
}

void downloadFile(const std::string& proxy_host, int proxy_port, const std::string& url) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(proxy_port);
    server_addr.sin_addr.s_addr = inet_addr(proxy_host.c_str());

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to proxy server" << std::endl;
        close(sock);
        return;
    }

    std::cout << "Requesting " << url << std::endl;

    std::string request = getRequest(url);
    //std::string request = "GET " + url + " HTTP/1.0\r\n\r\n";
    send(sock, request.c_str(), request.size(), 0);

    char buffer[1024];
    std::string response;
    int bytes_read;
    while ((bytes_read = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, bytes_read);
    }

    close(sock);

    // Save the response to a file
    std::ofstream outfile(url.substr(url.find_last_of("/") + 1), std::ios::binary);
    outfile.write(response.c_str(), response.size());
    std::cout << "File saved successfully." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <proxy_host> <proxy_port> <url>" << std::endl;
        return -1;
    }

    std::string proxy_host = argv[1];
    int proxy_port = std::stoi(argv[2]);
    std::string url = argv[3];

    downloadFile(proxy_host, proxy_port, url);

    return 0;
}
