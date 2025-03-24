if conda env list | grep executorch; then
    echo "Environment executorch already exists."
else
    echo "Environment executorch does not exist. Creating it..."
    conda create -n executorch python=3.10
    pushd .
    cd executorch
    ./install_executorch.sh --pybind xnnpack
    popd
fi
conda activate executorch

if [ $# -gt 0 ]; then
    if [ "$1" = "android" ]; then
        target="ANDROID"
    elif [ "$1" = "ios" ]; then
        target="IOS"
    else
        echo "Invalid argument. Please specify 'android' or 'ios'."
        exit 1
    fi
else
    target="ANDROID"
fi

# build main lib
cd executorch
cmake . -DCMAKE_INSTALL_PREFIX=cmake-out-android \
    -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK}/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-28 \
    -DBUILD_TESTING=OFF \
    -DEXECUTORCH_ENABLE_LOGGING=ON \
    -DEXECUTORCH_LOG_LEVEL=Info \
    -DEXECUTORCH_BUILD_XNNPACK=ON \
    -DEXECUTORCH_XNNPACK_SHARED_WORKSPACE=ON \
    -DEXECUTORCH_BUILD_EXTENSION_DATA_LOADER=ON \
    -DEXECUTORCH_BUILD_EXTENSION_MODULE=ON \
    -DEXECUTORCH_BUILD_EXTENSION_RUNNER_UTIL=ON \
    -DEXECUTORCH_BUILD_EXTENSION_TENSOR=ON \
    -DEXECUTORCH_BUILD_KERNELS_OPTIMIZED=ON \
    -DEXECUTORCH_BUILD_KERNELS_QUANTIZED=ON \
    -DEXECUTORCH_BUILD_KERNELS_CUSTOM=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -Bcmake-out-android
    # -DEXECUTORCH_BUILD_NEURON="${EXECUTORCH_BUILD_NEURON}" \
    # -DNEURON_BUFFER_ALLOCATOR_LIB="${NEURON_BUFFER_ALLOCATOR_LIB}" \
    # -DEXECUTORCH_BUILD_QNN="${EXECUTORCH_BUILD_QNN}" \
    # -DQNN_SDK_ROOT="${QNN_SDK_ROOT}" \
    # -DEXECUTORCH_BUILD_VULKAN="${EXECUTORCH_BUILD_VULKAN}" \

cmake --build cmake-out-android -j16 --target install --config Release

# build wrapper 
if [ ${target} = "ANDROID" ]; then
cmake  -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-28 \
    -DCMAKE_INSTALL_PREFIX=cmake-out-android \
    -DCMAKE_BUILD_TYPE=Release \
    -DPYTHON_EXECUTABLE=python \
    -DEXECUTORCH_BUILD_XNNPACK=ON \
    -DEXECUTORCH_BUILD_KERNELS_OPTIMIZED=ON \
    -DEXECUTORCH_BUILD_KERNELS_QUANTIZED=ON \
    -DEXECUTORCH_BUILD_KERNELS_CUSTOM=ON \
    -DEXECUTORCH_BUILD_LLAMA_JNI=ON \
    -DBUILD_ANDROID=ON \
    -Bcmake-out-android/wrapper/lib \
    ../wrapper/lib
cmake --build "cmake-out-android/wrapper/lib/" -j16 --config Release

mkdir -p ../wrapper/lib/android
cp cmake-out-android/wrapper/lib/libet_wrapper.so ../wrapper/lib/android/libet_wrapper.so
fi