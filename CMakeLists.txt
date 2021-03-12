cmake_minimum_required(VERSION 3.0)
project(NSTask)

set(CMAKE_C_STANDARD   03)
set(CMAKE_CXX_STANDARD 14)

enable_testing()

add_subdirectory(third-party/googletest)
add_subdirectory(server)

add_executable(run_server server.cpp)
target_link_libraries(run_server nsserver)
target_include_directories(run_server PRIVATE server/include)
