cmake_minimum_required(VERSION 3.0)
project(NSClient)

set(CMAKE_C_STANDARD   03)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -Wextra -Werror")

set(SRC src/NSClient.cpp)

include_directories(include)
add_library(nsclient ${SRC})