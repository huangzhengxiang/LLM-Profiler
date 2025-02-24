from transformers import AutoTokenizer
import json
import os
import random
import pandas as pd
from pathlib import Path
from tqdm import tqdm
import argparse

data_stats_dict = {}
sample_stats_dict = {}

def read_config(config_path="data_config.json") -> dict:
    with open(config_path, "rt") as config_file:
        config = json.load(config_file)
        print("config:\n", config)
    return config

def resolve_path(path: str, mode: str, is_data=True) -> str:
    if not is_data:
        os.makedirs(path, exist_ok=True)
        return os.path.join(path, f"{mode}.json")
    if mode == 'ShareGPT':
        return os.path.join(path, "sharegpt_jsonl", "common_en_70k.jsonl")
    elif mode == 'RoleBench':
        return os.path.join(path, "rolebench-eng", "role-generalization", "role_specific", "test.jsonl")
    elif mode == 'math_qa':
        return os.path.join(path, "test.json")
    elif mode == 'truthful_qa':
        return os.path.join(path, "generation", "validation-00000-of-00001.parquet")
    else:
        raise NotImplementedError("Dataset Not Supported!")

def parquet2json(parquet_file: str):
    df = pd.read_parquet(parquet_file) 
    return df.to_dict(orient='records')

def read_file(input_file: str):
    with open(input_file,'rt',encoding='utf-8') as f:
        if (Path(input_file).suffix==".jsonl"):
            json_arr = [json.loads(line.strip()) for line in f.read().split("\n") if line.strip()!=""] 
        elif (Path(input_file).suffix==".json"):
            json_arr = json.load(f)
        elif (Path(input_file).suffix==".parquet"):
            json_arr = parquet2json(input_file)
        else:
            raise NotImplementedError("File Format Not supported!")
    return json_arr

def process_entry(tokenizer,
                  entry: dict, 
                  user_select: str, 
                  assistant_select=None,
                  cnv_select=None):
    prefill = 0
    decode = 0
    turns = 0
    # construct messages
    messages = [{"role": "system", "content": "You're a helpful assistant."}]
    cnvs = [entry] if cnv_select is None else entry[cnv_select]
    for cnv in cnvs:
        messages.append({"role": "user", "content": cnv[user_select]})
        if assistant_select is not None:
            messages.append({"role": "assistant", 
                             "content": cnv[assistant_select][0] if isinstance(cnv[assistant_select], list) else cnv[assistant_select]})
        turns += 1
    # calculate length
    total_text = tokenizer.apply_chat_template(
        messages,
        tokenize=True,
        add_generation_prompt=True
    )
    prefill_text = tokenizer.apply_chat_template(
        [m for m in messages if m["role"]!="assistant"],
        tokenize=True,
        add_generation_prompt=True
    )
    total = len(total_text)
    prefill = len(prefill_text)
    decode = total - prefill
    return prefill, decode, turns, prefill/turns, decode/turns

# statistics: [entry, prefill/entry, decode/entry, turns, prefill/turn, decode/turn] on average
def prefill_decode_token(tokenizer, input_root, mode, is_data=True):
    entry = 0
    prefill_token = 0
    decode_token = 0
    turns = 0
    prefill_per_turn = 0
    decode_per_turn = 0

    problem_set = []
    input_file = resolve_path(input_root, mode, is_data)
    json_arr = read_file(input_file)
    entry = len(json_arr)

    # tokenize
    if mode == "ShareGPT":
        user_select = "human"
        assistant_select = "assistant"
        cnv_select = "conversation"
    if mode == "RoleBench":
        user_select = "question"
        assistant_select = "generated"
        cnv_select = None
    if mode == "math_qa":
        user_select = "Problem"
        assistant_select = None
        cnv_select = None
    if mode == "truthful_qa":
        user_select = "question"
        assistant_select = None
        cnv_select = None
    for record in tqdm(json_arr):
        p, d, t, pt, dt = process_entry(tokenizer,
                                        record, 
                                        user_select, 
                                        assistant_select, 
                                        cnv_select)
        prefill_token += p
        decode_token += d
        turns += t
        prefill_per_turn += pt
        decode_per_turn += dt
    
    return entry, prefill_token/entry, decode_token/entry, \
        turns/entry, prefill_per_turn/entry, decode_per_turn/entry 

def calc_stats(tokenizer, config: dict, is_data=True) -> dict:
    stats_dict = data_stats_dict if is_data else sample_stats_dict
    for name, path in config["data_paths"].items():
        entry, prefill_per_entry, decode_per_entry, \
            turns, prefill_per_turn, decode_per_turn \
            = prefill_decode_token(tokenizer, 
                                   os.path.join(".", config["data_root" if is_data else "sample_root"], path), 
                                   name, is_data)
        stats_dict[name] = {"entry": entry,
                            "prefill/entry": prefill_per_entry,
                            "turns": turns}
        if decode_per_turn != 0:
            stats_dict[name]["decode/entry"] = decode_per_entry
        if turns > 1:
            stats_dict[name]["prefill/turn"] = prefill_per_turn
            stats_dict[name]["decode/turn"] = decode_per_turn
    return
    

def sample_cnv(input_file, output_file, config: dict):
    json_arr = read_file(input_file)
    idx_list = random.sample(range(0, len(json_arr)), config["sample_num"])
    idx_list.sort()
    samples = [json_arr[idx] for idx in idx_list]
    with open(output_file,'wt',encoding='utf-8') as out:
        json.dump(samples, out)
    return

def sample_subset(config: dict):
    print("sample subset: {} samples per dataset...".format(config["sample_num"]))
    for name, path in tqdm(config["data_paths"].items()):
        input_file = resolve_path(os.path.join(".", config["data_root"], path),
                                  name, True)
        output_file = resolve_path(os.path.join(".", config["sample_root"], path),
                                   name, False)
        sample_cnv(input_file, output_file, config)
    return

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--stats", action="store_true", default=False)
    parser.add_argument("--sample", action="store_true", default=False)
    args = parser.parse_args()
    need_stats = args.stats
    need_sample = args.sample
    # 0. load config and tokenizer
    config = read_config()
    tokenizer = AutoTokenizer.from_pretrained("../model/Qwen2.5-1.5B-Instruct")
    print("Qwen2 Tokenizer Loaded!")
    # 1. calculate data stats
    if need_stats:
        calc_stats(tokenizer, config, True)
        print(data_stats_dict) 
        with open(os.path.join(".", config["data_root"], "data_stats.json"), "wt") as f:
            json.dump(data_stats_dict, f, indent=4)
    if need_sample:
        # 2. sample from data
        sample_subset(config)
        # 3. calculate sample stats
        calc_stats(tokenizer, config, False)
        print(sample_stats_dict)
        with open(os.path.join(".", config["sample_root"], "sample_stats.json"), "wt") as f:
            json.dump(sample_stats_dict, f, indent=4)