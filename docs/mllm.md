### mllm

See ./mllm/README.md for methods of convering models and tokenizer.json to mllm format. 

See ./mllm/scripts/build* and ./mllm/scripts/run* for methods of building mllm core library and running models from terminal. 
Note that tokenizer doesn't need conversion as they're ready in the directory of ./mllm/vocab

See ./mllm/src/backends/qnn/README.md for QNN NPU supports installation and usage.

#### QNN-Htp supports for Qwen2.5-0.5B-Instruct 


On PC, compile:
```bash
export QNN_SDK_ROOT=mllm/src/backends/qnn/qualcomm_ai_engine_direct_220/
export ANDROID_NDK_ROOT=/path/to/your/ndk
export PATH=$PATH:$ANDROID_NDK_ROOT

source mllm/src/backends/qnn/5.5.0/setup_sdk_env.source
source $QNN_SDK_ROOT/bin/envsetup.sh

cd mllm/src/backends/qnn/LLaMAOpPackageHtp/LLaMAPackage/
make htp_aarch64 && make htp_v75
```

On PC, convert:
```bash
python tools/convertor/converter.py --input_model ../model/Qwen2.5-0.5B-Instruct/model.safetensors --output_model build/model/qwen2_5-0_5b-instruct-npu-mllm/16bit.mllm --type=safetensor
./build/pc/bin/quantize build/model/qwen2_5-0_5b-instruct-npu-mllm/16bit.mllm build/model/qwen2_5-0_5b-instruct-npu-mllm/model_int8.mllm Q8_0
./build/pc/bin/quantize build/model/qwen2_5-0_5b-instruct-npu-mllm/16bit.mllm build/model/qwen2_5-0_5b-instruct-npu-mllm/model_q4_0_4_4.mllm Q4_0_4_4
cp vocab/qwen2.5_* build/model/qwen2_5-0_5b-instruct-npu-mllm/
adb push build/model/qwen2_5-0_5b-instruct-npu-mllm/ /data/local/tmp/llm/model/
```


```bash
export LIBPATH=./src/backends/qnn/qualcomm_ai_engine_direct_220/
export ANDR_LIB=$LIBPATH/lib/aarch64-android
export OP_PATH=./src/backends/qnn/LLaMAOpPackageHtp/LLaMAPackage/build
export DEST=/data/local/tmp/llm/

adb push $ANDR_LIB/libQnnHtp.so $DEST
adb push $ANDR_LIB/libQnnHtpV75Stub.so $DEST
adb push $ANDR_LIB/libQnnHtpPrepare.so $DEST
adb push $ANDR_LIB/libQnnHtpProfilingReader.so $DEST
adb push $ANDR_LIB/libQnnHtpOptraceProfilingReader.so $DEST
adb push $ANDR_LIB/libQnnHtpV75CalculatorStub.so $DEST
adb push $LIBPATH/lib/hexagon-v75/unsigned/libQnnHtpV75Skel.so $DEST
adb push $OP_PATH/aarch64-android/libQnnLLaMAPackage.so $DEST/libQnnLLaMAPackage_CPU.so
adb push $OP_PATH/hexagon-v75/libQnnLLaMAPackage.so $DEST/libQnnLLaMAPackage_HTP.so
```

```bash
export LD_LIBRARY_PATH=.
./demo_qwen2.5_npu -v model/qwen2_5-0_5b-instruct-npu-mllm/qwen2.5_vocab.mllm -e model/qwen2_5-0_5b-instruct-npu-mllm/qwen2.5_merges.txt -m model/qwen2_5-0_5b-instruct-npu-mllm/model_int8.mllm -d model/qwen2_5-0_5b-instruct-npu-mllm/model_q4_0_4_4.mllm -b 0.5B -t 6 
./demo_qwen -v model/qwen2_5-0_5b-instruct-npu-mllm/qwen2.5_vocab.mllm -e model/qwen2_5-0_5b-instruct-npu-mllm/qwen2.5_merges.txt -m model/qwen2_5-0_5b-instruct-npu-mllm/model_q4_0_4_4.mllm -b 0.5B -t 6 
```