#include "logger.h"
#include <ctime>
#include <iomanip>
#include <sstream>

std::mutex Logger::mutex_;

std::string Logger::timestamp() {
    auto now = std::time(nullptr);
    std::tm  tm_info{};
#ifdef _WIN32
    localtime_s(&tm_info, &now);
#else
    localtime_r(&now, &tm_info);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::log(const std::string& level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "[" << timestamp() << "] [" << level << "] " << msg << "\n";
    std::cout.flush();
}

void Logger::info (const std::string& msg) { log("INFO ", msg); }
void Logger::warn (const std::string& msg) { log("WARN ", msg); }
void Logger::error(const std::string& msg) { log("ERROR", msg); }
