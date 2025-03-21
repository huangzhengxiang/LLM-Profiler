### MLC-LLM

https://llm.mlc.ai/docs/deploy/android.html

1. Python package
```bash
python -m pip install --pre -U -f https://mlc.ai/wheels mlc-llm-nightly-cpu mlc-ai-nightly-cpu
# verify it
mlc-llm -h
```

2. Rust, NDK, JDK, TVM Unity runtime


3. Model and Quant
https://llm.mlc.ai/docs/compilation/configure_quantization.html


4. config android dependencies
Cross-compilation with rust is difficult: https://github.com/rust-lang/cargo/issues/7611 (linker `cc` shall be set to host linker, while cross linker is already set by `export CC_aarch64_linux_android=`)
```bash
export PATH=${PATH}:~/.cargo/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/lib
export JAVA_HOME=~/.jdks/corretto-18.0.2/
export ANDROID_NDK=~/Android/Sdk/ndk/27.0.12077973/
export TVM_NDK_CC=${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang
export MLC_LLM_SOURCE_DIR=~/Desktop/ANL/Project/AIoT-Demo/mlc-llm/
export TVM_SOURCE_DIR=${MLC_LLM_SOURCE_DIR}/3rdparty/tvm/
export PATH=${PATH}:${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/
export CC_aarch64_linux_android=${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang
export CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER=${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang
```
