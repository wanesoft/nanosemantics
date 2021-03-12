cmake_minimum_required(VERSION 3.0)
project(NSServer)

set(CMAKE_C_STANDARD   03)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -Wextra -Werror")

set(SRC src/main.cpp src/NSServer.cpp src/NSThreadPool.cpp)
set(THIRD_PARTY_DIR third-party)

include_directories(include)
add_subdirectory(${THIRD_PARTY_DIR}/googletest)

add_executable(nsserver ${SRC})

add_subdirectory(tests)