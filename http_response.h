#pragma once

#include <string>
#include <unordered_map>

/**
 * HttpResponse
 * ------------
 * Builds a well-formed HTTP/1.1 response string ready to send over a socket.
 *
 * Usage:
 *   HttpResponse res;
 *   res.set_status(200, "OK")
 *      .set_header("Content-Type", "text/html")
 *      .set_body("<h1>Hello</h1>");
 *   std::string raw = res.build();
 */
class HttpResponse {
public:
    HttpResponse();

    HttpResponse& set_status(int code, const std::string& text);
    HttpResponse& set_header(const std::string& key, const std::string& value);
    HttpResponse& set_body(const std::string& body,
                           const std::string& content_type = "text/html");

    // Serialise to a raw HTTP/1.1 response string.
    std::string build() const;

    // Convenience static factories.
    static std::string make_200(const std::string& body,
                                const std::string& content_type = "text/html");
    static std::string make_404(const std::string& path = "");
    static std::string make_500();
    static std::string make_400();

private:
    int         status_code_{200};
    std::string status_text_{"OK"};
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;

    static std::string reason_phrase(int code);
};
