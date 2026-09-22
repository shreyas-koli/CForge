cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(CForge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# -----------------------------------------------------------------------------
# Compiler Executable Target
#   Sources:
#     - src/driver/main.cpp     : Entry point; argc/argv parsing
#     - src/driver/pipeline.cpp : Compilation pipeline (stub stages)
#     - src/lexer/token.cpp     : Token type name helper (Day 4)
#     - src/lexer/lexer.cpp     : Lexer: identifier/keyword scanning (Day 5)
# -----------------------------------------------------------------------------
add_executable(cforge
    src/driver/main.cpp
    src/driver/pipeline.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
)

# -----------------------------------------------------------------------------
# Testing Configuration (Catch2 v2.13.10 + CTest)
#   Sources:
#     - tests/test_main.cpp    : Catch2 runner entry point + sanity check (Day 2)
#     - tests/test_driver.cpp  : Driver + pipeline unit tests (Day 3)
#     - tests/test_token.cpp   : Token / TokenType unit tests (Day 4)
#     - tests/test_lexer.cpp   : Lexer identifier/keyword tests (Day 5)
#     - src/driver/pipeline.cpp: Shared with cforge target; main.cpp excluded
#     - src/lexer/token.cpp    : Shared with cforge target
#     - src/lexer/lexer.cpp    : Shared with cforge target
# -----------------------------------------------------------------------------
enable_testing()
include(CTest)

include(FetchContent)
FetchContent_Declare(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.10.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(cforge_tests
    tests/test_main.cpp
    tests/test_driver.cpp
    tests/test_token.cpp
    tests/test_lexer.cpp
    src/driver/pipeline.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
)
target_link_libraries(cforge_tests PRIVATE Catch2::Catch2)

# Enable CTest integration for Catch2
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(Catch)
catch_discover_tests(cforge_tests)
