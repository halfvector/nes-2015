INCLUDE(biicode/cmake/tools)
INCLUDE(biicode/boost/setup)

INIT_BIICODE_BLOCK()
ADD_BIICODE_TARGETS()

bii_find_boost(COMPONENTS filesystem system coroutine context thread REQUIRED)

# enable C++ x11
ACTIVATE_CPP11(INTERFACE ${BII_BLOCK_TARGET})

target_include_directories(${BII_BLOCK_TARGET} INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(${BII_BLOCK_TARGET} INTERFACE ${Boost_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")