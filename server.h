#pragma once

#include "thread_pool.h"
#include "http_parser.h"
#include "http_response.h"
#include "router.h"
#include "logger.h"

#include <string>
#include <atomic>
#include <cstddef>

/**
 * Server
 * ------
 * Owns the listening socket, a ThreadPool, and a Router.
 * Accepts connections in a tight loop, then delegates each
 * client fd to a worker thread via the pool.
 *
 * Usage:
 *   Server server(8080, 4);   // port 8080, 4 worker threads
 *   server.router().add("GET", "/", handler);
 *   server.run();             // blocks until stopped
 */
class Server {
public:
    explicit Server(int port = 8080, size_t num_threads = 4);
    ~Server();

    // Start accepting connections — blocks the calling thread.
    void run();

    // Signal the accept loop to stop (safe to call from a signal handler).
    void stop();

    // Access the router to register routes before calling run().
    Router& router() { return router_; }

    // Disallow copy and move.
    Server(const Server&)            = delete;
    Server& operator=(const Server&) = delete;

private:
    int         port_;
    int         server_fd_{-1};
    ThreadPool  pool_;
    HttpParser  parser_;
    HttpResponse response_;
    Router      router_;
    std::atomic<bool> running_{false};

    void setup_socket();
    void handle_client(int client_fd) const;
    static constexpr int BACKLOG     = 64;
    static constexpr int BUFFER_SIZE = 8192;
};
