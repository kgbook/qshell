cmake_minimum_required(VERSION 3.8)
project(qshell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

option(QSLOG_BUILD_TESTS "build tests" ON)
option(QSLOG_BUILD_EXAMPLES "build examples" ON)
# clang-tidy
option(QSLOG_CLANG_TIDY_ENABLE "run clang-tidy" ON)
if(QSLOG_CLANG_TIDY_ENABLE AND ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "enabled clang-tidy")
endif()

message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

add_subdirectory(third_party)
add_subdirectory(src)


if (QSLOG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (QSLOG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
