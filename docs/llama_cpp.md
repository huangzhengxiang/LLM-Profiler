### llama.cpp Model Converter

First, convert h5 PyTorch model to gguf fp16 format.
See ./llama.cpp/convert_hf_to_gguf.py

Then, convert gguf fp16 format to Q4_1/Q4_0 (asymmetric int4/symmetric int4) to compare.
See ./llama.cpp/examples/quantize/README.md