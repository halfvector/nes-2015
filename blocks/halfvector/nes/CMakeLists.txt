INCLUDE(biicode/cmake/tools)
INCLUDE(biicode/boost/setup)

INIT_BIICODE_BLOCK()
ADD_BIICODE_TARGETS()

bii_find_boost(COMPONENTS filesystem system coroutine context thread REQUIRED)

# enable C++ x14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

target_include_directories(${BII_BLOCK_TARGET} INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(${BII_BLOCK_TARGET} INTERFACE ${Boost_LIBRARIES})

# configure warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wunused-function -Wunused-variable")