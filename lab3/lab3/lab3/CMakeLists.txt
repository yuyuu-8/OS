cmake_minimum_required(VERSION 3.14)

project(marker_threads_lab VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()