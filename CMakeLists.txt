cmake_minimum_required(VERSION 3.17)
project(1_Data_Base)

set(CMAKE_CXX_STANDARD 14)

add_executable(1_Data_Base main.cpp database.cpp date.h date.cpp condition_parser.h condition_parser.cpp token.h token.cpp node.h node.cpp)