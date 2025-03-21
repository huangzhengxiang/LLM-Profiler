//
// Created by hzx on 2025/2/21.
//

#ifndef LLAMA_CPP_WRAPPER_H_
#define LLAMA_CPP_WRAPPER_H_

#include "llm_wrapper.h"


// llama.cpp headers
#include "arg.h"
#include "common.h"
#include "log.h"
#include "sampling.h"
#include "llama.h"


class llamacppWrapper : public LLMWrapper {
protected:
    common_init_result llm;
    common_params params;
    common_chat_templates chat_templates;
    std::vector<common_chat_msg> chat_msgs;
    common_sampler * smpl = NULL;
    struct ggml_threadpool * threadpool = NULL;
    struct ggml_threadpool * threadpool_batch = NULL;
public:
    llamacppWrapper(const char* model_dir,
                    std::string backend_name,
                    std::string tmp_path,
                    std::string prefill_thread_num,
                    std::string decode_thread_num,
                    std::string prefill_power_mode,
                    std::string decode_power_mode,
                    std::string decode_cores,
                    std::string decode_tune_times);
    std::string chat_add_and_format(const std::string & role, const std::string & content);
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
    virtual ~llamacppWrapper() override;
};


#endif // LLAMA_CPP_WRAPPER_H_