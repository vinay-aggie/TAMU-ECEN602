
#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <unordered_map>
#include <list>
#include <string>
#include <iostream>

namespace http
{

    enum class retDateParse
    {
        SUCCESS = 1,
        TIME_UNIT_PASSED = 2,
        FAILURE_TO_PARSE = 3,
    };

    struct httpHeader
    {
        std::string hostName;
        std::string filePath;
        std::string lastAccessTime;
        std::string lastModified;
        std::string expires;
        std::string body;
    };

    class LRUCache
    {
        private:
            // Max elements the cache can hold.
            size_t capacity;

            // Doubly linked list
            std::list<std::pair<std::string, httpHeader>> cachedList;

            // Map to provide O(1) access time.
            std::unordered_map<std::string, std::list<std::pair<std::string, httpHeader>>::iterator> cachedMap;

        public:
            // Consutuctor.
            LRUCache(size_t _capacity)
            {
                this->capacity = _capacity;
                printf("Constructor initialized!\n");
            }

            // Method to access an element.
            httpHeader fetch(const std::string &key)
            {
                auto iterator = cachedMap.find(key);
                if (iterator == cachedMap.end())
                {
                    throw std::runtime_error("Element not found!");
                }

                cachedList.splice(cachedList.begin(), cachedList, iterator->second);
                return iterator->second->second;
            }

            // Method to add a new element to the cache.
            void add(const std::string &key, const httpHeader &value)
            {
                auto iterator = cachedMap.find(key);

                if (iterator != cachedMap.end())
                {
                    // Update value and move item to the front.
                    iterator->second->second = value;
                    cachedList.splice(cachedList.begin(), cachedList, iterator->second);
                    return;
                }

                // Remove Least Recently Used Element if Cache is
                // at capacity.
                if (cachedList.size() == capacity)
                {
                    auto popElement = cachedList.back();
                    cachedMap.erase(popElement.first);
                    cachedList.pop_back();
                }

                cachedList.emplace_front(key, value);
                cachedMap[key] = cachedList.begin();
            }

            // Mainly for debugging.
            void display(void) const
            {
                printf("Printing cache\n\n\n");
                for (const auto&element: cachedList)
                {
                    std::cout << "{ " << element.first << ": " << element.second.hostName
                              << " + " << element.second.filePath << "} ";
                }
                std::cout << std::endl;
            }
    };
}

#endif /* HTTP_UTILS_H */
