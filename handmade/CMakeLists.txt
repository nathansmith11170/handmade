cmake_minimum_required(VERSION 3.28.0 FATAL_ERROR)
project("handmade"
  LANGUAGES CXX
)

#
# Set language version used
#
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_policy(VERSION 3.28)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# Import the modules from libc++
#
include(FetchContent)
FetchContent_Declare(
  std
  URL "file://${LIBCXX_BUILD}/modules/c++/v1/"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SYSTEM
)
FetchContent_MakeAvailable(std)

#
# Adjust project compiler flags
#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fprebuilt-module-path=${std_BINARY_DIR}/CMakeFiles/std.dir/>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fprebuilt-module-path=${std_BINARY_DIR}/CMakeFiles/std.compat.dir/>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-nostdinc++>)
# The include path needs to be set to be able to use macros from headers.
# For example from, the headers <cassert> and <version>.
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-isystem>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${LIBCXX_BUILD}/include/c++/v1>)

#
# Adjust project linker flags
#
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-nostdlib++>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-L${LIBCXX_BUILD}/lib>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-Wl,-rpath,${LIBCXX_BUILD}/lib>)
# Linking against the standard c++ library is required for CMake to get the proper dependencies.
link_libraries(std c++)
link_libraries(std.compat c++)
# Use custom libc++ compiled without exception 'feature'

# Add the project
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)
find_package(SDL2 REQUIRED)
add_executable(sdl_handmade)
target_include_directories(sdl_handmade PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(sdl_handmade PRIVATE ${SDL2_LIBRARIES})
target_sources(sdl_handmade
    PUBLIC
        sdl_main.cpp
        sdl_platform_impl.cpp
        handmade_impl.cpp)
target_sources(sdl_handmade
    PRIVATE
    FILE_SET CXX_MODULES FILES
    platform.cppm
    handmade.cppm)
target_compile_features(sdl_handmade PRIVATE cxx_std_23)
