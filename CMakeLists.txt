cmake_minimum_required(VERSION 3.15)
project(HttpServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

# ── Main server executable ──────────────────────────────────────────────────
add_executable(http_server
    src/main.cpp
    src/server.cpp
    src/thread_pool.cpp
    src/http_parser.cpp
    src/http_response.cpp
    src/router.cpp
    src/logger.cpp
)

target_include_directories(http_server PRIVATE include)

# Link pthread on Linux/Mac
find_package(Threads REQUIRED)
target_link_libraries(http_server PRIVATE Threads::Threads)

# ── Google Test ─────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(run_tests
    tests/test_http_parser.cpp
    tests/test_thread_pool.cpp
    tests/test_router.cpp
    src/http_parser.cpp
    src/http_response.cpp
    src/thread_pool.cpp
    src/router.cpp
    src/logger.cpp
)

target_include_directories(run_tests PRIVATE include)
target_link_libraries(run_tests PRIVATE gtest_main Threads::Threads)

include(GoogleTest)
gtest_discover_tests(run_tests)
