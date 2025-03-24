//
// Created by hzx on 2025/2/21.
//

#include "mnn_wrapper.h"

using namespace MNN;
using namespace MNN::Express;
using namespace MNN::Transformer;

#ifdef DYNAMIC_LOAD_SYMBOLS
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
}
#endif

LLMWrapper* LLMWrapper::createMNNWrapper(const char* model_dir,
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
}


MNNWrapper::MNNWrapper(const char* model_dir,
                       std::string backend_name,
                       std::string tmp_path,
                       std::string prefill_thread_num,
                       std::string decode_thread_num,
                       std::string prefill_power_mode,
                       std::string decode_power_mode,
                       std::string decode_cores,
                       std::string decode_tune_times) {
    if (!llm.get()) {
        model_name = std::string(model_dir);
        llm.reset(Llm::createLLM(model_dir));
        llm->set_config("{\"tmp_path\":\"" + tmp_path + "\"}"); // tmp_path (string, need quotation marks)
        if (!backend_name.empty()) { llm->set_config("{\"backend_type\":\"" + backend_name + "\"}"); }
        if (!prefill_thread_num.empty()) { llm->set_config("{\"prefill_thread_num\":" + prefill_thread_num + "}"); } // thread_num (int, no quotation marks)
        if (!decode_thread_num.empty()) { llm->set_config("{\"decode_thread_num\":" + decode_thread_num + "}"); } // thread_num (int, no quotation marks)
        if (!prefill_power_mode.empty()) { llm->set_config("{\"prefill_power\":\"" + prefill_power_mode + "\"}"); } // power (string: need quotation marks)
        if (!decode_power_mode.empty()) { llm->set_config("{\"decode_power\":\"" + decode_power_mode + "\"}"); } // power (string: need quotation marks)
        if (!decode_cores.empty()) { llm->set_config("{\"decode_cores\":\"" + decode_cores + "\"}"); } // power (string: need quotation marks)
        if (!decode_tune_times.empty()) { llm->set_config("{\"decode_tune_times\":" + decode_tune_times + "}"); } // power (string: need quotation marks)
        llm->load();
        trace();
    }
}
bool MNNWrapper::isReady() {
    if (llm.get()) {
        return true;
    }
    return false;
}
std::string getAntiPrompt(std::string model_name) {
    if (model_name.find("qwen")!=std::string::npos) {
        return "<|im_end|>\n";
    } else if (model_name.find("llama")!=std::string::npos) {
        return "";
    } else if (model_name.find("gemma")!=std::string::npos) {
        return "<end_of_turn>\n";
    } else if (model_name.find("phi")!=std::string::npos) {
        return "\n";
    } else {
        return "";
    }
}
void MNNWrapper::trace() {
    llm->trace(true);
    std::vector<int> test_prompt(30, 200);
    llm->switchMode(Llm::Prefill);
    llm->setKVCacheInfo(test_prompt.size(), 0);
    llm->forward(test_prompt);
    // decode tracing
    llm->switchMode(Llm::Decode);
    llm->setKVCacheInfo(1, 0);
    llm->forward({200});
    llm->trace(false);
    // reset
    llm->reset();
    llm->setKVCacheInfo(0, llm->getCurrentHistory());
}
void MNNWrapper::tunePrefill() {
    llm->tuning(PREFILL_BIGLITTLE_CORE, {});
}
void MNNWrapper::startDecodeTune(int tolerance) {
    std::vector<int> empty;
    llm->decode_tuning(empty, nullptr, (int)tolerance);
}
bool MNNWrapper::endDecodeTune(std::vector<int>& plan, float* energy, int tolerance) {
    return llm->decode_tuning(plan, energy, tolerance);
}
int MNNWrapper::forward(const std::vector<int>& tokens, bool is_prefill, bool is_first_prefill) {
    VARP logits;
    if ((bool)is_prefill) {
        // test prefill
        llm->switchMode(Llm::Prefill);
        if ((bool)is_first_prefill) {
            llm->setKVCacheInfo(tokens.size(), llm->getCurrentHistory());
        } else {
            llm->setKVCacheInfo(tokens.size(), 0);
        }
    } else {
        // test decode, decode for length times
        llm->switchMode(Llm::Decode);
        llm->setKVCacheInfo(1, 0);
    }
    logits = llm->forward(tokens); // prefill a prompt of length length.
    return llm->sample(logits, {});
}
void MNNWrapper::reset() {
    llm->setKVCacheInfo(0, llm->getCurrentHistory());
    llm->reset();
}
std::vector<int> MNNWrapper::tokenizer_encode(const std::string& inputStr,
                                              bool use_template,
                                              bool need_antiprompt,
                                              std::string system_prompt) {
    std::vector<int> tokens = llm->tokenizer_encode(inputStr, use_template);
    if (!system_prompt.empty()) {
        auto sys = llm->tokenizer_encode(llm->apply_chat_template({std::make_pair(std::string("system"), system_prompt)}), false);
        tokens.insert(tokens.begin(), sys.begin(), sys.end());
    }
    if (need_antiprompt && !getAntiPrompt(model_name).empty()) {
        auto antiprompt = llm->tokenizer_encode(getAntiPrompt(model_name), false);
        tokens.insert(tokens.begin(), antiprompt.begin(), antiprompt.end());
    }
    return tokens;
}
std::string MNNWrapper::tokenizer_decode(const std::vector<int>& tokens) {
    std::string output_str;
    for (auto& t:tokens) {
        output_str += llm->tokenizer_decode(t);
    }
    return output_str;
}
bool MNNWrapper::isStop(int id) {
    return llm->is_stop(id);
}
MNNWrapper::~MNNWrapper() {}