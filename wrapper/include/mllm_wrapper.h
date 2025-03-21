//
// Created by hzx on 2025/2/21.
//

#ifndef MLLM_WRAPPER_H_
#define MLLM_WRAPPER_H_

#include "llm_wrapper.h"

// mllm headers
#include "models/qwen/configuration_qwen.hpp"
#include "models/qwen/modeling_qwen.hpp"
#include "models/qwen/tokenization_qwen.hpp"
#include "models/llama3/modeling_llama3.hpp"
#include "models/llama3/tokenization_llama3.hpp"
#include "models/gemma/configuration_gemma.hpp"
#include "models/gemma/modeling_gemma.hpp"
#include "models/gemma/tokenization_gemma.hpp"
#include "models/gemma2/configuration_gemma2.hpp"
#include "models/gemma2/modeling_gemma2.hpp"
#include "models/phi3/modeling_phi3.hpp"
#include "models/phi3/tokenization_phi3.hpp"
#include "models/transformer/configuration_transformer.hpp"
#include "Module.hpp"
#include "tokenizers/Tokenizer.hpp"

using namespace mllm;

class mllmWrapper : public LLMWrapper {
protected:
    ModelType model_type;
    std::string billion;
    std::unique_ptr<TransformerConfig> config;
    std::unique_ptr<mllm::Tokenizer> tokenizer;
    std::unique_ptr<mllm::Module> llm;
    bool parseModelInfo(std::string model_dir);
    void loadModel(std::string model_dir);
    std::string apply_system_prompt(std::string system_prompt);
    std::string apply_user_prompt(std::string user_prompt);
    std::vector<int> tokenizeInternal(const std::string& text);
    std::string getAntiPrompt();
public:
    mllmWrapper(const char* model_dir,
                    std::string backend_name,
                    std::string tmp_path,
                    std::string prefill_thread_num,
                    std::string decode_thread_num,
                    std::string prefill_power_mode,
                    std::string decode_power_mode,
                    std::string decode_cores,
                    std::string decode_tune_times);
    virtual bool isReady() override;
    virtual void trace() override;
    virtual void tunePrefill() override;
    virtual void startDecodeTune(int tolerance) override;
    virtual bool endDecodeTune(std::vector<int>& plan, float* energy, int tolerance) override;
    virtual int forward(const std::vector<int>& tokens, bool is_prefill, bool is_first_prefill) override;
    virtual void reset() override;
    virtual bool isStop(int id) override;
    virtual std::vector<int> tokenizer_encode(const std::string& inputStr,
                                              bool use_template = true,
                                              bool need_antiprompt = false,
                                              std::string system_prompt = "") override;
    virtual std::string tokenizer_decode(const std::vector<int>& tokens) override;
    virtual ~mllmWrapper() override;
};


#endif // MLLM_WRAPPER_H_