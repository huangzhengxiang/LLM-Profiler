## MNN Model converter
Use llmexport tool to convert huggingface h5 format to mnn format. 
Refer to ./MNN/transformers/llm/export/README.md

### example export workflow
Create a environment called `llm`, `python=3.10` for best. Ensure the version of MNN to be 3.0.4  (as we checkout at that point).

```bash
cd MNN/transformers/llm/export/
conda activate llm
pip install -r requirements.txt
pip install MNN==3.0.4
```

Then, download models from huggingface or modelscope. Export the h5 format to mnn format (4-bit channel-wise asymmetric, or Q4_1):

```bash
python llmexport.py --path <path-to-your-hf-model> --dst_path <path-to-exported-mnn-path> --export mnn --quant_bit 4 --quant_block 0
```

Then, `adb push` the converted model to `/data/local/tmp/llm/model` on the cell phone.