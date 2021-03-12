cmake_minimum_required(VERSION 3.0)

if(WIN32)
add_definitions(-DGTEST_HAS_SEH=0 -DGTEST_IS_THREADSAFE=1)
endif()
add_definitions(-DGTEST_HAS_EXCEPTIONS=0)
add_library(gtest STATIC googletest-1.8.0/src/gtest-all.cc)
target_include_directories(gtest PRIVATE googletest-1.8.0)
target_include_directories(gtest PUBLIC googletest-1.8.0/include)

add_library(gtest_main STATIC googletest-1.8.0/src/gtest_main.cc)
target_link_libraries(gtest_main gtest)
