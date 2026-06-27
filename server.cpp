#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>

Server::Server(int port, size_t num_threads)
    : port_(port), pool_(num_threads) {}

Server::~Server() {
    stop();
    if (server_fd_ >= 0) {
        ::close(server_fd_);
        server_fd_ = -1;
    }
}

// ── Socket setup ──────────────────────────────────────────────────────────────

void Server::setup_socket() {
    server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
        throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));

    // Allow immediate reuse of the port after restart.
    int opt = 1;
    ::setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port_));

    if (::bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));

    if (::listen(server_fd_, BACKLOG) < 0)
        throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
}

// ── Accept loop ───────────────────────────────────────────────────────────────

void Server::run() {
    setup_socket();
    running_.store(true);

    Logger::info("Server listening on port " + std::to_string(port_) +
                 " with " + std::to_string(4) + " worker threads");
    Logger::info("Press Ctrl+C to stop");

    while (running_.load()) {
        sockaddr_in client_addr{};
        socklen_t   client_len = sizeof(client_addr);

        int client_fd = ::accept(server_fd_,
                                 reinterpret_cast<sockaddr*>(&client_addr),
                                 &client_len);

        if (client_fd < 0) {
            if (!running_.load()) break;       // stop() was called
            Logger::warn("accept() failed: " + std::string(strerror(errno)));
            continue;
        }

        std::string client_ip = ::inet_ntoa(client_addr.sin_addr);
        Logger::info("New connection from " + client_ip);

        // Hand off to the thread pool — capture by value so fd is safe.
        pool_.enqueue([this, client_fd, client_ip]() {
            handle_client(client_fd);
            Logger::info("Closed connection from " + client_ip);
        });
    }

    Logger::info("Server stopped.");
}

void Server::stop() {
    running_.store(false);
    // Unblock accept() by closing the socket.
    if (server_fd_ >= 0) {
        ::shutdown(server_fd_, SHUT_RDWR);
    }
}

// ── Per-connection handler (runs on a worker thread) ─────────────────────────

void Server::handle_client(int client_fd) const {
    char   buffer[BUFFER_SIZE]{};
    ssize_t bytes = ::recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes <= 0) {
        ::close(client_fd);
        return;
    }

    std::string raw(buffer, static_cast<size_t>(bytes));
    HttpRequest req = parser_.parse(raw);

    std::string response;
    if (!req.valid) {
        response = HttpResponse::make_400();
        Logger::warn("400 Bad Request");
    } else {
        Logger::info(req.method + " " + req.path);
        response = router_.dispatch(req);
    }

    ::send(client_fd, response.c_str(), response.size(), 0);
    ::close(client_fd);
}
