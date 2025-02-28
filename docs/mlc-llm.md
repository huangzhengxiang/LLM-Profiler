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
export ANDROID_NDK=~/NDK/android-ndk/
export TVM_NDK_CC=
export MLC_LLM_SOURCE_DIR=~/Desktop/ANL/Project/AIoT-Demo/mlc-llm/
export TVM_SOURCE_DIR=${MLC_LLM_SOURCE_DIR}/3rdparty/tvm/

   