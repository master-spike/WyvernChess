add_executable(wyvern_tests
    perft_tests.cpp
)

target_link_libraries(wyvern_tests PRIVATE wyvern_engine)
wyvern_apply_common_options(wyvern_tests)

add_test(NAME wyvern_tests COMMAND wyvern_tests)
