cmake_minimum_required(VERSION 3.22)

project(TeaJS)

include(src/CMakeLists.txt)
set(MAIN_SRC ${SRC_SRC})

add_executable(TeaJS ${MAIN_SRC})

include_directories(TeaJS src/)

#execute_process(COMMAND python3 runTests.py)