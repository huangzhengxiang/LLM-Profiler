## executorch

executorch/examples/models/llama/README.md
executorch/examples/models/llama/non_cpu_backends.md
executorch/examples/demo-apps/android/LlamaDemo/docs/delegates/xnnpack_README.md
executorch/examples/demo-apps/android/LlamaDemo/docs/delegates/mediatek_README.md
executorch/examples/demo-apps/android/LlamaDemo/docs/delegates/qualcomm_README.md

### 1. Install executorch
`executorch` for model conversion can only be installed on computer with Nvidia GPU. For buck2 file watcher issue: https://stackoverflow.com/questions/53930305/nodemon-error-system-limit-for-number-of-file-watchers-reached/
```bash
conda create -n executorch python=3.10
./install_executorch.sh --pybind xnnpack
```

### 2. Get models
You can request and download model weights for Llama through Meta official [website](https://llama.meta.com/).

Download `Llama-3.2-1B-Instruct-SpinQuant_INT4_EO8` and `Llama-3.2-3B-Instruct-SpinQuant_INT4_EO8` and optionally `Llama-3.2-1B-Instruct` and `Llama-3.2-3B-Instruct` with their instructions.

```bash
mkdir -p executorch/.model
mv ~/.llama/checkpoints/Llama3.2-1B-Instruct-int4-spinquant-eo8 executorch/.model
mv ~/.llama/checkpoints/Llama3.2-3B-Instruct-int4-spinquant-eo8 executorch/.model
mv ~/.llama/checkpoints/Llama3.2-1B-Instruct executorch/.model
mv ~/.llama/checkpoints/Llama3.2-3B-Instruct executorch/.model
```

### 3. Convert model with our scripts
Convert downloaded pre-quantized SpinQuant model to pte file:
```bash
cd executorch
python -m examples.models.llama.export_llama --model "llama3_2" --checkpoint .model/Llama3.2-1B-Instruct-int4-spinquant-eo8/consolidated.00.pth --params .model/Llama3.2-1B-Instruct-int4-spinquant-eo8/params.json -kv --use_sdpa_with_kv_cache -X -d fp32 --xnnpack-extended-ops --preq_mode 8da4w_output_8da8w --preq_group_size 32 --max_seq_length 2048 --max_context_length 2048 --preq_embedding_quantize 8,0 --use_spin_quant native --metadata '{"get_bos_id":128000, "get_eos_ids":[128009, 128001]}' --output-dir .model/ --output_name "llama3_2-1b-spinquant.pte"
python -m examples.models.llama.export_llama --model "llama3_2" --checkpoint .model/Llama3.2-3B-Instruct-int4-spinquant-eo8/consolidated.00.pth --params .model/Llama3.2-3B-Instruct-int4-spinquant-eo8/params.json -kv --use_sdpa_with_kv_cache -X -d fp32 --xnnpack-extended-ops --preq_mode 8da4w_output_8da8w --preq_group_size 32 --max_seq_length 2048 --max_context_length 2048 --preq_embedding_quantize 8,0 --use_spin_quant native --metadata '{"get_bos_id":128000, "get_eos_ids":[128009, 128001]}' --output-dir .model/ --output_name "llama3_2-3b-spinquant.pte"
```

Convert original floating point model to (per-channel 4-bit quantized) xnnpack:
```bash
cd executorch
python -m examples.models.llama.export_llama --model "llama3_2" --checkpoint .model/Llama3.2-1B-Instruct/consolidated.00.pth --params .model/Llama3.2-1B-Instruct/params.json -kv --use_sdpa_with_kv_cache -X -d fp32 --xnnpack-extended-ops --pt2e_quantize xnnpack_dynamic_qc4 --embedding-quantize 8,0 --max_seq_length 2048 --max_context_length 2048 --metadata '{"get_bos_id":128000, "get_eos_ids":[128009, 128001]}' --output-dir .model/ --output_name "llama3_2-1b-Q4.pte"
python -m examples.models.llama.export_llama --model "llama3_2" --checkpoint .model/Llama3.2-3B-Instruct/consolidated.00.pth --params .model/Llama3.2-3B-Instruct/params.json -kv --use_sdpa_with_kv_cache -X -d fp32 --xnnpack-extended-ops --pt2e_quantize xnnpack_dynamic_qc4 --embedding-quantize 8,0 --max_seq_length 2048 --max_context_length 2048 --metadata '{"get_bos_id":128000, "get_eos_ids":[128009, 128001]}' --output-dir .model/ --output_name "llama3_2-3b-Q4.pte"
``` 

Copy `tokenizer.model` (rename to `tokenizer.bin`) and `.pte`  (rename to `model.pte`) to the same folder. Organize the .model directory as:

```
executorch/.model/
    |- llama3_2-1b-instruct-q4-torch
        |- tokenizer.bin
        |- model.pte
    |- llama3_2-3b-instruct-q4-torch
        |- tokenizer.bin
        |- model.pte
    |- ...
```


upload to your phone:

```bash
adb shell mkdir -p /data/local/tmp/llm/
adb push .model/llama3_2-1b-instruct-q4-torch/ /data/local/tmp/llm/model
```