## LLM profiler操作指南

### 界面介绍

select engine: 选择想要测试的llm引擎

select model：选则想要测试的模型。注意模型需要和引擎匹配（例：在使用MNN作为engine的时候，需要选择后缀为MNN的模型）

对应关系:MNN对应mnn后缀，llama对应gguf，mllm对应mllm，torch对应torch，mediapipe对应bin

select backend: 选择运行的模式。但并不是所有的engine都支持所有的backend。如果遇到了不支持的情况在运行时会有warning，并自动将backend切换回CPU

prefill & decode thread: 设置想用来prefill和decode的线程数，对于所有的engine都适用。对于MNN在GPU模式下，一般使用68或132（表示内部代码）

下面的六个功能为MNN引擎的专属，其他引擎填入不会有效果。

如果prefill power和decode power设为default，normal或high，下面的四个框选择不填将会使用MNN的默认设置。

如果将prefill power选择tune prefill，decode power选择memory或者exhaustive，或填入下面则会使用我们自己的方法。

test的方法可以选择fixed length和dataset，dataset有四种可用的方法。

### 用法

每次使用需要选择并填入所有你需要的参数，然后选择加载模型。当模型加载完毕后点击start test即可，test完成后会在下方的绿色框中显示各项实验数据。

### 注意

1.在点击了加载模型后就不能变更除了test以外的所有选项，如果要变更需要重启整个app。

2.engine和模型后缀的对应关系:MNN对应mnn，llama对应gguf，mllm只能测mllm后缀的llama和qwen，torch对应torch，mediapipe对应bin

3.在运行实验的时候为了耗电数据的准确，请不要插着充电线运行

4.为了保护设备同时保证准确性，实验将在电量<50%或者温度>40°C时停止运行。如果需要继续运行请等待充电或降温后按continue按钮继续实验。如果点击start直接提示温度异常，dataset模式会自动每五秒钟尝试重新开始，但是fixed length需要过一会后手动点击尝试重新实验。

5.若在下方的绿色显示框中，出现异常数值（如小于等于0的数值）需要记录。

### 测试时的参数列表

test部分，需要dataset test四个dataset全部进行测试，而fixed length test需要prefill length为64，256，1024;decode length为128，256，512共九种情况分别测试（mediapipe最多支持1024token,所以不需要prefill length = 1024 的组）

MNN的default和其他engine都不需要填入任何参数，只需要选择engine/model/backend和test中的所有选项进行测试即可。

MNN 非default 参数设置为0 0 tune_prefill memory 不填 不填 10 6 