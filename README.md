### Android Demo

Android demo of ***Look Once to Hear: Target Speech Hearing with Noisy Examples. Look Once to Hear***. The github link to the original repository is https://github.com/vb000/LookOnceToHear

The DNN inference engine on the phone is supported by [MNN](https://github.com/alibaba/MNN). MNN supports for Qwen2-Audio-7B will be coming soon.

The phone demo is cloned from . It can record the sound with microphone.

#### Git clone
You needs all the submoudles. add `--recursive` for your `git clone`.
```bash
git clone --recursive https://github.com/huangzhengxiang/Look-Once-to-Hear-Android-Demo.git
```

#### download model and convert to MNN format


#### Git Submodule
```bash
git submodule add https://github.com/alibaba/MNN.git MNN
```