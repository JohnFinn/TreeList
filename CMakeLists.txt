cmake_minimum_required(VERSION 3.10)
project(tree_list_test)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(GTest REQUIRED)

add_executable(tree_list_test test_tree_list.cpp TreeList.h Node.h)
target_link_libraries(tree_list_test GTest::gtest GTest::gtest_main)
