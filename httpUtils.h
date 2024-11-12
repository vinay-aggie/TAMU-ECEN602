#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <string>
#include <map>
#include <iostream>

// Parses an HTTP request or response string and extracts headers and body.
// Returns true if parsing is successful, false otherwise.
// Parsed headers are stored in the 'headers' map and the body in 'body'.
bool parseHttp(const std::string& httpMessage, std::map<std::string, std::string>& headers, std::string& body) {
    size_t headerEndPos = httpMessage.find("\r\n\r\n");
    if (headerEndPos == std::string::npos) {
        std::cerr << "Error: Invalid HTTP message format, missing header terminator." << std::endl;
        return false;
    }
    
    std::string headerPart = httpMessage.substr(0, headerEndPos);
    body = httpMessage.substr(headerEndPos + 4); // Skip the "\r\n\r\n"

    size_t linePos = 0, lineEndPos;
    while ((lineEndPos = headerPart.find("\r\n", linePos)) != std::string::npos) {
        std::string line = headerPart.substr(linePos, lineEndPos - linePos);
        size_t delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 2);
            headers[key] = value;
        } else {
            std::cerr << "Warning: Malformed header line, missing delimiter: " << line << std::endl;
        }
        linePos = lineEndPos + 2; // Move to the next line
    }

    return true;
}

#endif // HTTPUTILS_H
