cmake_minimum_required(VERSION 3.6)
project(llm-profiler)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# control Android/iOS Build: only 1 can set to be ON
option(BUILD_ANDROID "build LLM-Profiler for Android" ON)
option(BUILD_IOS "build LLM-Profiler for iOS" OFF)
# if both ON, build android
if (BUILD_ANDROID AND BUILD_IOS)
    set(BUILD_IOS OFF)
endif()

# set flags
if (BUILD_ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -llog") # additional link to android/log
    set(DYNAMIC_LOAD_SYMBOLS ON) # dynamically load jni symbols in Java end
else()
    if (BUILD_IOS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
        set(DYNAMIC_LOAD_SYMBOLS OFF) # build ordinary shared library logic
    endif()
endif()

# compile MNN first
if (BUILD_ANDROID)
set(MNN_USE_LOGCAT ON CACHE BOOL "Open MNN_USE_LOGCAT" FORCE)
endif()
set(MNN_BUILD_LLM ON CACHE BOOL "Open MNN_BUILD_LLM" FORCE)
set(MNN_LOW_MEMORY ON CACHE BOOL "Open MNN_LOW_MEMORY" FORCE)
set(MNN_SUPPORT_TRANSFORMER_FUSE ON CACHE BOOL "Open MNN_SUPPORT_TRANSFORMER_FUSE" FORCE)
set(MNN_CPU_WEIGHT_DEQUANT_GEMM ON CACHE BOOL "Open MNN_CPU_WEIGHT_DEQUANT_GEMM" FORCE)
set(MNN_ARM82 ON CACHE BOOL "Open MNN_ARM82" FORCE)
set(LLM_SUPPORT_AUDIO ON CACHE BOOL "Open LLM_SUPPORT_AUDIO" FORCE)
set(MNN_BUILD_AUDIO ON CACHE BOOL "Open MNN_BUILD_AUDIO" FORCE)
set(LLM_SUPPORT_VISION ON CACHE BOOL "Open LLM_SUPPORT_VISION" FORCE)
add_definitions(-DLLM_SUPPORT_VISION)
set(MNN_BUILD_OPENCV ON CACHE BOOL "Open MNN_BUILD_OPENCV" FORCE)
set(MNN_IMGCODECS ON CACHE BOOL "Open MNN_IMGCODECS" FORCE)
if (BUILD_ANDROID)
set(MNN_OPENCL ON CACHE BOOL "Open MNN_OPENCL" FORCE)
endif()
if (BUILD_IOS)
set(MNN_OPENCL OFF CACHE BOOL "Close MNN_OPENCL" FORCE)
set(MNN_METAL ON CACHE BOOL "Open MNN_METAL" FORCE)
set(MNN_USE_THREAD_POOL OFF CACHE BOOL "Close MNN_USE_THREAD_POOL, iOS dosen't support CPU affinity" FORCE)
set(MNN_BUILD_SHARED_LIBS OFF CACHE BOOL "Close MNN_BUILD_SHARED_LIBS" FORCE)
set(MNN_SEP_BUILD OFF CACHE BOOL "Close MNN_SEP_BUILD" FORCE)
# Do not use MNN.framework, but mnn_wrapper
endif()
# compile llama.cpp
set(LLAMA_BUILD_COMMON ON CACHE BOOL "Open LLAMA_BUILD_COMMON" FORCE)
# compile mllm
set(ARM ON CACHE BOOL "Open mllm compilatin for ARM" FORCE)
set(APK ON CACHE BOOL "Open mllm compilation for APK" FORCE)
set(QNN ON CACHE BOOL "Open mllm compilation for QNN" FORCE)
set(DEBUG OFF CACHE BOOL "Close mllm debug" FORCE)
set(TEST OFF  CACHE BOOL "Close mllm test" FORCE)
set(QUANT OFF CACHE BOOL "Close mllm quant" FORCE)
set(QNN_VALIDATE_NODE ON CACHE BOOL "Open mllm QNN_VALIDATE_NODE" FORCE)
set(MLLM_BUILD_XNNPACK_BACKEND OFF CACHE BOOL "Close mllm xnnpack, use its own op implmentation" FORCE)

# include dir
include_directories(
                    ${CMAKE_CURRENT_LIST_DIR}/dataset/include
                    ${CMAKE_CURRENT_LIST_DIR}/wrapper/include
                    # llama.cpp include
                    ${CMAKE_CURRENT_LIST_DIR}/llama.cpp/ggml/include
                    ${CMAKE_CURRENT_LIST_DIR}/llama.cpp/include
                    ${CMAKE_CURRENT_LIST_DIR}/llama.cpp/common
                    # mllm include
                    ${CMAKE_CURRENT_LIST_DIR}/mllm/include
                    ${CMAKE_CURRENT_LIST_DIR}/mllm/src
                    ${CMAKE_CURRENT_LIST_DIR}/mllm/include
                    ${CMAKE_CURRENT_LIST_DIR}/mllm/third_party
                    ${CMAKE_CURRENT_LIST_DIR}/mllm/third_party/fmt/include
                    # MNN include
                    ${CMAKE_CURRENT_LIST_DIR}/MNN/tools/audio/include/
                    ${CMAKE_CURRENT_LIST_DIR}/MNN/transformers/llm/engine/include/
                    ${CMAKE_CURRENT_LIST_DIR}/MNN/include/
                    ${CMAKE_CURRENT_LIST_DIR}/MNN/tools/cv/include/
                    ${CMAKE_CURRENT_LIST_DIR}/MNN/3rd_party/
                    )


if (BUILD_ANDROID)
# jni include
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/include/
    )

# add dataset wrapper
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dataset)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/wrapper)

# add engine source directory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/MNN)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/llama.cpp)
# add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/mllm)

# jni file
set(SRCS "${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/iot_audio_jni.cpp")
# mllm forgotten files
# list(APPEND SRCS "mllm/src/tokenizers/Tiktoken/tiktoken.cpp")

# compile dynamic so, support Linux/Mac
add_library(mnn_jni SHARED ${SRCS})
add_library(dataset_jni SHARED "${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/dataset_jni.cpp")
target_link_libraries(dataset_jni dataset)
target_link_libraries(mnn_jni dataset mnn_wrapper)
# llama.cpp
add_library(llama_cpp_jni SHARED ${SRCS})
target_link_libraries(llama_cpp_jni dataset llama_cpp_wrapper)
# mllm
# add_library(mllm_jni SHARED ${SRCS})
# target_link_libraries(mllm_jni dataset mllm_wrapper)
# executorch
add_library(et_jni SHARED ${SRCS})
target_link_libraries(et_jni dataset et_wrapper)


set(MNN_LLM_LIBS "")
set(LLAMA_CPP_LLM_LIBS "")
# set(MLLM_LLM_LIBS "")
set(ET_LLM_LIBS "")
list(APPEND MNN_LLM_LIBS $<TARGET_FILE:dataset>)
list(APPEND LLAMA_CPP_LLM_LIBS $<TARGET_FILE:dataset>)
# list(APPEND MLLM_LLM_LIBS $<TARGET_FILE:dataset>)
list(APPEND ET_LLM_LIBS $<TARGET_FILE:dataset>)
# MNN
list(APPEND MNN_LLM_LIBS $<TARGET_FILE:MNN>)
list(APPEND MNN_LLM_LIBS $<TARGET_FILE:MNN_Express>)
list(APPEND MNN_LLM_LIBS $<TARGET_FILE:MNNOpenCV>)
list(APPEND MNN_LLM_LIBS $<TARGET_FILE:MNN_CL>)
list(APPEND MNN_LLM_LIBS $<TARGET_FILE:MNNAudio>)
list(APPEND MNN_LLM_LIBS $<TARGET_FILE:mnn_wrapper>)
# llama.cpp
list(APPEND LLAMA_CPP_LLM_LIBS $<TARGET_FILE:llm>)
list(APPEND LLAMA_CPP_LLM_LIBS $<TARGET_FILE:llama>)
list(APPEND LLAMA_CPP_LLM_LIBS $<TARGET_FILE:ggml>)
list(APPEND LLAMA_CPP_LLM_LIBS $<TARGET_FILE:ggml-base>)
list(APPEND LLAMA_CPP_LLM_LIBS $<TARGET_FILE:ggml-cpu>)
list(APPEND LLAMA_CPP_LLM_LIBS $<TARGET_FILE:llama_cpp_wrapper>)
# mllm
# list(APPEND MLLM_LLM_LIBS $<TARGET_FILE:mllm_wrapper>)
# executorch
list(APPEND ET_LLM_LIBS $<TARGET_FILE:et_wrapper>)

# after the last one finished
add_custom_command(TARGET mnn_jni POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${MNN_LLM_LIBS}
            ${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/libs/arm64-v8a)
add_custom_command(TARGET llama_cpp_jni POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${LLAMA_CPP_LLM_LIBS}
            ${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/libs/arm64-v8a)
# add_custom_command(TARGET mllm_jni POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#             ${MLLM_LLM_LIBS}
#             ${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/libs/arm64-v8a)
add_custom_command(TARGET et_jni POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${ET_LLM_LIBS}
            ${CMAKE_CURRENT_LIST_DIR}/android2/app/src/main/jni/libs/arm64-v8a)
endif()


if(BUILD_IOS)
# add source directory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/MNN)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/llama.cpp)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dataset)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/wrapper)

endif()