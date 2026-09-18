cmake_minimum_required(VERSION 3.16)
project(http_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(SOURCES
    http_server/main.cpp
    http_server/socket.cpp
    http_server/thread_pool.cpp
)

add_executable(http_server ${SOURCES})

target_include_directories(http_server PRIVATE http_server)

if(WIN32)
    target_link_libraries(http_server PRIVATE ws2_32)
endif()

target_link_libraries(http_server PRIVATE Threads::Threads)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(http_server PRIVATE -Wall -Wextra)
endif()

set_target_properties(http_server PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/http_server"
)