cmake_minimum_required(VERSION 4.1)
project(minilang)

set(CMAKE_CXX_STANDARD 20)

add_executable(minilang
        src/main.cpp
        src/LexAnalyzer.cpp
        src/SyntaxAnalyzer.cpp)
