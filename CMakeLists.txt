cmake_minimum_required(VERSION 3.10)
project(ProjectA)

set(CMAKE_CXX_STANDARD 17)

# Explicitly list all source files to be compiled together
add_executable(engine 
    src/main.cpp
    src/bitboard.cpp
    src/evaluate.cpp
    src/movegen.cpp
    src/search.cpp
    src/uci.cpp
    src/zobrist.cpp
)