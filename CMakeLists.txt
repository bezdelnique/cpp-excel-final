cmake_minimum_required(VERSION 3.16)
project(cpp_excel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wfatal-errors -Werror=pedantic -Wno-unused-parameter -Wno-unused-function -fpermissive")

set(CPP_EXCEL_FILES
        common.h
        main.cpp
        structures.cpp
        test_runner_p.h
        )

# == cpp_excel ==
add_executable(cpp_excel ${CPP_EXCEL_FILES})

