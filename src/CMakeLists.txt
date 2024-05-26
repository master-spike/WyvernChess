set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Werror")

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT err)


add_executable(wyvernchess main.cpp 
    evaluate.cpp
    magicbb.cpp
    movegen.cpp
    position.cpp
    search.cpp
    transposition.cpp
    utils.cpp
    zobrist.cpp
)

if(ipo_supported)
    message(STATUS "LTO enabled")
    set_property(TARGET wyvernchess PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported: <${err}>")
endif()
