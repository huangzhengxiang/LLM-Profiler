cmake_minimum_required(VERSION 3.24) # 3.24 is required for WHOLE_ARCHIVE

project(et_wrapper)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(EXECUTORCH_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../executorch")
include(${EXECUTORCH_ROOT}/tools/cmake/Utils.cmake)
set(_common_compile_options -Wno-deprecated-declarations -fPIC)
set(_common_include_directories ${EXECUTORCH_ROOT}/..)
if(NOT ANDROID_PLATFORM)
  set(ANDROID_PLATFORM android-30)
endif()

set(executorch_DIR ${CMAKE_CURRENT_BINARY_DIR}/../../lib/cmake/ExecuTorch)
find_package(executorch CONFIG REQUIRED)
target_link_options_shared_lib(executorch)

include_directories("${CMAKE_CURRENT_LIST_DIR}/../include")
file(GLOB SRCS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
add_library(et_wrapper SHARED ${SRCS})

set(link_libraries)
list(
  APPEND
  link_libraries
  executorch
  extension_data_loader
  extension_module
  extension_runner_util
  extension_tensor
  extension_threadpool
)

if(TARGET optimized_native_cpu_ops_lib)
  list(
    APPEND
    link_libraries
    optimized_native_cpu_ops_lib
  )
  target_link_options_shared_lib(optimized_native_cpu_ops_lib)
else()
  list(APPEND link_libraries portable_ops_lib portable_kernels)
  target_link_options_shared_lib(portable_ops_lib)
endif()

if(TARGET quantized_kernels)
  list(APPEND link_libraries quantized_kernels quantized_ops_lib)
  target_link_options_shared_lib(quantized_ops_lib)
endif()

if(TARGET qnn_executorch_backend)
  list(APPEND link_libraries qnn_executorch_backend)
endif()

if(TARGET xnnpack_backend)
  target_link_options_shared_lib(xnnpack_backend)
  list(APPEND link_libraries xnnpack_backend XNNPACK pthreadpool cpuinfo microkernels-prod)
endif()

if(TARGET vulkan_backend)
  target_link_options_shared_lib(vulkan_backend)
  list(APPEND link_libraries vulkan_backend)
endif()

if(EXECUTORCH_BUILD_KERNELS_CUSTOM)
  list(APPEND link_libraries $<LINK_LIBRARY:WHOLE_ARCHIVE,custom_ops>)
endif()

if(TARGET pthreadpool)
  target_include_directories(
    et_wrapper
    PUBLIC
    ${EXECUTORCH_ROOT}/backends/xnnpack/third-party/cpuinfo/include
  )
  target_include_directories(
    et_wrapper
    PUBLIC
    ${EXECUTORCH_ROOT}/backends/xnnpack/third-party/pthreadpool/include
  )
endif()

if(et_wrapper_CUSTOM_LIBRARY)
  list(APPEND link_libraries ${et_wrapper_CUSTOM_LIBRARY})
  target_link_libraries(
    et_wrapper -Wl,--whole-archive ${et_wrapper_CUSTOM_LIBRARY}
    -Wl,--no-whole-archive
  )
endif()

if(EXECUTORCH_BUILD_LLAMA_JNI)
  list(APPEND link_libraries llama_runner llava_runner)
  add_subdirectory(
    ${EXECUTORCH_ROOT}/examples/models/llava/runner
    ${CMAKE_CURRENT_BINARY_DIR}/../../examples/models/llava/runner
  )

  add_subdirectory(
    ${EXECUTORCH_ROOT}/examples/models/llama/runner
    ${CMAKE_CURRENT_BINARY_DIR}/../../examples/models/llama/runner
  )

  if(NEURON_BUFFER_ALLOCATOR_LIB)
      target_sources(
      et_wrapper PRIVATE
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/mtk_llama_runner.cpp
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/llama_runner/LlamaModelChunk.cpp
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/llama_runner/LlamaRuntime.cpp
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/llama_runner/ModelChunk.cpp
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/llama_runner/MultiModelLoader.cpp
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/llama_runner/llm_helper/mask_builder.cpp
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/llama_runner/llm_helper/rotary_embedding.cpp
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/llama_runner/llm_helper/token_embedding.cpp
    )
    target_include_directories(
      et_wrapper PRIVATE
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/
      ${EXECUTORCH_ROOT}/examples/mediatek/executor_runner/llama_runner
    )
    add_library(libneuron_buffer_allocator SHARED IMPORTED)
    set_property(TARGET libneuron_buffer_allocator PROPERTY IMPORTED_LOCATION ${NEURON_BUFFER_ALLOCATOR_LIB})
    list(APPEND link_libraries neuron_backend libneuron_buffer_allocator)
    target_compile_definitions(et_wrapper PRIVATE EXECUTORCH_BUILD_MEDIATEK=1)
  endif()
endif()

target_compile_options(et_wrapper PUBLIC ${_common_compile_options})

target_link_libraries(et_wrapper ${link_libraries} log)
