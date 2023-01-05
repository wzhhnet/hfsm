###############################################################################
#    Model Element   : CMakeLists
#    Component       : EventHub
#    File Name       : CMakeLists.txt
#    Author          : wanch
###############################################################################
cmake_minimum_required(VERSION 3.4)

project(cpphfsm
  LANGUAGES CXX
  VERSION "1.0.0"
)

add_library(${PROJECT_NAME} StateMachine.cpp)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC eventhub)

if (SAMPLE)
set(SAMPLE_NAME hfsm_sample)
add_executable(${SAMPLE_NAME} SampleSM.cpp)
target_link_libraries(${SAMPLE_NAME} LINK_PUBLIC ${PROJECT_NAME})
endif ()
