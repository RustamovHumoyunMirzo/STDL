cmake_minimum_required(VERSION 3.15)
project(STDL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(STDL
    src/scene.cpp
    src/parser.cpp
    examples/main.cpp
)

target_include_directories(STDL PRIVATE
    src
    include
    external/PEGTL/include
)