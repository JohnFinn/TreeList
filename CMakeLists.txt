cmake_minimum_required(VERSION 3.10)
project(tree_list_test)
set(CMAKE_C_STANDARD 17)

add_executable(tree_list_test test_tree_list.cpp TreeList.h Node.h)
target_link_libraries(tree_list_test gtest)
