cmake_minimum_required(VERSION 3.10)
project(MessagePassing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(receiver receiver.cpp)
add_executable(sender sender.cpp)
add_executable(tests tests.cpp)