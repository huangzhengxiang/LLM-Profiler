## LLM-Profiler

<div align="center">
<img src="./icon/1024.png" width="15%">
</div>

***LLM Inference Engine Profiler (Android/iOS)***

*Key features*: 
- Unplugged testing wrapped in app, no adb needed, better in simulating real-world unplugged use. 
- Temperature is kept below $40^\circ \mathbf{C}$ before each test, and wait the app for a while to cool down automatically to avoid severe CPU&GPU throttling. 
- Charge level is kept to be above 50% to avoid phones from some vendors automatically activating power-saving.
- Support most of phones with Android API Level $\geq 30$, iOS level $\geq 16$.
- android app locates in `android2` folder, and iOS app locates in `ios` folder.

*Currently Supported Engines*:
- [x] [MNN](https://github.com/Embedded-AI-Systems/MNN-Habst.git) (Our Modified Version of MNN-3.0.4) (Android: CPU/GPU-OpenCL) (iOS: )
- [x] [llama.cpp](https://github.com/ggml-org/llama.cpp/tree/73e2ed3ce3492d3ed70193dd09ae8aa44779651d) (Version b4735) (Android: CPU) (iOS: )
- [ ] [mllm](https://github.com/UbiquitousLearning/mllm/tree/bbf87ffb8cb47860cdc2118c06ccad5b4ab84227) (hash bbf87ff) (Android: CPU/NPU-QNN-Htp) (iOS: )
- [x] [MediaPipe](https://github.com/google-ai-edge/mediapipe-samples) (Version 0.10.20) (Android: CPU/GPU-OpenCL) (iOS: )
- [ ] [MLC-LLM](https://github.com/mlc-ai/mlc-llm/tree/b636b2ac5e0c8bac6cf2a5427c3380fff856447e) (Version v0.19.0) (Android: GPU-OpenCL) (iOS: )
- [ ] [executorch]() (Version ) (Android: CPU-XNNPack/NPU-QNN-Htp/NPU-MTK) (iOS: )
 
*Currently Supported Metrics*:
- [x] prefill/decode speed (tok/s)
- [x] capacity consumption (uAh/tok)
- [x] energy consumption (mJ/tok)
- [ ] temperature (peak+average) (measured but not reported yet)
- [ ] external file size (model disk occupation)
- [ ] runtime memory size (RSS size)
- [ ] perplexity
- [ ] accuracy

*Currently Supported Models*:
- [x] Qwen Series (text-generation)
- [x] Llama Series (text-generation)
- [x] Gemma Series
- [x] Phi-2
- [ ] Phi-3-mini

*Currently Supported Test Mode*:
- [x] json/jsonl/parquet file stored dataset subset testing (subset because of high time/energy cost for large dataset testing on phone)
- [x] designated/fixed length input test
- [ ] user input string (interactive testing mode)

The anroid demo is located in `./android2` directory.

### 1. Git clone
You needs all the submoudles. add `--recursive` for your `git clone`.
```bash
git clone --recursive https://github.com/huangzhengxiang/LLM-Profiler.git
```

### 2. Quick Start

#### 2.1 model preparation
1. Convert model to mnn/gguf/tflite... format. (for model converting methods, please refer to each format's repository)

2. Push your model to `/data/local/tmp/llm/model` directory.
```bash
# example
adb shell mkdir /data/local/tmp/llm
adb shell mkdir /data/local/tmp/llm/model/
adb push model/qwen2_5-1_5b-int4-mnn/ /data/local/tmp/llm/model/
```

#### 2.2 release version installation
The release version is ready to use at `android2\app\release\app-release.apk` or in GitHub Release. Install it on your cell phone.


#### 2.3 APP use
After model and apk uploading. Install the apk and use it. Click `加载模型` first and then record your voice after you see it finished `模型加载完成`.


### 3. Build from source
Several LLM inference engines are contained in this app: MNN-Habst (Ours), llama.cpp,  

MNN-Habst is up-to-date with MNN master branch at commit: 5bd7ffc22a54f6436e387ec2a5cfde7e207feba1 (Version 3.0.4).
Then, heterogeneity-aware backend selection and tuning (Habst algorithm) is added to the repo of [MNN-Habst](https://github.com/Embedded-AI-Systems/MNN-Habst.git) which is the submodule.

llama.cpp is added at commit: 73e2ed3ce3492d3ed70193dd09ae8aa44779651d (Version b4735), being the submodule.

Then, open project in `Android Studio` and build.

- gradle-8.9
- JDK: corretto-18
- ndk version: 27.0.12077973

### 4. Multi-Threading Options for MNN-Habst
Internal: `Power_Normal`, `Power_High`, `Power_MemoryBound`, `Power_SelectCore`. ("normal", "high", "memory", "select")
External Additional Option: "exhaustive", (requires an additional list of selective core group size. e.g., 8Gen3 [1,3,2,2], big->small, and the results are stored in a local file.), "tune_prefill" (tune prefill).


### 5. Datasets Supports
1. multi-turn conversation dataset (ShareGPT-en): https://huggingface.co/datasets/shareAI/ShareGPT-Chinese-English-90k (./sharegpt_jsonl/common_en_70k.jsonl) (input prefill controlled decode)
2. role play (RoleLLM): https://huggingface.co/datasets/ZenMoore/RoleBench (./rolebench-eng/role-generalization/role_specific/test.jsonl) (input prefill, controlled decode)
3. math problem  QA (math_qa): https://huggingface.co/datasets/allenai/math_qa (input prefill, free talk decode)
4. Open Domain QA (truthful_qa): https://huggingface.co/datasets/truthfulqa/truthful_qa (input prefill, free talk decode)
