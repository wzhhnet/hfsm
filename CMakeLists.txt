###############################################################################
#    Model Element   : CMakeLists
#    Component       : hfsm
#    Copyright       : Copyright (c) 2021, MXNavi Co.,Ltd. ALL RIGHTS RESERVED
#    File Name       : CMakeLists.txt
#    Author          : wanch
###############################################################################
cmake_minimum_required(VERSION 3.4)

project(hfsm
  LANGUAGES C CXX
  VERSION "1.0.0"
)

option(BUILD_TEST "building example code" OFF)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(SRC_PATH ${PROJECT_SOURCE_DIR})
set(GTEST_PATH ${SRC_PATH}/test/gtest)
include_directories(${SRC_PATH})
aux_source_directory(. SRC_LIBS)

if (BUILD_TEST)
include_directories(${GTEST_PATH})
#add_subdirectory(test/gtest)
file(GLOB GTEST_SRCS ${GTEST_PATH}/src/gtest-all.cpp)
aux_source_directory(./test TEST_SRCS)
#file(GLOB SRC_TEST test.c)
endif ()

set(STATIC_LIB_NAME ${PROJECT_NAME}-static)
set(SHARED_LIB_NAME ${PROJECT_NAME}-shared)

if (BUILD_TEST)
set(TEST_EXEC_NAME ${PROJECT_NAME}-unit)
endif ()

add_library(${STATIC_LIB_NAME} STATIC ${SRC_LIBS})
add_library(${SHARED_LIB_NAME} SHARED ${SRC_LIBS})
target_link_libraries(${STATIC_LIB_NAME} INTERFACE pthread)
target_link_libraries(${SHARED_LIB_NAME} LINK_PUBLIC -lpthread)

set_target_properties(${STATIC_LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${SHARED_LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

if (BUILD_TEST)
add_executable(${TEST_EXEC_NAME} ${TEST_SRCS} ${GTEST_SRCS})
target_link_libraries(${TEST_EXEC_NAME} LINK_PUBLIC ${STATIC_LIB_NAME} gtest)
endif ()

