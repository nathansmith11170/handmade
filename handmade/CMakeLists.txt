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

# Use custom libc++ compiled without support for exceptions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -stdlib=libc++")
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-nostdlib++>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-nostdinc++>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-L/usr/local/lib>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-Wl,-rpath,/usr/local/lib>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-lc++>)

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
        platform.hpp
        handmade.hpp
        handmade_impl.cpp)
target_compile_features(sdl_handmade PRIVATE cxx_std_23)
