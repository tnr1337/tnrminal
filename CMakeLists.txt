cmake_minimum_required(VERSION 3.10)
project(tnrm1n4l C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler warnings
if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Executable
add_executable(tnrm1n4l ${SOURCES})

# Windows specifics
if(WIN32)
    target_link_libraries(tnrm1n4l kernel32 user32)
    # UTF-8 support for console
    if(MSVC)
        target_compile_options(tnrm1n4l PRIVATE /utf-8)
    endif()
endif()
