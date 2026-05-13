cmake_minimum_required(VERSION 3.15...4.2)
project(CHIP_8 C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 REQUIRED)

add_executable(CHIP_8
        main.c
        fonts.h
        chip8.h
        chip8.c
)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(CHIP_8 PRIVATE
            -Wall
            -Wextra
            -Wimplicit-fallthrough
            -Wshadow
    )
endif()

target_link_libraries(CHIP_8 PRIVATE SDL2::SDL2)