//
// Created by hzx on 2025/2/21.
//

#ifndef LLM_WRAPPER_H
#define LLM_WRAPPER_H

// system headers
#include <string>
#include <vector>
#include <sstream>
#include <thread>

// only need to support Android & iOS, this symbol is enough
class __attribute__((visibility("default"))) LLMWrapper {
public:
    enum ModelType {
        Qwen2,
        Llama3,
        Gemma,
        Gemma2,
        Phi3,
    };
    static LLMWrapper* createWrapper(const char* model_dir,
                                     std::string backend_name,
                                     std::string tmp_path,
                                     std::string engine_name,
                                     std::string prefill_thread_num,
                                     std::string decode_thread_num,
                                     std::string prefill_power_mode,
                                     std::string decode_power_mode,
                                     std::string decode_cores,
                                     std::string decode_tune_times);
    static LLMWrapper* createMNNWrapper(const char* model_dir,
                                        std::string backend_name,
                                        std::string tmp_path,
                                        std::string engine_name,
                                        std::string prefill_thread_num,
                                        std::string decode_thread_num,
                                        std::string prefill_power_mode,
                                        std::string decode_power_mode,
                                        std::string decode_cores,
                                        std::string decode_tune_times);
    static LLMWrapper* createLLAMACPPWrapper(const char* model_dir,
                                        std::string backend_name,
                                        std::string tmp_path,
                                        std::string engine_name,
                                        std::string prefill_thread_num,
                                        std::string decode_thread_num,
                                        std::string prefill_power_mode,
                                        std::string decode_power_mode,
                                        std::string decode_cores,
                                        std::string decode_tune_times);
    static LLMWrapper* createMLLMWrapper(const char* model_dir,
                                        std::string backend_name,
                                        std::string tmp_path,
                                        std::string engine_name,
                                        std::string prefill_thread_num,
                                        std::string decode_thread_num,
                                        std::string prefill_power_mode,
                                        std::string decode_power_mode,
                                        std::string decode_cores,
                                        std::string decode_tune_times);
    static LLMWrapper* createETWrapper(const char* model_dir,
                                        std::string backend_name,
                                        std::string tmp_path,
                                        std::string engine_name,
                                        std::string prefill_thread_num,
                                        std::string decode_thread_num,
                                        std::string prefill_power_mode,
                                        std::string decode_power_mode,
                                        std::string decode_cores,
                                        std::string decode_tune_times);
    virtual bool isReady() { return true; }
    virtual void trace() {}
    virtual void tunePrefill() {}
    virtual void startDecodeTune(int tolerance) {}
    virtual bool endDecodeTune(std::vector<int>& plan, float* energy, int tolerance) { return true; }
    virtual int forward(const std::vector<int>& tokens, bool is_prefill, bool is_first_prefill) { return 0; }
    virtual void reset() {}
    virtual bool isStop(int id)=0;
    virtual std::vector<int> tokenizer_encode(const std::string& inputStr,
                                              bool use_template = true,
                                              bool need_antiprompt = false,
                                              std::string system_prompt = "")=0;
    virtual std::string tokenizer_decode(const std::vector<int>& tokens)=0;
    virtual ~LLMWrapper() {}
};

#endif // LLM_WRAPPER_H
