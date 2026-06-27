#pragma once

#include <string>
#include <mutex>
#include <iostream>

/**
 * Logger
 * ------
 * Thread-safe, timestamped logger. Writes to stdout.
 * Each log call is atomic — no interleaved output from concurrent threads.
 *
 * Usage:
 *   Logger::info("Server started on port 8080");
 *   Logger::error("accept() failed");
 */
class Logger {
public:
    static void info (const std::string& msg);
    static void warn (const std::string& msg);
    static void error(const std::string& msg);

private:
    static std::mutex        mutex_;
    static std::string       timestamp();
    static void              log(const std::string& level,
                                 const std::string& msg);
};
