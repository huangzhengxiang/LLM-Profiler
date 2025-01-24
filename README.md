## Android Demo

Android demo of ***Qwen2-Audio-7B-Instruct*** local deployment. https://huggingface.co/Qwen/Qwen2-Audio-7B-Instruct.

The DNN inference engine on the phone is powered by [MNN](https://github.com/alibaba/MNN). MNN supports for Qwen2-Audio-7B-Instruct is newly ready this week. Current demo shows on-device inference for MNN Qwen2-Audio-7B-Instruct.

The anroid demo is located in `android2`.

### 1. Git clone
You needs all the submoudles. add `--recursive` for your `git clone`.
```bash
git clone --recursive https://github.com/huangzhengxiang/Look-Once-to-Hear-Android-Demo.git
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
Before build, first modify MNN a little by running the following commands.

MNN-Habst is up-to-date with MNN master branch at commit: 5bd7ffc22a54f6436e387ec2a5cfde7e207feba1 (Version 3.0.4).
Then, heterogeneity-aware backend selection and tuning is added to the repo of [MNN-Habst](https://github.com/Embedded-AI-Systems/MNN-Habst.git) which is the submodule.

Then, open project in `Android Studio` and build.

### 4. ThreadPool Design

Prefill core number will be greater than decode, so tuning the onResize will allocate big enough buffer, while onExecute may use dynamic thread number.