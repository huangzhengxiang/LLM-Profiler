### Android Demo

Android demo of ***Qwen2-Audio-7B-Instruct*** local deployment. https://huggingface.co/Qwen/Qwen2-Audio-7B-Instruct.

The DNN inference engine on the phone is powered by [MNN](https://github.com/alibaba/MNN). MNN supports for Qwen2-Audio-7B-Instruct is newly ready this week. Current demo shows on-device inference for MNN Qwen2-Audio-7B-Instruct.

The anroid demo is located in `android2`.

#### Git clone
You needs all the submoudles. add `--recursive` for your `git clone`.
```bash
git clone --recursive https://github.com/huangzhengxiang/Look-Once-to-Hear-Android-Demo.git
```

#### model preparation
```bash
adb shell mkdir /data/local/tmp/llm
adb shell mkdir /data/local/tmp/llm/model/
adb push model/qwen2-audio-7b-mnn /data/local/tmp/llm/model/
```