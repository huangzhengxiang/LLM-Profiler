#include <iostream>
#include "llm_wrapper.h"

std::unique_ptr<LLMWrapper> model;
int main() {
    const char* model_dir = "/data/local/tmp/llm/model/llama3_2-3b-instruct-q4-torch";
    model.reset(LLMWrapper::createWrapper(model_dir,
                                          "CPU",
                                          "",
                                          "executorch",
                                          "4",
                                          "4",
                                          "high",
                                          "high",
                                          "",
                                          ""));
    printf("ETWrapper loaded!\n");
    int res = model->forward(model->tokenizer_encode("Hello!"), true, true);
    for (auto& id : model->tokenizer_encode("Hello!")) { std::cout << id << " " << std::flush; }
    std::cout << std::endl;

    // test decode, decode for length times
    std::vector<int> outTokens = {res};
    while (!model->isStop(res)) {
        res = model->forward({res}, false, false);
        outTokens.push_back(res);
        // __android_log_print(ANDROID_LOG_INFO, "MNN_PROFILE", "res: %s (%d)", model->tokenizer_decode({res}).c_str(), res);
        std::cout << model->tokenizer_decode({res}) << std::flush;
    }
    // __android_log_print(ANDROID_LOG_INFO, "MNN_PROFILE", "res: %d", res);
    printf("%s\n", model->tokenizer_decode(outTokens).c_str());
    printf("ETWrapper forward!\n");
}