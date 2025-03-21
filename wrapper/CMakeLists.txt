
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_library(mnn_wrapper SHARED src/mnn_wrapper.cpp)
add_library(llama_cpp_wrapper SHARED src/llama_cpp_wrapper.cpp)
# add_library(mllm_wrapper SHARED src/mllm_wrapper.cpp)

if (BUILD_ANDROID)
    add_library(et_wrapper SHARED IMPORTED GLOBAL) 
    set_target_properties(et_wrapper PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/android/libet_wrapper.so)
endif()

# MNN
set_target_properties(mnn_wrapper PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
target_link_libraries(mnn_wrapper MNN MNN_Express)
target_link_libraries(mnn_wrapper MNNOpenCV)
target_link_libraries(mnn_wrapper llm)
target_link_libraries(mnn_wrapper MNN_CL)
# llama.cpp
target_link_libraries(llama_cpp_wrapper common llama ggml ggml-base ggml-cpu ${CMAKE_THREAD_LIBS_INIT})
# mllm
# target_link_libraries(mllm_wrapper mllm_lib)