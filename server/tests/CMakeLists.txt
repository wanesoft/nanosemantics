cmake_minimum_required(VERSION 3.0)
project(nsserver_tests)

set(CMAKE_CXX_STANDARD 14)

if(NOT TARGET tests)
    add_custom_target(tests)
endif(NOT TARGET tests)