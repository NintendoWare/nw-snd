cmake_minimum_required(VERSION 3.0.0)
project(nw_snd VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_STATIC_LIBRARY_PREFIX "")

include(CTest)
enable_testing()

add_library(nw_snd STATIC
    ./src/include/nw/snd/bfwav.h
    ./src/bfwav.cpp
)

target_include_directories(nw_snd PUBLIC ./src/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/ExtendedIO)
target_link_libraries(nw_snd PUBLIC ExtendedIO)

set(BUILD_TESTING ON)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
