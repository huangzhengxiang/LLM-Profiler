from transformers import AutoTokenizer
import json
import os
import random
import pandas as pd
from pathlib import Path

# Load the tokenizer (which includes the vocabulary file)
tokenizer = AutoTokenizer.from_pretrained("../Meta-Llama/Llama-3.2-1B-Instruct")

# # Input text
# text = ""

# # Tokenize the text
# tokens = tokenizer.tokenize(text)
# print("Tokens:", tokens)

# # Convert tokens to token IDs using the vocabulary
# token_ids = tokenizer.convert_tokens_to_ids(tokens)
# print("Token IDs:", token_ids)

# # Decode token IDs back to text
# decoded_text = tokenizer.decode(token_ids)
# print("Decoded Text:", decoded_text)

def parquet2json(parquet_file: str):
    df = pd.read_parquet(parquet_file) 
    json_file = os.path.dirname(parquet_file) + Path(os.path.basename(parquet_file)).stem
    df.to_json(json_file, orient='records') 

def prefill_decode_token(input_file,mode):
    prefill_token = 0
    decode_token = 0
    tokenizer = AutoTokenizer.from_pretrained("../Meta-Llama/Llama-3.2-1B-Instruct")
    with open(input_file,'r',encoding='utf-8') as f:
        if mode == 'math_qa':
            json_arr = json.load(f)
            for problem_set in json_arr:
                problem = problem_set['Problem']
                prefill_token += len(tokenizer.tokenize(problem))
        elif mode == 'truthful_qa':
            json_arr = json.load(f)
            for problem_set in json_arr:
                problem = problem_set['question']
                prefill_token += len(tokenizer.tokenize(problem))
        else:
            for line in f:
                data = json.loads(line.strip())

                if mode == 'ShareGPT':
                    conversations = data['conversation']
                    for conv in conversations:
                        for key,val in conv.items():
                            if key == 'human':
                                prefill_token += len(tokenizer.tokenize(val))
                            elif key == 'assistant':
                                decode_token += len(tokenizer.tokenize(val))

                elif mode == 'RoleBench':
                    question  = data['question']
                    generated = data['generated'][0]
                    prefill_token += len(tokenizer.tokenize(question))
                    decode_token += len(tokenizer.tokenize(generated))

            
                


    return prefill_token,decode_token

def lines(input_file,mode):
    size = 0
    if mode == 'math_qa' or mode == 'truthful_qa':
        with open(input_file,'r',encoding='utf-8') as f:
            json_arr = json.load(f)
            return len(json_arr)
    else:
        with open(input_file,'r',encoding='utf-8') as f:
            for line in f:
                size += 1
    return size

def sample_token(input_file,output_file,mode,n=100):
    size = lines(input_file,mode)
    idx_list = random.sample(range(0,size),n)
    idx_list.sort()
    idx = 0
    with open(input_file,'r',encoding='utf-8') as in_file, open(output_file,'w',encoding='utf-8') as out_file:
        if mode == 'math_qa' or mode == 'truthful_qa':
            json_arr = json.load(in_file)
            for data_set in json_arr:
                if idx in idx_list:
                    out_file.write(json.dumps(data_set) + '\n')
                idx += 1
        else:

            for line in in_file:
                data = json.loads(line.strip())

                if idx in idx_list:
                    out_file.write(json.dumps(data) + '\n')
                idx += 1
    return size

if __name__ == "__main__":
    # input_file = '../Datasets/ShareGPT-Chinese-English-90k/sharegpt_jsonl/common_en_70k.jsonl'
    # output_file = 'common_en_70k_samp100.jsonl'
    # input_file = '../Datasets/RoleBench/rolebench-eng/role-generalization/role_specific/test.jsonl'
    # output_file = 'rolebench_test_samp100.jsonl'
    # input_file = '../Datasets/math_qa/problems/test.json'
    # output_file = 'math_qa_test_samp100.jsonl'
    input_file = 'validation-00000-of-00001.jsonl'
    output_file = 'validation-00000-of-00001_samp100.jsonl'
    # size = sample_token(input_file,output_file,'truthful_qa')
    # print("total size : {}".format(size))
    prefill_token,decode_token = prefill_decode_token(input_file,'truthful_qa')
    print("prefill token: {}, decode token: {}".format(prefill_token,decode_token))
    # for f in files:
    #     type = os.path.splitext(f)[-1]
    #     name = os.path.splitext(f)[0]
    #     if type == '.jsonl':
    #         prefill, decode = prefill_decode_token(path+f)
    #         json_files[f] = [prefill,decode]
    # for key in json_files.keys():
    #     print("{} : {}".format(key,json_files[key]))


