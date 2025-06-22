find_package(Boost REQUIRED COMPONENTS unit_test_framework)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

add_executable(tests test_employee.cpp)
target_link_libraries(tests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
