INCLUDE(biicode/cmake/tools)
INCLUDE(biicode/boost/setup)

INIT_BIICODE_BLOCK()
ADD_BIICODE_TARGETS()

bii_find_boost(COMPONENTS filesystem system coroutine context thread REQUIRED)

target_include_directories(${BII_BLOCK_TARGET} INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(${BII_BLOCK_TARGET} INTERFACE ${Boost_LIBRARIES})

# enable C++ x14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

# enable all warnings by default
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# suppress some unwanted warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-unused-variable")

# enable optimizations
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
