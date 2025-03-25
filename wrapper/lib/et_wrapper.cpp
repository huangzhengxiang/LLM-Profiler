/* Code Reference:
 * executorch/extension/llm
 * executorch/examples/models/llama/
 */

#include "et_wrapper.h"

#include <executorch/examples/models/llama/runner/runner.h>
#include <executorch/extension/llm/runner/util.h>
#include <executorch/examples/models/llama/tokenizer/llama_tiktoken.h>
#include <pytorch/tokenizers/llama2c_tokenizer.h>

using ::executorch::extension::Module;
using ::executorch::runtime::Error;
using ::executorch::runtime::Result;

namespace llm = ::executorch::extension::llm;
namespace {
static constexpr auto kEnableDynamicShape = "enable_dynamic_shape";
static constexpr auto kBosId = "get_bos_id";
static constexpr auto kEosIds = "get_eos_ids";
static constexpr auto kMaxSeqLen = "get_max_seq_len";
static constexpr auto kVocabSize = "get_vocab_size";
static constexpr auto kUseKVCache = "use_kv_cache";
static constexpr auto kUseSDPAWithKVCache = "use_sdpa_with_kv_cache";
} // namespace

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
    if (engine_name=="executorch") {
        return new ETWrapper(model_dir,
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

LLMWrapper* LLMWrapper::createETWrapper(const char* model_dir,
    std::string backend_name,
    std::string tmp_path,
    std::string engine_name,
    std::string prefill_thread_num,
    std::string decode_thread_num,
    std::string prefill_power_mode,
    std::string decode_power_mode,
    std::string decode_cores,
    std::string decode_tune_times) {
    if (engine_name=="executorch") {
        return new ETWrapper(model_dir,
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

std::vector<uint64_t> convert2unsigned(const std::vector<int>& ids) {
    std::vector<uint64_t> token_ids;
    for (const auto& id: ids) {
        token_ids.push_back((uint64_t)id);
    }
    return token_ids;
}
std::vector<int> convert2int(const std::vector<uint64_t>& token_ids) {
    std::vector<int> ids;
    for (const auto& id: token_ids) {
        ids.push_back((int)id);
    }
    return ids;  
}
std::string apply_system_prompt(std::string system_prompt) {
    return "<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n" + system_prompt + "<|eot_id|>";
}
std::string apply_user_prompt(std::string user_prompt) {
    return "<|start_header_id|>user<|end_header_id|>\n\n" + user_prompt + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
}
std::string getAntiPrompt() {
    return "";
}
ETWrapper::ETWrapper(const char* model_dir,
                     std::string backend_name,
                     std::string tmp_path,
                     std::string prefill_thread_num,
                     std::string decode_thread_num,
                     std::string prefill_power_mode,
                     std::string decode_power_mode,
                     std::string decode_cores,
                     std::string decode_tune_times):metadata_({
                        {kEnableDynamicShape, false},
                        {kMaxSeqLen, 2048},
                        {kUseKVCache, true},
                        {kUseSDPAWithKVCache, false},
                     }) {
    module_ = std::make_unique<Module>(std::string(model_dir) + "/model.pte", Module::LoadMode::File);
    module_->load_method("forward");
    // load tokenizer. Assuming tiktoken is the default tokenizer
    tokenizer_ = nullptr;
    tokenizer_ = example::get_tiktoken_for_llama();
    tokenizer_->load(std::string(model_dir) + "/tokenizer.bin");

    metadata_[kBosId] = tokenizer_->bos_tok();
    eos_ids = std::make_unique<std::unordered_set<uint64_t>>(std::unordered_set<uint64_t>{tokenizer_->eos_tok()});
    metadata_[kVocabSize] = tokenizer_->vocab_size();

    // methods
    const auto method_names = module_->method_names().get();
    for (auto& pair : metadata_) {
        const auto& method_name = pair.first;
        auto& value = pair.second;
        if (method_names.count(method_name)) {
            value = module_->get(method_name).get().toScalar().to<decltype(metadata_)::mapped_type>();
        } else {
            ET_LOG(
                Info,
                "Methond %s not found, using the default value %" PRId64,
                method_name.c_str(),
                value);
        }
        ET_LOG(Info, "Metadata: %s = %" PRId64, method_name.c_str(), value);
    }
    if (method_names.count(kEosIds)) {
        eos_ids->clear();
        for (int i=0; i<module_->execute(kEosIds)->size(); ++i) {
            auto value = (*(module_->execute(kEosIds))).operator[](i).toScalar().to<int64_t>();
            eos_ids->emplace(value);
            ET_LOG(Info, "eos_id = %" PRId64, value);
        }
    }

    // runners
    text_decoder_runner_ = std::make_unique<llm::TextDecoderRunner>(
        module_.get(),
        metadata_.at(kUseKVCache),
        metadata_.at(kVocabSize),
        0.8f); // temperature 0.8f
    text_prefiller_ = std::make_unique<llm::TextPrefiller>(
        text_decoder_runner_.get(),
        metadata_.at(kUseKVCache),
        metadata_.at(kEnableDynamicShape));
}
bool ETWrapper::isReady() {
    return (module_->is_loaded() && tokenizer_ && text_decoder_runner_ && text_prefiller_);
}
void ETWrapper::trace() {}
void ETWrapper::tunePrefill() {}
void ETWrapper::startDecodeTune(int tolerance) {}
bool ETWrapper::endDecodeTune(std::vector<int>& plan, float* energy, int tolerance) { return true; }
int  ETWrapper::forward(const std::vector<int>& tokens, bool is_prefill, bool is_first_prefill) {
    if (is_prefill && is_first_prefill) {
        cur_pos = 0;
    }
    if (is_prefill) {
        // prefill
        std::vector<uint64_t> unsigned_tokens = convert2unsigned(tokens);
        auto res = text_prefiller_->prefill(unsigned_tokens, cur_pos);
        // cur_pos already modified !!!! (No need to add again)
        return (int)(res.get());
    } else {
        // decode
        uint64_t cur_token = (uint64_t)tokens.back();
        std::vector<uint64_t> token_data = {cur_token};
        std::vector<executorch::aten::SizesType> token_shape = {1, 1};
        // initialize input tensor wrappers
        auto tokens_managed = executorch::extension::from_blob(token_data.data(), token_shape, 
                                executorch::aten::ScalarType::Long);
        auto start_pos_managed = executorch::extension::from_blob(&cur_pos, {1}, 
                                executorch::aten::ScalarType::Long);
        // execution
        auto logits_res = text_decoder_runner_->step(tokens_managed, start_pos_managed);
        executorch::aten::Tensor& logits_tensor = logits_res.get();
        // sampling
        auto res = text_decoder_runner_->logits_to_token(logits_tensor);
        cur_pos++;
        return (int)res;
    }
}
void ETWrapper::reset() {
    cur_pos = 0;
}
bool ETWrapper::isStop(int id) {
    if (eos_ids->find((uint64_t)id) != eos_ids->end()) {
        return true;
    }
    return false;
}
std::vector<int> ETWrapper::tokenizer_encode(const std::string& inputStr,
                                             bool use_template,
                                             bool need_antiprompt,
                                             std::string system_prompt) {
    std::string text = "";
    if (need_antiprompt && !getAntiPrompt().empty()) {
        auto antiprompt = getAntiPrompt();
        text += antiprompt;
    }
    if (!system_prompt.empty()) {
        text += apply_system_prompt(system_prompt);
    }
    if (use_template) {
        text += apply_user_prompt(inputStr);
    } else {
        text += inputStr;
    }    
    return convert2int(tokenizer_->encode(text, 0, 0).get());
}
std::string ETWrapper::tokenizer_decode(const std::vector<int>& tokens) {
    // prev token isn't used in tiktoken, but we still implement it correctly here.
    std::string output_str = "";
    int prev = tokens.front();
    for (const auto& id: tokens) {
        output_str += tokenizer_->decode((uint64_t)prev, (uint64_t)id).get();
        prev = id;
    }
    return output_str;
}
ETWrapper::~ETWrapper() {}
