cmake_minimum_required(VERSION 3.0)
project(nsserver_tests)

set(CMAKE_CXX_STANDARD 14)

if(NOT TARGET tests)
    add_custom_target(tests)
endif(NOT TARGET tests)

add_executable(test_NSThreadPool EXCLUDE_FROM_ALL test_NSThreadPool.cpp)
target_link_libraries(test_NSThreadPool nsserver gtest_main)
add_test(NSThreadPool test_NSThreadPool)
add_dependencies(tests test_NSThreadPool)