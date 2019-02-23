cmake_minimum_required(VERSION 3.12)
project(mmars)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(.)

add_executable(mmars
        benchmark.cpp
        benchmark.hpp
        cli11.hpp
        instruction.hpp
        main.cpp
        mmars.cpp
        mmars.hpp
        parser.cpp
        parser.hpp
        thread_pool.hpp
        util.cpp
        util.hpp
        warrior.hpp)

# filesystem support
target_link_libraries(${PROJECT_NAME} stdc++fs pthread)

