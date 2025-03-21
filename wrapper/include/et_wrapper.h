//
// Created by hzx on 2025/2/21.
//

#ifndef ET_WRAPPER_H_
#define ET_WRAPPER_H_

#include "llm_wrapper.h"

// executorch headers
#include <executorch/extension/llm/runner/irunner.h>
#include <executorch/extension/llm/runner/stats.h>
#include <executorch/extension/llm/runner/text_decoder_runner.h>
#include <executorch/extension/llm/runner/text_prefiller.h>
#include <executorch/extension/llm/runner/text_token_generator.h>
#include <executorch/extension/module/module.h>
#include <pytorch/tokenizers/tokenizer.h>


class ETWrapper : public LLMWrapper {
protected:
    // model
    std::unique_ptr<::executorch::extension::Module> module_;
    std::unique_ptr<::tokenizers::Tokenizer> tokenizer_;
    std::unordered_map<std::string, int64_t> metadata_;
    std::unique_ptr<::executorch::extension::llm::TextPrefiller> text_prefiller_;
    std::unique_ptr<::executorch::extension::llm::TextDecoderRunner> text_decoder_runner_;
    std::unique_ptr<std::unordered_set<uint64_t>> eos_ids;
    int64_t cur_pos = 0;
public:
    ETWrapper(const char* model_dir,
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
    virtual ~ETWrapper() override;
};

#endif // ET_WRAPPER_H_