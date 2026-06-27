#include "http_parser.h"
#include <gtest/gtest.h>

class HttpParserTest : public ::testing::Test {
protected:
    HttpParser parser;
};

// ── Request line ──────────────────────────────────────────────────────────────

TEST_F(HttpParserTest, ParsesGETMethod) {
    auto req = parser.parse("GET / HTTP/1.1\r\n\r\n");
    EXPECT_TRUE(req.valid);
    EXPECT_EQ(req.method, "GET");
}

TEST_F(HttpParserTest, ParsesPOSTMethod) {
    auto req = parser.parse("POST /submit HTTP/1.1\r\n\r\nbody=data");
    EXPECT_TRUE(req.valid);
    EXPECT_EQ(req.method, "POST");
}

TEST_F(HttpParserTest, ParsesPath) {
    auto req = parser.parse("GET /hello/world HTTP/1.1\r\n\r\n");
    EXPECT_EQ(req.path, "/hello/world");
}

TEST_F(HttpParserTest, ParsesVersion) {
    auto req = parser.parse("GET / HTTP/1.1\r\n\r\n");
    EXPECT_EQ(req.version, "HTTP/1.1");
}

TEST_F(HttpParserTest, EmptyRawMarkedInvalid) {
    auto req = parser.parse("");
    EXPECT_FALSE(req.valid);
}

// ── Headers ───────────────────────────────────────────────────────────────────

TEST_F(HttpParserTest, ParsesSingleHeader) {
    std::string raw = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    auto req = parser.parse(raw);
    EXPECT_EQ(req.headers.at("host"), "localhost");
}

TEST_F(HttpParserTest, ParsesMultipleHeaders) {
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Accept: text/html\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";
    auto req = parser.parse(raw);
    EXPECT_EQ(req.headers.at("host"),       "example.com");
    EXPECT_EQ(req.headers.at("accept"),     "text/html");
    EXPECT_EQ(req.headers.at("connection"), "keep-alive");
}

TEST_F(HttpParserTest, HeaderKeysAreLowerCased) {
    std::string raw = "GET / HTTP/1.1\r\nContent-Type: application/json\r\n\r\n";
    auto req = parser.parse(raw);
    EXPECT_EQ(req.headers.at("content-type"), "application/json");
}

TEST_F(HttpParserTest, HeaderValuesAreTrimmed) {
    std::string raw = "GET / HTTP/1.1\r\nHost:   localhost   \r\n\r\n";
    auto req = parser.parse(raw);
    EXPECT_EQ(req.headers.at("host"), "localhost");
}

// ── Body ──────────────────────────────────────────────────────────────────────

TEST_F(HttpParserTest, ParsesBody) {
    std::string raw = "POST /data HTTP/1.1\r\n\r\nhello=world";
    auto req = parser.parse(raw);
    EXPECT_EQ(req.body, "hello=world");
}

TEST_F(HttpParserTest, EmptyBodyForGET) {
    auto req = parser.parse("GET / HTTP/1.1\r\n\r\n");
    EXPECT_TRUE(req.body.empty());
}
