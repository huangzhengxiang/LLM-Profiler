include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

if(DYNAMIC_LOAD_SYMBOLS)
add_definitions(-DDYNAMIC_LOAD_SYMBOLS)
endif()

set(WRAPPER_HEADER ${CMAKE_CURRENT_LIST_DIR}/include/llm_wrapper.h)

add_library(mnn_wrapper SHARED ${CMAKE_CURRENT_LIST_DIR}/src/mnn_wrapper.cpp ${WRAPPER_HEADER})
add_library(llama_cpp_wrapper SHARED ${CMAKE_CURRENT_LIST_DIR}/src/llama_cpp_wrapper.cpp ${WRAPPER_HEADER})
if (BUILD_ANDROID)
# also mllm forgotten files
add_library(mllm_wrapper SHARED ${CMAKE_CURRENT_LIST_DIR}/src/mllm_wrapper.cpp ${CMAKE_CURRENT_LIST_DIR}/../mllm/src/tokenizers/Tiktoken/tiktoken.cpp ${WRAPPER_HEADER})
endif()

# include pre_build et_wrapper
if (BUILD_ANDROID)
    add_library(et_wrapper SHARED IMPORTED GLOBAL) 
    set_target_properties(et_wrapper PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/android/libet_wrapper.so)
endif()

# MNN
set_target_properties(mnn_wrapper PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
target_link_libraries(mnn_wrapper MNN MNN_Express)
target_link_libraries(mnn_wrapper MNNOpenCV)
target_link_libraries(mnn_wrapper llm)
if(BUILD_ANDROID)
target_link_libraries(mnn_wrapper MNN_CL)
endif()
if(BUILD_IOS)
target_link_libraries(mnn_wrapper MNNMetal)
endif()
# llama.cpp
target_link_libraries(llama_cpp_wrapper common llama ggml ggml-base ggml-cpu ${CMAKE_THREAD_LIBS_INIT})
# mllm
if (BUILD_ANDROID)
target_link_libraries(mllm_wrapper mllm_lib)
endif()

if (BUILD_IOS)
# Set linker flags to export all symbols
set_target_properties(mnn_wrapper PROPERTIES FRAMEWORK TRUE)
set_target_properties(mnn_wrapper PROPERTIES
    MACOSX_FRAMEWORK_IDENTIFIER com.hzx.llm.mnn_wrapper
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PACKAGE_VERSION}
    MACOSX_FRAMEWORK_BUNDLE_VERSION ${PACKAGE_VERSION}
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
)
set_target_properties(MNN PROPERTIES MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/../MNN/project/ios/MNN/Info.plist)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_LIST_DIR}/include/llm_wrapper.h PROPERTIES MACOSX_PACKAGE_LOCATION Headers/)

# set_target_properties(llama_cpp_wrapper PROPERTIES FRAMEWORK TRUE)
# set_target_properties(llama_cpp_wrapper PROPERTIES
#     MACOSX_FRAMEWORK_IDENTIFIER com.hzx.llm.llama_cpp_wrapper
#     MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PACKAGE_VERSION}
#     MACOSX_FRAMEWORK_BUNDLE_VERSION ${PACKAGE_VERSION}
#     XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
# )
# set_target_properties(MNN PROPERTIES MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/../MNN/project/ios/MNN/Info.plist)
# SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_LIST_DIR}/include/llm_wrapper.h PROPERTIES MACOSX_PACKAGE_LOCATION Headers/)
# set_target_properties(llama PROPERTIES MACOSX_PACKAGE_LOCATION "")
# set_target_properties(ggml PROPERTIES MACOSX_PACKAGE_LOCATION "")
# set_target_properties(ggml-base PROPERTIES MACOSX_PACKAGE_LOCATION "")
# set_target_properties(ggml-cpu PROPERTIES MACOSX_PACKAGE_LOCATION "")
endif()