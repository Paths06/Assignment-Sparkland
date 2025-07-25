cmake_minimum_required(VERSION 3.15)
project(CoinbaseHFTTicker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Set target triplet explicitly
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows")
endif()

# Find basic packages
find_package(Threads REQUIRED)

# Find nlohmann_json
find_package(nlohmann_json CONFIG QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "Found nlohmann_json via CONFIG")
    set(JSON_TARGET "nlohmann_json::nlohmann_json")
else()
    set(VCPKG_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
    include_directories(${VCPKG_INCLUDE_DIR})
    message(STATUS "Using nlohmann/json.hpp directly from: ${VCPKG_INCLUDE_DIR}")
endif()

# Find ixwebsocket
find_package(ixwebsocket CONFIG QUIET)
if(ixwebsocket_FOUND)
    message(STATUS "Found ixwebsocket via CONFIG")
    set(WEBSOCKET_TARGET "ixwebsocket::ixwebsocket")
else()
    # Manual search for ixwebsocket
    find_path(IXWEBSOCKET_INCLUDE_DIR
        NAMES ixwebsocket/IXWebSocket.h
        PATHS "C:/vcpkg/installed/x64-windows/include"
        REQUIRED
    )
    
    find_library(IXWEBSOCKET_LIBRARY
        NAMES ixwebsocket
        PATHS "C:/vcpkg/installed/x64-windows/lib"
        REQUIRED
    )
    
    message(STATUS "Found ixwebsocket manually")
    message(STATUS "  Include: ${IXWEBSOCKET_INCLUDE_DIR}")
    message(STATUS "  Library: ${IXWEBSOCKET_LIBRARY}")
endif()

# Compiler flags for Windows
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # Suppress unsafe function warnings
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/ema_calculator.cpp
    src/ticker_data.cpp
    src/logger.cpp
    src/json_parser.cpp
    src/csv_writer.cpp
    src/websocket_client.cpp
    src/hft_processor.cpp
    src/test_runner.cpp
)

# Create executable
add_executable(coinbase_ticker ${SOURCES})

# Link libraries - start with basics
target_link_libraries(coinbase_ticker PRIVATE
    Threads::Threads
)

# Add JSON library (header-only, so just need includes which we already added)
if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(coinbase_ticker PRIVATE nlohmann_json::nlohmann_json)
    message(STATUS "Linked nlohmann_json target")
else()
    message(STATUS "Using nlohmann_json as header-only (already included)")
endif()

# Add ixwebsocket library
if(TARGET ixwebsocket::ixwebsocket)
    target_link_libraries(coinbase_ticker PRIVATE ixwebsocket::ixwebsocket)
    message(STATUS "Linked ixwebsocket target")
elseif(IXWEBSOCKET_LIBRARY)
    target_include_directories(coinbase_ticker PRIVATE ${IXWEBSOCKET_INCLUDE_DIR})
    target_link_libraries(coinbase_ticker PRIVATE ${IXWEBSOCKET_LIBRARY})
    message(STATUS "Linked ixwebsocket library manually")
endif()

# Add Windows networking and crypto libraries
if(WIN32)
    target_link_libraries(coinbase_ticker PRIVATE 
        ws2_32 
        wsock32
        bcrypt 
        crypt32
        secur32
    )
    message(STATUS "Added Windows networking and crypto libraries")
endif()

# Find and link OpenSSL if available (for SSL WebSocket support)
find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    target_link_libraries(coinbase_ticker PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    message(STATUS "Linked OpenSSL for SSL support")
else()
    message(STATUS "OpenSSL not found - WebSocket SSL support may be limited")
endif()

message(STATUS "Configuration completed successfully!")
message(STATUS "Ready to build with: cmake --build build --config Release")