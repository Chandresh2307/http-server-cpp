#include "http_parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// ── helpers ──────────────────────────────────────────────────────────────────

std::pair<std::string, std::string>
HttpParser::split_once(const std::string& s, const std::string& delim) {
    auto pos = s.find(delim);
    if (pos == std::string::npos)
        return {s, ""};
    return {s.substr(0, pos), s.substr(pos + delim.size())};
}

std::string HttpParser::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

std::string HttpParser::to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

// ── parse ─────────────────────────────────────────────────────────────────────

HttpRequest HttpParser::parse(const std::string& raw) const {
    HttpRequest req;

    // Split head (everything before \r\n\r\n) and body.
    auto [head, body] = split_once(raw, "\r\n\r\n");
    req.body = body;

    std::istringstream stream(head);
    std::string line;

    // ── Request line: GET /path HTTP/1.1 ─────────────────────────────────────
    if (!std::getline(stream, line)) return req;
    if (!line.empty() && line.back() == '\r') line.pop_back();

    std::istringstream request_line(line);
    if (!(request_line >> req.method >> req.path >> req.version))
        return req;

    // ── Headers ───────────────────────────────────────────────────────────────
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;

        auto [key, value] = split_once(line, ":");
        if (!value.empty())
            req.headers[to_lower(trim(key))] = trim(value);
    }

    req.valid = true;
    return req;
}
