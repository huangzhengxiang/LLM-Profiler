cmake_minimum_required(VERSION 3.6)
project(llm-profiler)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# compile MNN first
set(MNN_USE_LOGCAT ON CACHE BOOL "Open MNN_USE_LOGCAT" FORCE)
set(MNN_BUILD_LLM ON CACHE BOOL "Open MNN_BUILD_LLM" FORCE)
set(MNN_LOW_MEMORY ON CACHE BOOL "Open MNN_LOW_MEMORY" FORCE)
set(MNN_SUPPORT_TRANSFORMER_FUSE ON CACHE BOOL "Open MNN_SUPPORT_TRANSFORMER_FUSE" FORCE)
set(MNN_ARM82 ON CACHE BOOL "Open MNN_ARM82" FORCE)
set(LLM_SUPPORT_AUDIO ON CACHE BOOL "Open LLM_SUPPORT_AUDIO" FORCE)
set(MNN_BUILD_AUDIO ON CACHE BOOL "Open MNN_BUILD_AUDIO" FORCE)
set(LLM_SUPPORT_VISION ON CACHE BOOL "Open LLM_SUPPORT_VISION" FORCE)
add_definitions(-DLLM_SUPPORT_VISION)
set(MNN_BUILD_OPENCV ON CACHE BOOL "Open MNN_BUILD_OPENCV" FORCE)
set(MNN_IMGCODECS ON CACHE BOOL "Open MNN_IMGCODECS" FORCE)
set(MNN_OPENCL ON CACHE BOOL "Open MNN_OPENCL" FORCE)
# compile llama.cpp
set(LLAMA_BUILD_COMMON ON CACHE BOOL "Open LLAMA_BUILD_COMMON" FORCE)

# padd source directory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/MNN)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/llama.cpp)

# include dir
include_directories(
                    # jni include
                    ${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/include/
                    # llama.cpp include
                    ${CMAKE_CURRENT_LIST_DIR}/llama.cpp/ggml/include
                    ${CMAKE_CURRENT_LIST_DIR}/llama.cpp/include
                    ${CMAKE_CURRENT_LIST_DIR}/llama.cpp/common
                    # MNN include
        `           ${CMAKE_CURRENT_LIST_DIR}/MNN/transformers/llm/engine/include/
                    ${CMAKE_CURRENT_LIST_DIR}/MNN/include/
                    ${CMAKE_CURRENT_LIST_DIR}/MNN/tools/cv/include/
                    ${CMAKE_CURRENT_LIST_DIR}/MNN/3rd_party/
                    )

# jni file
file(GLOB SRCS "${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/*.cpp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
# compile dynamic so, support Linux/Mac
add_library(iot SHARED ${SRCS})
set_target_properties(iot PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
target_link_libraries(iot MNN MNN_Express)
target_link_libraries(iot MNNOpenCV)
target_link_libraries(iot llm)
target_link_libraries(iot MNN_CL)
# llama.cpp
target_link_libraries(iot common llama ggml ggml-base ggml-cpu ${CMAKE_THREAD_LIBS_INIT})

set(LLM_LIBS "")
list(APPEND LLM_LIBS $<TARGET_FILE:MNN>)
list(APPEND LLM_LIBS $<TARGET_FILE:MNN_Express>)
list(APPEND LLM_LIBS $<TARGET_FILE:MNNOpenCV>)
list(APPEND LLM_LIBS $<TARGET_FILE:MNN_CL>)
list(APPEND LLM_LIBS $<TARGET_FILE:MNNAudio>)
list(APPEND LLM_LIBS $<TARGET_FILE:llm>)
list(APPEND LLM_LIBS $<TARGET_FILE:llama>)
list(APPEND LLM_LIBS $<TARGET_FILE:ggml>)
list(APPEND LLM_LIBS $<TARGET_FILE:ggml-base>)
list(APPEND LLM_LIBS $<TARGET_FILE:ggml-cpu>)
add_custom_command(TARGET iot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${LLM_LIBS}
            ${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/libs/arm64-v8a)
