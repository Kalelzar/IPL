cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 14)

# Source
project(cl2html VERSION 1)

file(GLOB ALL_SRC CONFIGURE_DEPENDS
        "*.hpp"
        "*.cpp"
        )

add_executable(cl2html ${ALL_SRC})
