cmake_minimum_required(VERSION 3.20)
project(os_ex4)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(os_ex4
        MemoryConstants2.h
        MemoryConstants.h
        PhysicalMemory.cpp
        PhysicalMemory.h
#        SimpleTest.cpp
        test_example.cpp
        VirtualMemory.h)
