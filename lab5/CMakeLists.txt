cmake_minimum_required(VERSION 3.10)
project(EmployeeManager)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(server src/server.cpp)
add_executable(client src/client.cpp)

enable_testing()
add_subdirectory(tests)
