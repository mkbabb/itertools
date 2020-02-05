project(itertools)
cmake_minimum_required(VERSION 3.2)


set(CMAKE_CXX_COMPILER "/usr/local/Cellar/llvm/9.0.0_1/bin/clang++")


add_compile_options(-std=c++2a)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()



add_executable(${PROJECT_NAME} tests.cpp)
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})




