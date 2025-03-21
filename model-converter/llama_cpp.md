### llama.cpp Model Converter

First, convert h5 PyTorch model to gguf fp16 format.
See `./llama.cpp/convert_hf_to_gguf.py`

Then, convert gguf fp16 format to Q4_1/Q4_0 (asymmetric int4/symmetric int4) to compare.
See `./llama.cpp/examples/quantize/README.md`

### example export workflow
1. convert h5 PyTorch model to gguf fp16 format:
    First, ensure you have a conda environment that has `transformers>=4.44`.
    Then, call `./llama.cpp/convert_hf_to_gguf.py` to convert hf model to fp16 gguf model.
 ```bash
 cd llama.cpp
 python convert_hf_to_gguf.py <path-to-hf-model> --outfile <path-to-exported-gguf-file>  --outtype f16
 ```

1. quantize gguf fp16 format to Q4_1/Q4_0 (asymmetric int4/symmetric int4) to compare:
    First, you need to compile llama.cpp quantizer on your host PC:
 ```bash
 cd llama.cpp
 mkdir -p build/pc/
 cd build/pc
 cmake ../../ && make -j16
 ```
    Then, you call `./llama.cpp/build/pc/bin/llama-quantize` to quantize the fp16 model
 ```bash
 ./llama-quantize <path-to-f16-model-gguf> <path-to-Q4_1-model-gguf> Q4_1
 ./llama-quantize ./build/model/model-f16.gguf ./models/mymodel/model-Q4_1.gguf Q4_1
 ```