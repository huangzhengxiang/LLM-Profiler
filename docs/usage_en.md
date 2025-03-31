# LLM Profiler Operation Guide

## Interface Introduction

**Select Engine**: Choose the LLM engine to test.  
**Select Model**: Select the model to test. Ensure the model matches the engine (e.g., `.mnn` models for MNN engine).

**Engine-Model Suffix Mapping**:
- **MNN**: `.mnn`
- **Llama**: `.gguf`
- **MLLM**: `.mllm` (supports only Llama/Qwen models)
- **Torch**: `.torch`
- **MediaPipe**: `.bin`

**Select Backend**: Choose the runtime backend. Unsupported backends will trigger a warning and revert to CPU.

**Prefill & Decode Threads**:
- Set thread counts for prefill/decode phases.
- For MNN GPU mode: typically `68` or `132` (internal codes).

### MNN-Exclusive Settings
*(Ignored by other engines)*
- **Prefill Power**:
    - `default`/`normal`/`high`: Use MNN defaults.
    - `tune prefill`: Enable custom tuning.
- **Decode Power**:
    - `default`/`normal`/`high`: Use MNN defaults.
    - `memory`/`exhaustive`: Enable custom tuning.
- Leave the four fields below blank for default MNN settings.

**Test Method**:
- `Fixed Length`: Manually set prefill/decode lengths.
- `Dataset`: Choose from four predefined datasets.

---

## Usage
1. Fill required parameters → Click **Load Model**.
2. After loading, click **Start Test** → Results show in the green box.

---

## Important Notes
1. **No parameter changes after loading** → Restart the app to modify settings.
2. **Engine-suffix relationships** must match strictly.
3. **Unplug charging cables** during tests for accurate power measurements.
4. **Safety Limits**:
    - Pause if battery <50% or temperature >40°C → Resume with **Continue** after cooling/charging.
    - Dataset mode auto-retries every 5s; Fixed-length requires manual retries.
5. **Record anomalies** (e.g., values ≤0) in results.

---

## Test Parameters
### Dataset Test
- Run all four predefined datasets.

### Fixed-Length Test
| Prefill Lengths | Decode Lengths | MediaPipe Exclusion |  
|-----------------|----------------|---------------------|  
| 64, 256, 1024   | 128, 256, 512  | Skip 1024 prefill   |  

**Total Combinations**:
- 9 tests (6 for MediaPipe).

### Ours Settings
```plaintext
engine: MNN
backend: CPU
prefill thread: 0
decode thread: 0
prefill power: tune_prefill 
decode power: memory 
prefill cores:
decode cores: 
tune times: 50 
decode tol: 8 
```

The more the `tune times` is, the higher the tuning accuracy is. 