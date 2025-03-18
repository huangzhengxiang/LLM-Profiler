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
Cross-compilation with rust is difficult: https://github.com/rust-lang/cargo/issues/7611
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


LC_ALL="C" PATH="/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/bin:/home/hzx/miniconda3/envs/llm/bin:/home/hzx/.local/bin:/home/hzx/miniconda3/condabin:/home/hzx/.cargo/bin:/home/hzx/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/snap/bin:~:~/android-studio/bin:/toolchains/llvm/prebuilt/linux-x86_64/bin/:/home/hzx/Android/Sdk/ndk/27.0.12077973//toolchains/llvm/prebuilt/linux-x86_64/bin/" VSLANG="1033" "cc" "-m64" "/tmp/rustcoEHsvS/symbols.o" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/build/rayon-core-793f7ae606df9327/build_script_build-793f7ae606df9327.build_script_build.2daec9ee-cgu.0.rcgu.o" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/build/rayon-core-793f7ae606df9327/build_script_build-793f7ae606df9327.build_script_build.2daec9ee-cgu.1.rcgu.o" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/build/rayon-core-793f7ae606df9327/build_script_build-793f7ae606df9327.build_script_build.2daec9ee-cgu.2.rcgu.o" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/build/rayon-core-793f7ae606df9327/build_script_build-793f7ae606df9327.build_script_build.2daec9ee-cgu.3.rcgu.o" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/build/rayon-core-793f7ae606df9327/build_script_build-793f7ae606df9327.build_script_build.2daec9ee-cgu.4.rcgu.o" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/build/rayon-core-793f7ae606df9327/build_script_build-793f7ae606df9327.build_script_build.2daec9ee-cgu.5.rcgu.o" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/build/rayon-core-793f7ae606df9327/build_script_build-793f7ae606df9327.9zpsglg4uad5gij.rcgu.o" "-Wl,--as-needed" "-L" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/deps" "-L" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib" "-Wl,-Bstatic" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libstd-8389830094602f5a.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libpanic_unwind-41c1085b8c701d6f.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libobject-f733fcc57ce38b99.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libmemchr-6495ec9d4ce4f37d.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libaddr2line-1e3796360cca5b49.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libgimli-2e7f329b154436e1.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/librustc_demangle-1e1f5b8a84008aa8.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libstd_detect-cbcb223c64b13cf3.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libhashbrown-b40bc72e060a8196.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libminiz_oxide-1eb33ae9877d3c0f.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libadler-0335d894dd05bed7.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/librustc_std_workspace_alloc-076a893ead7e7ab5.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libunwind-2e924dd85b2e9d95.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libcfg_if-7975ffb5e62386c4.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/liblibc-285425b7cea12024.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/liballoc-38694d775e998991.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/librustc_std_workspace_core-914eb40be05d8663.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libcore-27094fcca7e14863.rlib" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib/libcompiler_builtins-919e055b306699ae.rlib" "-Wl,-Bdynamic" "-Wl,--eh-frame-hdr" "-Wl,-z,noexecstack" "-L" "/home/hzx/.rustup/toolchains/1.70-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/lib" "-o" "/home/hzx/Desktop/ANL/Project/AIoT-Demo/mlc-llm/3rdparty/tokenizers-cpp/build/release/build/rayon-core-793f7ae606df9327/build_script_build-793f7ae606df9327" "-Wl,--gc-sections" "-pie" "-Wl,-z,relro,-z,now" "-nodefaultlibs"