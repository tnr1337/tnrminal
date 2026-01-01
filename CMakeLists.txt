cmake_minimum_required(VERSION 3.10)
project(tnrm1n4l C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Executable
add_executable(tnrm1n4l ${SOURCES})

# Windows specifics
if(WIN32)
    target_link_libraries(tnrm1n4l kernel32 user32)
endif()
