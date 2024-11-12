#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <string>
#include <map>
#include <mutex>
#include "Cache.h"

class ProxyServer {
public:
    ProxyServer(const std::string& ip, int port);
    void run();
private:
    std::string ipAddress;
    int port;
    Cache cache;

    void handleClient(int clientSocket);
    std::string getWebResponse(const std::string& url);
};

#endif // PROXY_SERVER_H
