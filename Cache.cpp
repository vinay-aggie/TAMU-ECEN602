#include "Cache.h"

Cache::Cache(int maxEntries) : maxEntries(maxEntries) {}

std::string Cache::get(const std::string& url) {
    // Retrieve data if exists and is fresh
    return "";
}

void Cache::put(const std::string& url, const std::string& data, std::time_t expiration) {
    // Store data with LRU logic
}
