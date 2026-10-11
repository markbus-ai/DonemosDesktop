find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(donemos_tests
    DateUtilsTest.cpp
)

target_include_directories(donemos_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(donemos_tests PRIVATE Qt6::Test)

add_test(NAME donemos_tests COMMAND donemos_tests)
