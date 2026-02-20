cmake_minimum_required(VERSION 3.15)
project(STDL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(STDL STATIC
    src/scene.cpp
    src/parser.cpp
)

target_include_directories(STDL
    PUBLIC
        include
        external/PEGTL/include
        src
)

add_executable(STDL_example
    examples/main.cpp
)

target_link_libraries(STDL_example PRIVATE STDL)