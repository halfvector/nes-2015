ADD_BIICODE_TARGETS()

#INCLUDE(biicode/cmake/tools)
INCLUDE(biicode/boost/setup)
set(Boost_USE_STATIC_LIBS OFF)
bii_find_boost(COMPONENTS filesystem system coroutine context thread REQUIRED)

target_include_directories(${BII_BLOCK_TARGET} INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(${BII_BLOCK_TARGET} INTERFACE ${Boost_LIBRARIES})

# enable C++ x14
#add_definitions(-std=c++1y)
TARGET_COMPILE_OPTIONS(${BII_main_TARGET} INTERFACE "-std=c++1y")

# enable all warnings by default
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# suppress some unwanted warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-unused-variable")

# debug information
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ftrap-function=abort -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic -Wl,-stack_size -Wl,0x10000")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls")

# enable optimizations
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
