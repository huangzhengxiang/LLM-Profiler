//
// Created by hzx on 2025/2/21.
//

#include "llm_wrapper.h"

LLMWrapper* LLMWrapper::createWrapper(const char* model_dir,
                                      std::string backend_name,
                                      std::string tmp_path,
                                      std::string engine_name,
                                      std::string prefill_thread_num,
                                      std::string decode_thread_num,
                                      std::string prefill_power_mode,
                                      std::string decode_power_mode,
                                      std::string decode_cores,
                                      std::string decode_tune_times) {
    if (engine_name=="MNN") {
        return new MNNWrapper(model_dir,
                              backend_name,
                              tmp_path,
                              prefill_thread_num,
                              decode_thread_num,
                              prefill_power_mode,
                              decode_power_mode,
                              decode_cores,
                              decode_tune_times);
    }
    if (engine_name=="llama.cpp") {
        return new llamacppWrapper(model_dir,
                                   backend_name,
                                   tmp_path,
                                   prefill_thread_num,
                                   decode_thread_num,
                                   prefill_power_mode,
                                   decode_power_mode,
                                   decode_cores,
                                   decode_tune_times);
    }
    if (engine_name=="mllm") {
        return new mllmWrapper(model_dir,
                               backend_name,
                               tmp_path,
                               prefill_thread_num,
                               decode_thread_num,
                               prefill_power_mode,
                               decode_power_mode,
                               decode_cores,
                               decode_tune_times);
    }
}