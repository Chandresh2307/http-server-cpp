#pragma once

#include "http_parser.h"
#include <functional>
#include <string>
#include <vector>

// A handler receives the parsed request and returns a raw HTTP response string.
using Handler = std::function<std::string(const HttpRequest&)>;

struct Route {
    std::string method;   // "GET", "POST", …
    std::string path;     // exact path match e.g. "/about"
    Handler     handler;
};

/**
 * Router
 * ------
 * Matches an HttpRequest to the first registered route.
 * Falls back to a 404 response if no route matches.
 *
 * Usage:
 *   Router router;
 *   router.add("GET", "/", [](const HttpRequest&) {
 *       return HttpResponse::make_200("<h1>Home</h1>");
 *   });
 *   std::string response = router.dispatch(request);
 */
class Router {
public:
    void        add(const std::string& method,
                    const std::string& path,
                    Handler            handler);

    std::string dispatch(const HttpRequest& req) const;

private:
    std::vector<Route> routes_;
};
