cmake_minimum_required(VERSION 3.13)  # CMake version check
project(xow)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 20)            

add_subdirectory(utils)
add_subdirectory(controller)

set(SOURCE_FILES 
    xow.cpp
)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(xow ${SOURCE_FILES})

find_package(Threads REQUIRED)

target_link_libraries(xow usb-1.0 Threads::Threads util controller)

add_definitions(-DVERSION="HI" -DDEBUG)
