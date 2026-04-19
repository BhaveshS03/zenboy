cmake_minimum_required(VERSION 3.20)
project(zenboy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
    SDL2
    URL "https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-2.28.5.zip"
)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)

# Collect sources
file(GLOB_RECURSE CORE_SOURCES
    "src/core/*.cpp"
    "src/core/mbc/*.cpp"
)
file(GLOB_RECURSE FRONTEND_SOURCES
    "src/frontend/*.cpp"
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${FRONTEND_SOURCES}
)

add_executable(zenboy ${ALL_SOURCES})

target_include_directories(zenboy PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(zenboy PRIVATE SDL2::SDL2main SDL2::SDL2-static)

if(MSVC)
    target_compile_options(zenboy PRIVATE /W4 /O2)
else()
    target_compile_options(zenboy PRIVATE -Wall -Wextra -O2)
endif()
