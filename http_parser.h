#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>

/**
 * HttpRequest
 * -----------
 * Plain-old-data struct representing a parsed HTTP/1.1 request.
 */
struct HttpRequest {
    std::string method;                                  // GET, POST, DELETE …
    std::string path;                                    // /index.html
    std::string version;                                 // HTTP/1.1
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    bool        valid{false};
};

/**
 * HttpParser
 * ----------
 * Parses a raw HTTP/1.1 request string into an HttpRequest.
 *
 * Usage:
 *   HttpParser parser;
 *   HttpRequest req = parser.parse(raw_text);
 *   if (req.valid) { ... }
 */
class HttpParser {
public:
    HttpRequest parse(const std::string& raw) const;

private:
    // Splits raw at the first occurrence of delim; returns {left, right}.
    static std::pair<std::string, std::string>
    split_once(const std::string& s, const std::string& delim);

    static std::string trim(const std::string& s);
    static std::string to_lower(std::string s);
};
