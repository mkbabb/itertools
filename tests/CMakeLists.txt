project(itertools)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_COMPILER "clang++")

add_compile_options(-fcoroutines-ts -stdlib=libc++ -std=c++2a -ggdb3)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup()

add_executable(${PROJECT_NAME} tests.cpp)
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})




