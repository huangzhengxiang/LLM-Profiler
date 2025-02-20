## LLM-Profiler

Android demo of ***Qwen2-Audio-7B-Instruct*** local deployment. https://huggingface.co/Qwen/Qwen2-Audio-7B-Instruct.

The DNN inference engine on the phone is powered by [MNN](https://github.com/alibaba/MNN). MNN supports for Qwen2-Audio-7B-Instruct is newly ready this week. Current demo shows on-device inference for MNN Qwen2-Audio-7B-Instruct.

The anroid demo is located in `android2`.

### 1. Git clone
You needs all the submoudles. add `--recursive` for your `git clone`.
```bash
git clone --recursive https://github.com/huangzhengxiang/LLM-Profiler.git
```

### 2. Quick Start

#### model preparation
The Qwen model (approx 6.2GB) shall first be downloaded from jbox and uploaded to phone.

```bash
adb shell mkdir /data/local/tmp/llm
adb shell mkdir /data/local/tmp/llm/model/
adb push model/qwen2-audio-7b-mnn/ /data/local/tmp/llm/model/
```

#### release version
The release version is ready to use at `android2\app\release\app-release.apk`.

#### use
After model and apk uploading. Install the apk and use it. Click `加载模型` first and then record your voice after you see it finished `模型加载完成`.


### 3. build from source
Several LLM inference engines are contained in this app: MNN-Habst (Ours), llama.cpp,  

MNN-Habst is up-to-date with MNN master branch at commit: 5bd7ffc22a54f6436e387ec2a5cfde7e207feba1 (Version 3.0.4).
Then, heterogeneity-aware backend selection and tuning (Habst algorithm) is added to the repo of [MNN-Habst](https://github.com/Embedded-AI-Systems/MNN-Habst.git) which is the submodule.

llama.cpp is added at commit: 73e2ed3ce3492d3ed70193dd09ae8aa44779651d (Version b4735), being the submodule.

Then, open project in `Android Studio` and build.


### 4. Multi-Threading Options for MNN-Habst
Internal: `Power_Normal`, `Power_High`, `Power_MemoryBound`, `Power_SelectCore`. ("normal", "high", "memory", "select")
External Additional Option: "exhaustive", (requires an additional list of selective core group size. e.g., 8Gen3 [1,3,2,2], big->small, and the results are stored in a local file.), "tune_prefill" (tune prefill).


### 5. Datasets
1. multi-turn conversation dataset (ShareGPT-en): https://huggingface.co/datasets/shareAI/ShareGPT-Chinese-English-90k (./sharegpt_jsonl/common_en_70k.jsonl) (input prefill controlled decode)
2. role play (RoleLLM): https://huggingface.co/datasets/ZenMoore/RoleBench (./rolebench-eng/role-generalization/role_specific/test.jsonl) (input prefill, controlled decode)
3. math problem  QA (math_qa): https://huggingface.co/datasets/allenai/math_qa (input prefill, free talk decode)
4. Open Domain QA (truthful_qa): https://huggingface.co/datasets/truthfulqa/truthful_qa (input prefill, free talk decode)