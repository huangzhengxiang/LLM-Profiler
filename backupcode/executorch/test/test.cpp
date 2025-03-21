#include <iostream>
#include "llm_wrapper.h"

std::unique_ptr<LLMWrapper> model;
int main() {
    const char* model_dir = "/data/local/tmp/llm/model/llama3_2-1b-instruct-q4-torch";
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
    model->forward({200, 200}, true, true);
    printf("ETWrapper forward!\n");
}