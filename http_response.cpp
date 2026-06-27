#include "http_response.h"
#include <sstream>

HttpResponse::HttpResponse() {
    headers_["Server"]     = "http-server-cpp/1.0";
    headers_["Connection"] = "close";
}

HttpResponse& HttpResponse::set_status(int code, const std::string& text) {
    status_code_ = code;
    status_text_ = text.empty() ? reason_phrase(code) : text;
    return *this;
}

HttpResponse& HttpResponse::set_header(const std::string& key,
                                       const std::string& value) {
    headers_[key] = value;
    return *this;
}

HttpResponse& HttpResponse::set_body(const std::string& body,
                                     const std::string& content_type) {
    body_ = body;
    headers_["Content-Type"]   = content_type;
    headers_["Content-Length"] = std::to_string(body_.size());
    return *this;
}

std::string HttpResponse::build() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code_ << " " << status_text_ << "\r\n";
    for (auto& [k, v] : headers_)
        oss << k << ": " << v << "\r\n";
    oss << "\r\n" << body_;
    return oss.str();
}

// ── static factories ──────────────────────────────────────────────────────────

std::string HttpResponse::make_200(const std::string& body,
                                   const std::string& content_type) {
    return HttpResponse{}.set_status(200, "OK").set_body(body, content_type).build();
}

std::string HttpResponse::make_404(const std::string& path) {
    std::string body = "<html><body><h1>404 Not Found</h1>";
    if (!path.empty()) body += "<p>" + path + "</p>";
    body += "</body></html>";
    return HttpResponse{}.set_status(404, "Not Found").set_body(body).build();
}

std::string HttpResponse::make_500() {
    std::string body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
    return HttpResponse{}.set_status(500, "Internal Server Error").set_body(body).build();
}

std::string HttpResponse::make_400() {
    std::string body = "<html><body><h1>400 Bad Request</h1></body></html>";
    return HttpResponse{}.set_status(400, "Bad Request").set_body(body).build();
}

std::string HttpResponse::reason_phrase(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}
