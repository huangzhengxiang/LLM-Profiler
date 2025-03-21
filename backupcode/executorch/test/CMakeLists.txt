cmake_minimum_required(VERSION 3.18)
project(unit_test)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../../wrapper/include)
add_library(et_wrapper SHARED IMPORTED GLOBAL)
set_target_properties(et_wrapper PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/../../../wrapper/lib/android/libet_wrapper.so)

add_executable(test test.cpp)
target_link_libraries(test et_wrapper)