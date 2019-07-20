cmake_minimum_required(VERSION 3.13.2)
project(nes)

# include dependencies imported by conan
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	set(CONAN_SETTINGS_COMPILER "clang")
	set(CONAN_SETTINGS_COMPILER_VERSION "8")
	set(CONAN_SETTINGS_COMPILER_TOOLSET "clang")
    conan_basic_setup()
else()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(CMAKE_CXX_STANDARD 17)

	# enable optimal config for valgrind
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-omit-frame-pointer -g -glldb")
	# end of valgrind config

	# enable C++ x14
	#add_definitions(-std=c++1y)
	#TARGET_COMPILE_OPTIONS(${BII_main_TARGET} INTERFACE "-std=c++1y")

	# enable all warnings by default
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

	# suppress unwanted warnings
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-unused-variable")

	# generate complete debug type info, tune for GDB (-ggdb) or LLDB (-glldb)
	# enable address sanitization to easily catch buffer overflows and heap corruptions
	# -O1 optimizations are compatible with address-sanitizer
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -fno-omit-frame-pointer -g -glldb -fsanitize=address")

	# enable speed optimizations
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

	# generate optimization report
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Rpass=inline -fsave-optimization-record")

	# improve template/type error messages
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-show-template-tree -fno-elide-type")

	# disable optimizations for profiling mode
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")

	#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic -Wl,-stack_size -Wl,0x10000")

	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls")

	#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wno-pragma-pack -Wno-missing-braces -D_CRT_SECURE_NO_WARNINGS /GS- /O2 /Ob2")
endif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

find_package(OpenGL REQUIRED)
find_package(GLEW 2.1 REQUIRED)
find_package(SDL2 2.0.9 REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/*.h")
add_executable(nes ${SOURCES})

# copy non-comiling dependencies
add_custom_command(TARGET nes POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/src/shaders
		${CMAKE_CURRENT_BINARY_DIR}/bin/shaders)

add_custom_command(TARGET nes POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/fonts
		${CMAKE_CURRENT_BINARY_DIR}/bin/fonts)

add_custom_command(TARGET nes POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/roms
		${CMAKE_CURRENT_BINARY_DIR}/bin/roms)

# conan macro will link with all dependencies
conan_target_link_libraries(nes)
target_link_libraries(nes ${CONAN_LIBS})