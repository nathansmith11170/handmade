cmake_minimum_required(VERSION 3.5.0)
project(handmade VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package(SDL2 REQUIRED)
include_directories(SDL2Test ${SDL2_INCLUDE_DIRS})

add_executable(sdl_handmade sdl_handmade.cpp)
set_target_properties(sdl_handmade
    PROPERTIES
    CXX_STANDARD 23
    CXX_EXTENSIONS Off
)
target_compile_options(sdl_handmade PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(sdl_handmade ${SDL2_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
