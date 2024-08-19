cmake_minimum_required(VERSION 3.27)
project(NESem)

set(CMAKE_CXX_STANDARD 17)

add_executable(NESem main.cpp
        NES_CPU.cpp
        NES_CPU.h
        NES_MEMORY.cpp
        NES_MEMORY.h
        NES_CARTRIDGE.cpp
        NES_CARTRIDGE.h
        NES_PPU.cpp
        NES_PPU.h
        NES_APU.cpp
        NES_APU.h)
target_link_libraries(NESem SDL2)
