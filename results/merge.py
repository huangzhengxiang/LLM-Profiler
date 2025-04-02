import pandas as pd
import os
from typing import Dict, Tuple

def merge_model(root, format="csv") -> pd.DataFrame:
    dataframes = []
    for file in os.listdir(os.path.join(root, "output")):
        if file.endswith(f".{format}"):
            path = os.path.join(root, "output", file)
            df = pd.read_csv(path)
            dataframes.append(df)
    merged_df = pd.concat(dataframes, ignore_index=True)
    merged_df = merged_df.sort_values(by=["prefill_len", "decode_len"], ignore_index=True)
    merged_df.to_csv(os.path.join(root, "merged.csv"), float_format=lambda f: "{:.4f}".format(f), index=False)
    return merged_df

def merge_device(root) -> Dict[str, pd.DataFrame]:
    device_dict = {}
    for model in os.listdir(root):
        if not os.path.isdir(os.path.join(root, model)):
            continue
        device_dict[model] = merge_model(os.path.join(root, model))
    return device_dict
     
def merge_all(root) -> Dict[str, Dict[str, pd.DataFrame]]:
    dict_all = {}
    for device in os.listdir(root):
        if os.path.isdir(os.path.join(root, device)):
            dict_all[device] = merge_device(os.path.join(root, device))   
    return dict_all

def parse_name(name: str) -> Tuple[str, str]:
    engine = name.split("-")[0]
    model = name.replace(engine + "-", "")
    return engine, model

def compose_name(engine, model) -> str:
    return "-".join([engine, model])  

def compare_model(ours, others) -> pd.DataFrame:
    eps = 1e-6
    result = ours.copy()
    columns_to_compare = [col for col in ours.columns if col not in ["prefill_len", "decode_len"]]
    result[columns_to_compare] = (ours[columns_to_compare] - others[columns_to_compare]) / (others[columns_to_compare] + eps)
    return result

def compare_device(root: str, data: Dict[str, pd.DataFrame]) -> Dict[str, pd.DataFrame]:
    ours_model = []
    comparison = {}
    for key in data:
        engine, model = parse_name(key)
        if engine == "ours":
            ours_model.append(model)
    for key in data:
        engine, model = parse_name(key)
        if (engine != "ours") and (model in ours_model):
            compare_name = compose_name("-".join(["ours", engine]), model)
            comparison[compare_name] \
                = compare_model(data[compose_name("ours", model)], data[compose_name(engine, model)])
            comparison[compare_name].to_csv(os.path.join(root, compare_name + ".csv"), index=False)
    return comparison

def compare_all(root, data: Dict[str, Dict[str, pd.DataFrame]]) -> Dict[str, Dict[str, pd.DataFrame]]:
    comparison = {}
    for device, dd in data.items():
        comparison[device] = compare_device(os.path.join(root, device), dd)
    return comparison

def average_device(root, device, data: Dict[str, Dict[str, pd.DataFrame]]):
    results = []
    for key, value in data.items():
        columns_to_compare = []
        for col in value.columns:
            if (col not in ["prefill_len", "decode_len"]) \
                and (not col.endswith("_turn")) \
                and (not col.endswith("_temp")) \
                and (not col.endswith("_percentage_turn")):
                columns_to_compare.append(col)
        results.append(pd.DataFrame(value[columns_to_compare].mean(axis=0),).T)
        results[-1].insert(0, "test", key)
        results[-1].insert(1, "device", device)
    merged_df = pd.concat(results, ignore_index=True)
    return merged_df

def average_all(root, data: Dict[str, Dict[str, pd.DataFrame]]):
    results = []
    for device, dd in data.items():
        results.append(average_device(root, device, dd))
    merged_df = pd.concat(results, ignore_index=True)
    merged_df.to_csv(os.path.join(root, "compare-results.csv"), float_format=lambda f: "{:.4f}".format(f), index=False)
    return merged_df
    

if __name__=="__main__":
    data = merge_all(".")
    comparison = compare_all(".", data)
    results = average_all(".", comparison)
    print(results)
    