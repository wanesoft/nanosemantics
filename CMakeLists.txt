cmake_minimum_required(VERSION 3.0)
project(NSTask)

set(CMAKE_C_STANDARD   03)
set(CMAKE_CXX_STANDARD 14)

add_compile_options("-fsanitize=address")
link_libraries("-fsanitize=address")

enable_testing()

add_subdirectory(third-party/googletest)
add_subdirectory(server)
add_subdirectory(client)

add_executable(run_server server.cpp)
target_link_libraries(run_server nsserver)
target_include_directories(run_server PRIVATE server/include)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/server.conf ${CMAKE_CURRENT_BINARY_DIR}/server.conf COPYONLY)

add_executable(run_client client.cpp)
target_link_libraries(run_client nsclient)
target_include_directories(run_client PRIVATE client/include)
