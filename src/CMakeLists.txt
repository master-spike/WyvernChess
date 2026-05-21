set(WYVERN_ENGINE_SOURCES
    evaluate.cpp
    magicbb.cpp
    movegen.cpp
    position.cpp
    search.cpp
    transposition.cpp
    utils.cpp
)

add_library(wyvern_engine STATIC ${WYVERN_ENGINE_SOURCES})
target_include_directories(wyvern_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
wyvern_apply_common_options(wyvern_engine)

add_executable(wyvernchess main.cpp)
target_link_libraries(wyvernchess PRIVATE wyvern_engine)
set_target_properties(wyvernchess PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
wyvern_apply_common_options(wyvernchess)
