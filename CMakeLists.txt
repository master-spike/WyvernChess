cmake_minimum_required(VERSION 3.10)

project(WyvernChess)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/Ox /W1 /WX /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /W1 /WX /EHsc")
else()
    set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Werror")
endif()

set(CMAKE_CXX_STANDARD 20)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT err)

add_executable(wyvernchess src/main.cpp
    src/evaluate.cpp
    src/magicbb.cpp
    src/movegen.cpp
    src/position.cpp
    src/search.cpp
    src/transposition.cpp
    src/utils.cpp
)

if(ipo_supported)
    message(STATUS "LTO enabled")
    set_property(TARGET wyvernchess PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported: <${err}>")
endif()
