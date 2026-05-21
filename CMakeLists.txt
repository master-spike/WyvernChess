cmake_minimum_required(VERSION 3.16)

project(WyvernChess LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(WYVERN_BUILD_TESTS "Build WyvernChess tests" ON)
option(WYVERN_ENABLE_LTO "Enable interprocedural optimization when supported" ON)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT err)

function(wyvern_apply_common_options target_name)
    target_compile_features(${target_name} PUBLIC cxx_std_20)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W1 /WX /EHsc)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Werror)
    endif()

    if(WYVERN_ENABLE_LTO AND ipo_supported)
        set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endfunction()

if(WYVERN_ENABLE_LTO AND ipo_supported)
    message(STATUS "LTO enabled")
elseif(WYVERN_ENABLE_LTO)
    message(STATUS "LTO not supported: <${err}>")
endif()

add_subdirectory(src)

if(WYVERN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
