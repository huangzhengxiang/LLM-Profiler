import os
import sys
import json
import subprocess
from typing import Union, Tuple, List
import time
import argparse
import numpy as np

effective_factor = 80

def get_pid() -> Union[int, None]:
    cmd = "pymobiledevice3 developer dvt sysmon process single -a name=LLM-Profiler"
    info = subprocess.check_output(cmd.split(" "))
    info = json.decoder.JSONDecoder().decode(info.decode())
    for p in info:
        if p["name"]=="LLM-Profiler":
            return p["pid"]
    return None

def get_cpu() -> Union[float, None]:
    cmd = "pymobiledevice3 developer dvt sysmon process single -a name=LLM-Profiler"
    info = subprocess.check_output(cmd.split(" "))
    info = json.decoder.JSONDecoder().decode(info.decode())
    for p in info:
        if p["name"]=="LLM-Profiler":
            return p["cpuUsage"]
    return None

def process_logs(pid: int, log: str):
    matches = [line.split(" INFO ")[-1].strip() for line in log.split("\n") if line!=""]
    info = []
    for item in matches:
        try:
            info.append(json.decoder.JSONDecoder().decode(
                item.replace("\'", "\"").replace("{}{}:".format("{", pid), "{}\"{}\":".format("{", pid))
            )[f"{pid}"])
        except json.decoder.JSONDecodeError:
            pass
    return info
    

def estimate_energy(data: List[dict]) -> int:
    print(data)
    energy_list = [item["energy.cpu.cost"] + item["energy.gpu.cost"] for item in data]
    effective_energy = np.percentile(energy_list, effective_factor) # larger than 80% percentile to be valid
    print("top 80 percentage: {:.2f}".format(effective_energy))
    return np.average([e for e in energy_list if e >= effective_energy])
    
def estimate_cpu(data: List[float]) -> float:
    effective_cpu = np.percentile(data, effective_factor)
    print("top 80 cpuUsage: {:.2f}".format(effective_cpu))
    return np.average([e for e in data if e >= effective_cpu])

def monitor(pid: int) -> Tuple[int, int]:
    # return (cpu+gpu energy cost, cpu utilization rate)
    energy, cpu_rate = 0, 0
    print("start energy monitor")
    cmd = "pymobiledevice3 developer dvt energy {:d}".format(pid)
    process = subprocess.Popen(cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    cpu_rate = []
    try:
        # Sleep until a KeyboardInterrupt occurs
        while True:
            time.sleep(1)
            cpu_rate.append(get_cpu())
    except KeyboardInterrupt:
        # If a KeyboardInterrupt occurs, capture the partial output
        _, partial_error = process.communicate()
        info = process_logs(pid, partial_error)
        energy = estimate_energy(info)
        rate = estimate_cpu(cpu_rate)
    finally:
        # Ensure the process is terminated
        process.terminate()
        process.wait()
        return energy, rate
    
        
if __name__=="__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", action="store_true", default=False)
    parser.add_argument("--pid", type=int, default=0)
    args = parser.parse_args()
    
    # get LLm-Profiler pid first
    if args.pid <= 1:
        pid = get_pid()
        if pid is None:
            print("app LLM-Profiler not found!")
            exit(-1)
    else:
        pid = args.pid
    print("LLM-Profiler pid: {:d}".format(pid))
    
    # start monitor process
    if args.m:
        energy, rate = monitor(pid)
        print("energy consumption: {:.2f}, cpu utilization rate: {:.2f}".format(energy, rate))