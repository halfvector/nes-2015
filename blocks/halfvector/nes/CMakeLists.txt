cmake_minimum_required(VERSION 3.10)
include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.20.31.tar.gz"
        SHA1 "65c8f2292566aedb2fe10e08e09c74dda8e5d07a"
)


project(nes)

# define dependencies

# Boost - utils
hunter_add_package(Boost COMPONENTS system filesystem)
find_package(Boost CONFIG REQUIRED system filesystem)

# SDL2 - window/gfx
hunter_add_package(SDL2)
find_package(SDL2 CONFIG REQUIRED)

# spdlog - logger
hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# enable C++ x14
#add_definitions(-std=c++1y)
#TARGET_COMPILE_OPTIONS(${BII_main_TARGET} INTERFACE "-std=c++1y")

# enable all warnings by default
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# suppress some unwanted warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-unused-variable")

# debug information
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ftrap-function=abort -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -fsanitize=address")

#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic -Wl,-stack_size -Wl,0x10000")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(.)

add_executable(nes
#        performance/Lambda.cpp
#        performance/Loops.cpp
#        test/InstructionTest.cpp
#        test/MemoryTest.cpp
        Backtrace.cpp
        Backtrace.h
        Cartridge.h
        CartridgeLoader.cpp
        CartridgeLoader.h
        CPU.cpp
        CPU.h
        DI.h
        Exceptions.h
        GUI.cpp
        GUI.h
        Instructions.cpp
        Instructions.h
        Logging.cpp
        Logging.h
        main.cpp
        Memory.cpp
        Memory.h
        MemoryIO.cpp
        MemoryIO.h
        MemoryLookup.cpp
        MemoryLookup.h
        MemoryOperation.h
        MemoryStack.cpp
        MemoryStack.h
        Platform.h
        PPU.cpp
        PPU.h
        RegisterOperation.cpp
        RegisterOperation.h
        Registers.h)

target_link_libraries(nes SDL2::SDL2main SDL2::SDL2 Boost::system Boost::filesystem spdlog::spdlog)
