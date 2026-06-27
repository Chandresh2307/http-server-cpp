#include "router.h"
#include "http_response.h"
#include <algorithm>

void Router::add(const std::string& method,
                 const std::string& path,
                 Handler            handler) {
    routes_.push_back({method, path, std::move(handler)});
}

std::string Router::dispatch(const HttpRequest& req) const {
    for (const auto& route : routes_) {
        if (route.method == req.method && route.path == req.path)
            return route.handler(req);
    }
    return HttpResponse::make_404(req.path);
}
