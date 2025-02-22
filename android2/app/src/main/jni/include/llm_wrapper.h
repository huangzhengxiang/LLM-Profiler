//
// Created by hzx on 2025/2/21.
//

#ifndef LLM_WRAPPER_H
#define LLM_WRAPPER_H

// Android headers
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>

// system headers
#include <jni.h>
#include <string>
#include <vector>
#include <sstream>
#include <thread>

// MNN headers
#include "llm/llm.hpp"
// llama.cpp headers
#include "arg.h"
#include "common.h"
#include "log.h"
#include "sampling.h"
#include "llama.h"

using namespace MNN;
using namespace MNN::Express;
using namespace MNN::Transformer;


class LLMWrapper {
public:
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
    virtual bool isReady() { return true; }
    virtual void trace() {}
    virtual void tunePrefill() {}
    virtual void startDecodeTune(int tolerance) {}
    virtual bool endDecodeTune(std::vector<int>& plan, float* energy, int tolerance) { return true; }
    virtual int forward(const std::vector<int>& tokens, bool is_prefill, bool is_first_prefill) { return 0; }
    virtual void reset() {}
    virtual bool isStop(int id)=0;
    virtual std::vector<int> tokenizer_encode(const std::string& inputStr, bool use_template = true, bool need_antiprompt = false)=0;
    virtual std::string tokenizer_decode(const std::vector<int>& tokens)=0;
    virtual ~LLMWrapper() {}
};

class MNNWrapper : public LLMWrapper {
protected:
    std::unique_ptr<Llm> llm;
public:
    MNNWrapper(const char* model_dir,
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
    virtual std::vector<int> tokenizer_encode(const std::string& inputStr, bool use_template = true, bool need_antiprompt = false) override;
    virtual std::string tokenizer_decode(const std::vector<int>& tokens) override;
    virtual ~MNNWrapper() override;
};

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
    virtual std::vector<int> tokenizer_encode(const std::string& inputStr, bool use_template = true, bool need_antiprompt = false) override;
    virtual std::string tokenizer_decode(const std::vector<int>& tokens) override;
    virtual ~llamacppWrapper() override;
};

#endif // LLM_WRAPPER_H
