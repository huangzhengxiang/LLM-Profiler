//
// Created by hzx on 2025/3/14.
//

#include "llm_wrapper.h"
#include "tokenizers/Tokenizer.hpp"
#include "processor/PostProcess.hpp"

using namespace mllm;

std::vector<token_id_t> convert2unsigned(const std::vector<int>& ids) {
    std::vector<token_id_t> token_ids;
    for (const auto& id: ids) {
        token_ids.push_back((token_id_t)id);
    }
    return token_ids;
}

std::vector<int> convert2int(const std::vector<token_id_t>& token_ids) {
    std::vector<int> ids;
    for (const auto& id: token_ids) {
        ids.push_back((int)id);
    }
    return ids;  
}

bool mllmWrapper::parseModelInfo(std::string model_dir) {
    // parse model_type
    if (model_dir.find("qwen2")!=std::string::npos) {
        model_type = Qwen2;
    } else if (model_dir.find("llama3")!=std::string::npos) {
        model_type = Llama3;
    } else if (model_dir.find("gemma")!=std::string::npos) {
        model_type = Gemma;
    } else if (model_dir.find("gemma2")!=std::string::npos) {
        model_type = Gemma2;
    } else {
        return false;
    }
    // parse billion
    if (model_dir.find("0_5b")!=std::string::npos || model_dir.find("0.5b")!=std::string::npos
        || model_dir.find("0_5B")!=std::string::npos || model_dir.find("0.5B")!=std::string::npos) {
        billion="0.5B";
    } else if (model_dir.find("1b")!=std::string::npos || model_dir.find("1B")!=std::string::npos) {
        billion="1B";
    } else if (model_dir.find("1_5b")!=std::string::npos || model_dir.find("1.5b")!=std::string::npos
        || model_dir.find("1_5B")!=std::string::npos || model_dir.find("1.5B")!=std::string::npos) {
        billion="1.5B";
    } else if (model_dir.find("1_8b")!=std::string::npos || model_dir.find("1.8b")!=std::string::npos
        || model_dir.find("1_8B")!=std::string::npos || model_dir.find("1.8B")!=std::string::npos) {
        billion="1.8B";
    } else if (model_dir.find("2b")!=std::string::npos || model_dir.find("2B")!=std::string::npos) {
        billion="2B";
    } else if (model_dir.find("2_5b")!=std::string::npos || model_dir.find("2.5b")!=std::string::npos
        || model_dir.find("2_5B")!=std::string::npos || model_dir.find("2.5B")!=std::string::npos) {
        billion="2.5B";
    } else if (model_dir.find("3b")!=std::string::npos || model_dir.find("3B")!=std::string::npos) {
        billion="3B";
    } else if (model_dir.find("4b")!=std::string::npos || model_dir.find("4B")!=std::string::npos) {
        billion="4B";
    } else if (model_dir.find("7b")!=std::string::npos || model_dir.find("7B")!=std::string::npos) {
        billion="7B";
    } else if (model_dir.find("8b")!=std::string::npos || model_dir.find("8B")!=std::string::npos) {
        billion="8B";
    } else {
        return false;
    }
    return true;
}

void mllmWrapper::loadModel(std::string model_dir) {
    int tokens_limit = 2048; // default using 2048 as token limits.
    std::string vocab_path;
    std::string merge_path;
    std::string model_path;
    switch (model_type) {
        case Qwen2: {
            vocab_path = model_dir + "/vocab.mllm";
            merge_path = model_dir + "/merge.txt";
            model_path = model_dir + "/model.mllm";
            tokenizer.reset(new QWenTokenizer(vocab_path, merge_path));
            config.reset(new QWenConfig(tokens_limit, billion, RoPEType::HFHUBROPE));
            llm.reset(new QWenForCausalLM(*(QWenConfig*)(config.get())));
            llm->load(model_path);
            break;
        }
        case Llama3: {
            vocab_path = model_dir + "/vocab.model";
            model_path = model_dir + "/model.mllm";
            tokenizer.reset(new LLama3Tokenizer(vocab_path));
            config.reset(new Llama3Config(tokens_limit, billion));
            llm.reset(new Llama3Model(*(Llama3Config*)(config.get())));
            llm->load(model_path);
            break;
        }
        case Gemma: {

            break;
        }
        case Gemma2: {
            break;
        }
    }
}

mllmWrapper::mllmWrapper(const char* model_dir,
                         std::string backend_name,
                         std::string tmp_path,
                         std::string prefill_thread_num,
                         std::string decode_thread_num,
                         std::string prefill_power_mode,
                         std::string decode_power_mode,
                         std::string decode_cores,
                         std::string decode_tune_times) {
    if (!llm.get()) {
        bool success = parseModelInfo(std::string(model_dir));
        if (!backend_name.empty()) { 
            // currently only support cpu
        }
        if (!prefill_thread_num.empty()) { CPUBackend::cpu_threads = std::atoi(prefill_thread_num.c_str()); } // thread_num (int, no quotation marks)
        if (!success) {
            return;
        }
        loadModel(std::string(model_dir));
    }
}
void mllmWrapper::trace() {}
bool mllmWrapper::isReady() {
    return (llm.get() != nullptr);
}

std::string mllmWrapper::apply_system_prompt(std::string system_prompt) {
    switch (model_type) {
        case Qwen2: {
            break;
        }
        case Llama3: {
            return "<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n" + system_prompt + "<|eot_id|>";
        }
        case Gemma: {
            break;
        }
        case Gemma2: {
            break;
        }
    }
}
std::string mllmWrapper::apply_user_prompt(std::string user_prompt) {
    switch (model_type) {
        case Qwen2: {
            break;
        }
        case Llama3: {
            return "<|start_header_id|>user<|end_header_id|>\n\n" + user_prompt + "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
            break;
        }
        case Gemma: {
            break;
        }
        case Gemma2: {
            break;
        }
    }
}
std::vector<int> mllmWrapper::tokenizeInternal(const std::string& text) {
    auto tokens_id = vector<token_id_t>();
    switch(model_type) {
        case Qwen2: {
            break;
        }
        case Llama3: {
            auto tensor = tokenizer->tokenize(text);
            for (int i = 0; i < tensor.sequence(); ++i) {
                auto value = tensor.dataAt<float>(0, 0, i, 0);
                tokens_id.push_back(value);
            }
            break;
        }
    }
    return convert2int(tokens_id);
}

std::string mllmWrapper::getAntiPrompt() {
    switch (model_type) {
        case Qwen2: {
            return "<|im_end|>\n";
        }
        case Llama3: {
            return "<|eot_id|>";
        }
        case Gemma: case Gemma2: {
            return "<end_of_turn>\n";
        }
    }
    return "";
}
void mllmWrapper::tunePrefill() {}
void mllmWrapper::startDecodeTune(int tolerance) {}
bool mllmWrapper::endDecodeTune(std::vector<int>& plan, float* energy, int tolerance) {
    return true;
}
int mllmWrapper::forward(const std::vector<int>& tokens, bool is_prefill, bool is_first_prefill) {
    auto input_tensor = mllm::Tokenizer::tokens2Input(convert2unsigned(tokens));
    auto result = llm->operator()({input_tensor}); // explicitize the operator() for better understandability.
    auto [out_string, out_token] = tokenizer->detokenize(result[0]);
    return (int)out_token;
}
void mllmWrapper::reset() {
    llm->clear_kvcache();
}
std::vector<int> mllmWrapper::tokenizer_encode(const std::string& inputStr,
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
    return tokenizeInternal(text);
}
std::string mllmWrapper::tokenizer_decode(const std::vector<int>& tokens) {
    switch (model_type) {
        case Qwen2: {
            break;
        }
        case Llama3: {
            return tokenizer->detokenize(convert2unsigned(tokens));
        }
    }
}
bool mllmWrapper::isStop(int id) {
    switch (model_type) {
        case Qwen2: {
            break;
        }
        case Llama3: {
            std::string text = tokenizer->detokenize({(token_id_t)id});
            return (text == "<|endoftext|>" || text == "<|eot_id|>");
        }
        case Gemma: {
            break;
        }
        case Gemma2: {
            break;
        }
    }
    // impossible to get here if model is supported!
    return true;
}
mllmWrapper::~mllmWrapper() {}