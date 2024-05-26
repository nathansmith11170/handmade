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
