//
// Created by hzx on 2025/2/21.
//

#ifndef MNN_WRAPPER_H_
#define MNN_WRAPPER_H_

#include "llm_wrapper.h"

// MNN headers
#include "llm/llm.hpp"


using namespace MNN;
using namespace MNN::Express;
using namespace MNN::Transformer;


class MNNWrapper : public LLMWrapper {
protected:
    std::unique_ptr<Llm> llm;
    std::string model_name;
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
    virtual std::vector<int> tokenizer_encode(const std::string& inputStr,
                                              bool use_template = true,
                                              bool need_antiprompt = false,
                                              std::string system_prompt = "") override;
    virtual std::string tokenizer_decode(const std::vector<int>& tokens) override;
    virtual ~MNNWrapper() override;
};


#endif // MNN_WRAPPER_H_