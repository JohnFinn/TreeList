cmake_minimum_required(VERSION 3.10)
project(data_structures)
set(CMAKE_C_STANDARD 17)

add_executable(data_structures main.cpp TreeList.h Node.h TreeListIterator.cpp TreeListIterator.h)
target_link_libraries(data_structures gtest)
