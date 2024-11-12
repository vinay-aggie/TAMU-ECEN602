#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <unordered_map>
#include <list>
#include <ctime>

struct CacheEntry {
    std::string data;
    std::time_t lastAccess;
    std::time_t expiration;
};

class Cache {
public:
    Cache(int maxEntries);
    std::string get(const std::string& url);
    void put(const std::string& url, const std::string& data, std::time_t expiration);
private:
    int maxEntries;
    std::unordered_map<std::string, std::list<std::string>::iterator> cacheMap;
    std::list<std::string> lruList;
};

#endif // CACHE_H
