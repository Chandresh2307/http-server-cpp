#include "server.h"
#include "http_response.h"
#include "logger.h"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sstream>

// ── Global server pointer for signal handler ──────────────────────────────────
static Server* g_server = nullptr;

void signal_handler(int signal) {
    if (g_server) {
        Logger::warn("Signal " + std::to_string(signal) + " received — shutting down…");
        g_server->stop();
    }
}

// ── Routes ────────────────────────────────────────────────────────────────────

static std::string home_handler(const HttpRequest& /*req*/) {
    const std::string body = R"(
<!DOCTYPE html>
<html>
<head><title>C++ HTTP Server</title></head>
<body style="font-family:monospace;max-width:600px;margin:40px auto">
  <h1>&#128640; Multi-Threaded HTTP Server</h1>
  <p>Built from scratch in C++17 — no frameworks.</p>
  <ul>
    <li><a href="/about">About</a></li>
    <li><a href="/health">Health check (JSON)</a></li>
    <li><a href="/echo">Echo endpoint</a></li>
  </ul>
</body>
</html>)";
    return HttpResponse::make_200(body);
}

static std::string about_handler(const HttpRequest& /*req*/) {
    const std::string body = R"(
<!DOCTYPE html>
<html>
<head><title>About</title></head>
<body style="font-family:monospace;max-width:600px;margin:40px auto">
  <h1>About This Server</h1>
  <p>Stack: C++17 · POSIX sockets · std::thread · mutex · condition_variable</p>
  <p>Architecture: fixed thread pool, mutex-protected task queue, HTTP/1.1 parser.</p>
  <a href="/">&#8592; Home</a>
</body>
</html>)";
    return HttpResponse::make_200(body);
}

static std::string health_handler(const HttpRequest& /*req*/) {
    const std::string json = R"({"status":"ok","server":"http-server-cpp","version":"1.0"})";
    return HttpResponse::make_200(json, "application/json");
}

static std::string echo_handler(const HttpRequest& req) {
    std::ostringstream oss;
    oss << "<!DOCTYPE html><html><body style='font-family:monospace'>"
        << "<h2>Echo</h2><pre>"
        << "Method : " << req.method  << "\n"
        << "Path   : " << req.path    << "\n"
        << "Version: " << req.version << "\n\nHeaders:\n";
    for (auto& [k, v] : req.headers)
        oss << "  " << k << ": " << v << "\n";
    if (!req.body.empty())
        oss << "\nBody:\n" << req.body;
    oss << "</pre></body></html>";
    return HttpResponse::make_200(oss.str());
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    int    port        = 8080;
    size_t num_threads = 4;

    if (argc >= 2) port        = std::atoi(argv[1]);
    if (argc >= 3) num_threads = static_cast<size_t>(std::atoi(argv[2]));

    try {
        Server server(port, num_threads);
        g_server = &server;

        // Graceful shutdown on Ctrl+C / kill.
        std::signal(SIGINT,  signal_handler);
        std::signal(SIGTERM, signal_handler);

        // Register routes.
        server.router().add("GET", "/",       home_handler);
        server.router().add("GET", "/about",  about_handler);
        server.router().add("GET", "/health", health_handler);
        server.router().add("GET", "/echo",   echo_handler);

        server.run(); // blocks here
    } catch (const std::exception& e) {
        Logger::error(std::string("Fatal: ") + e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
