//
// Created by hzx on 2025/2/21.
//


#include "llama_cpp_wrapper.h"
#include "chat-template.hpp"

#include <thread>

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
}
#endif

LLMWrapper* LLMWrapper::createLLAMACPPWrapper(const char* model_dir,
    std::string backend_name,
    std::string tmp_path,
    std::string engine_name,
    std::string prefill_thread_num,
    std::string decode_thread_num,
    std::string prefill_power_mode,
    std::string decode_power_mode,
    std::string decode_cores,
    std::string decode_tune_times) {
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
}

// llama_token is int32_t
std::string llamacppWrapper::chat_add_and_format(const std::string & role, const std::string & content) {
    common_chat_msg new_msg{role, content, {}};
    auto formatted = common_chat_format_single(*chat_templates.template_default, chat_msgs, new_msg, role == "user", false);
    chat_msgs.push_back({role, content, {}});
    return formatted;
}

llamacppWrapper::llamacppWrapper(const char* model_dir,
                                 std::string backend_name,
                                 std::string tmp_path,
                                 std::string prefill_thread_num,
                                 std::string decode_thread_num,
                                 std::string prefill_power_mode,
                                 std::string decode_power_mode,
                                 std::string decode_cores,
                                 std::string decode_tune_times) {
    // initialize params
    char* argv = new char[10]{"llama.cpp"};
    common_params_parse(1, &argv, params, LLAMA_EXAMPLE_MAIN);
    // set threading
    params.model = std::string(model_dir);
    if (std::atoi(prefill_thread_num.c_str())>0) {
        params.cpuparams_batch.n_threads = std::atoi(prefill_thread_num.c_str());
    }
    if (params.cpuparams_batch.n_threads <= 0) {
        params.cpuparams_batch.n_threads = std::thread::hardware_concurrency();
    }
    params.special = true;
    // initialize model and backend
    common_init();
    llama_backend_init();
    llama_numa_init(params.numa);

    // load the model and apply lora adapter, if any. vocab & templates
    llm = common_init_from_params(params);
    llama_model * model = llm.model.get();
    llama_context * ctx = llm.context.get();
    const llama_vocab * vocab = llama_model_get_vocab(model);
    chat_templates = common_chat_templates_from_model(model, params.chat_template);
    const int n_ctx_train = llama_model_n_ctx_train(model);
    const int n_ctx = llama_n_ctx(ctx);
    const bool has_chat_template = chat_templates.has_explicit_template && chat_templates.template_default;
    const bool add_bos = llama_vocab_get_add_bos(vocab) && !params.use_jinja;
    if (!llama_model_has_encoder(model)) {
        GGML_ASSERT(!llama_vocab_get_add_eos(vocab));
    }

    // CPU ThreadPool settings
    LOG_INF("%s: llama threadpool init, n_threads = %d\n", __func__, (int) params.cpuparams.n_threads);
    auto * reg = ggml_backend_dev_backend_reg(ggml_backend_dev_by_type(GGML_BACKEND_DEVICE_TYPE_CPU));
    auto * ggml_threadpool_new_fn = (decltype(ggml_threadpool_new) *) ggml_backend_reg_get_proc_address(reg, "ggml_threadpool_new");
    auto * ggml_threadpool_free_fn = (decltype(ggml_threadpool_free) *) ggml_backend_reg_get_proc_address(reg, "ggml_threadpool_free");
    struct ggml_threadpool_params tpp_batch =
            ggml_threadpool_params_from_cpu_params(params.cpuparams_batch);
    struct ggml_threadpool_params tpp =
            ggml_threadpool_params_from_cpu_params(params.cpuparams);
    set_process_priority(params.cpuparams.priority);
    if (!ggml_threadpool_params_match(&tpp, &tpp_batch)) {
        threadpool_batch = ggml_threadpool_new_fn(&tpp_batch);

        // Start the non-batch threadpool in the paused state
        tpp.paused = true;
    }
    threadpool = ggml_threadpool_new_fn(&tpp);
    llama_attach_threadpool(ctx, threadpool, threadpool_batch);

    // initialize sampler
    smpl = common_sampler_init(model, params.sampling);
    delete [] argv;

}
bool llamacppWrapper::isReady() {
    return (llm.model.get()!=nullptr);
}
void llamacppWrapper::trace() {
    return;
}
void llamacppWrapper::tunePrefill() {
    return;
}
void llamacppWrapper::startDecodeTune(int tolerance) {
    return;
}
bool llamacppWrapper::endDecodeTune(std::vector<int>& plan, float* energy, int tolerance) {
    return true;
}
int  llamacppWrapper::forward(const std::vector<int>& tokens, bool is_prefill, bool is_first_prefill) {
    llama_context * ctx = llm.context.get();
    llama_decode(ctx, llama_batch_get_one((llama_token *)tokens.data(), tokens.size()));
    const llama_token id = common_sampler_sample(smpl, ctx, -1);
    common_sampler_accept(smpl, id, /* accept_grammar= */ true);
    return (int)id;
}
void llamacppWrapper::reset() {
    llama_context * ctx = llm.context.get();
    llama_kv_cache_clear(ctx);
}
bool llamacppWrapper::isStop(int id) {
    llama_model * model = llm.model.get();
    const llama_vocab * vocab = llama_model_get_vocab(model);
    return llama_vocab_is_eog(vocab, (llama_token)id);
}
std::vector<int> llamacppWrapper::tokenizer_encode(const std::string& inputStr,
                                                   bool use_template,
                                                   bool need_antiprompt,
                                                   std::string system_prompt) {
    std::vector<llama_token> tokens;
    llama_context * ctx = llm.context.get();
    if (use_template) {
        // add chat prompt and tokenize
        const auto user_inp = common_tokenize(ctx, chat_add_and_format("user", inputStr), false, true);
        tokens.insert(tokens.end(), user_inp.begin(), user_inp.end());
        const auto line_pfx = common_tokenize(ctx, params.input_prefix, false, true);
        const auto line_sfx = common_tokenize(ctx, params.input_suffix, false, true);
        tokens.insert(tokens.begin(), line_pfx.begin(), line_pfx.end());
        tokens.insert(tokens.end(), line_sfx.begin(), line_sfx.end());
        if (!system_prompt.empty()) {
            const auto sys = common_tokenize(ctx, chat_add_and_format("system", system_prompt), true, true);
            tokens.insert(tokens.begin(), sys.begin(), sys.end());
        }
        if (need_antiprompt && !params.antiprompt.empty()) {
            const auto first_antiprompt = common_tokenize(ctx, params.antiprompt.front(), false, true);
            tokens.insert(tokens.begin(), first_antiprompt.begin(), first_antiprompt.end());
        }
    } else {
        // directly tokenize
        const auto user_inp = common_tokenize(ctx, inputStr, false, true);
        tokens.insert(tokens.end(), user_inp.begin(), user_inp.end());
    }
    std::vector<int> int_tokens;
    for (auto& id: tokens) {
        int_tokens.push_back((int)id);
    }
    return int_tokens;
}
std::string llamacppWrapper::tokenizer_decode(const std::vector<int>& tokens) {
    llama_context * ctx = llm.context.get();
    std::string output_str;
    for (auto& id: tokens) {
        output_str += common_token_to_piece(ctx, id, params.special);
    }
    return output_str;
}
llamacppWrapper::~llamacppWrapper() {
    auto * reg = ggml_backend_dev_backend_reg(ggml_backend_dev_by_type(GGML_BACKEND_DEVICE_TYPE_CPU));
    auto * ggml_threadpool_new_fn = (decltype(ggml_threadpool_new) *) ggml_backend_reg_get_proc_address(reg, "ggml_threadpool_new");
    auto * ggml_threadpool_free_fn = (decltype(ggml_threadpool_free) *) ggml_backend_reg_get_proc_address(reg, "ggml_threadpool_free");
    common_sampler_free(smpl);
    llama_backend_free();
    ggml_threadpool_free_fn(threadpool);
    ggml_threadpool_free_fn(threadpool_batch);
}