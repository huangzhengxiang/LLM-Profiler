### Dataset download and samples
We pick 4 datasets from 4 mainstream task domains:
1. multi-turn conversation dataset (ShareGPT-en): https://huggingface.co/datasets/shareAI/ShareGPT-Chinese-English-90k (sharegpt_jsonl/common_en_70k.jsonl) (input prefill controlled decode)
2. role play (RoleLLM): https://huggingface.co/datasets/ZenMoore/RoleBench (rolebench-eng/role-generalization/role_specific/test.jsonl) (input prefill, controlled decode)
3. math problem  QA (math_qa): https://huggingface.co/datasets/allenai/math_qa, https://math-qa.github.io/ (test.json) (input prefill, free talk decode)
4. Open Domain QA (truthful_qa): https://huggingface.co/datasets/truthfulqa/truthful_qa (generation/validation-00000-of-00001.parquet) (input prefill, free talk decode)

Downloading can be done as follows:
```bash
cd data
git lfs install
git clone https://huggingface.co/datasets/shareAI/ShareGPT-Chinese-English-90k ShareGPT
git clone https://huggingface.co/datasets/ZenMoore/RoleBench RoleBench
wget https://math-qa.github.io/data/MathQA.zip && unzip MathQA.zip -d math_qa && rm MathQA.zip && rm -r math_qa/__MACOSX
git clone https://huggingface.co/datasets/truthfulqa/truthful_qa truthful_qa
```

The sampling and profiling (the dataset statistics) can be done by executing the python script:
```bash
python token-fetcher.py
```