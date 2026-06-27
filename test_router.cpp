#include "router.h"
#include "http_response.h"
#include <gtest/gtest.h>

class RouterTest : public ::testing::Test {
protected:
    Router router;

    void SetUp() override {
        router.add("GET", "/", [](const HttpRequest&) {
            return HttpResponse::make_200("<h1>Home</h1>");
        });
        router.add("GET", "/about", [](const HttpRequest&) {
            return HttpResponse::make_200("<h1>About</h1>");
        });
        router.add("POST", "/data", [](const HttpRequest& req) {
            return HttpResponse::make_200("received: " + req.body);
        });
    }

    // Helper: build a minimal valid HttpRequest.
    static HttpRequest make_req(const std::string& method,
                                const std::string& path,
                                const std::string& body = "") {
        HttpRequest req;
        req.method  = method;
        req.path    = path;
        req.version = "HTTP/1.1";
        req.body    = body;
        req.valid   = true;
        return req;
    }
};

// ── Dispatch ──────────────────────────────────────────────────────────────────

TEST_F(RouterTest, DispatchesRootRoute) {
    auto resp = router.dispatch(make_req("GET", "/"));
    EXPECT_NE(resp.find("200 OK"), std::string::npos);
    EXPECT_NE(resp.find("Home"),   std::string::npos);
}

TEST_F(RouterTest, DispatchesAboutRoute) {
    auto resp = router.dispatch(make_req("GET", "/about"));
    EXPECT_NE(resp.find("200 OK"), std::string::npos);
    EXPECT_NE(resp.find("About"),  std::string::npos);
}

TEST_F(RouterTest, Returns404ForUnknownPath) {
    auto resp = router.dispatch(make_req("GET", "/missing"));
    EXPECT_NE(resp.find("404"), std::string::npos);
}

TEST_F(RouterTest, MethodMismatchReturns404) {
    // POST to a GET-only route should 404.
    auto resp = router.dispatch(make_req("POST", "/"));
    EXPECT_NE(resp.find("404"), std::string::npos);
}

TEST_F(RouterTest, POSTRouteReceivesBody) {
    auto resp = router.dispatch(make_req("POST", "/data", "name=chandresh"));
    EXPECT_NE(resp.find("200 OK"),         std::string::npos);
    EXPECT_NE(resp.find("name=chandresh"), std::string::npos);
}

TEST_F(RouterTest, FirstMatchWins) {
    Router r;
    r.add("GET", "/", [](const HttpRequest&){ return HttpResponse::make_200("first"); });
    r.add("GET", "/", [](const HttpRequest&){ return HttpResponse::make_200("second"); });
    auto resp = r.dispatch(make_req("GET", "/"));
    EXPECT_NE(resp.find("first"),  std::string::npos);
    EXPECT_EQ(resp.find("second"), std::string::npos);
}
