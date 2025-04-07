
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>
#include <functional>
#include "mnn_wrapper/llm_wrapper.h"
#include <vector>
#include <utility>

#import <Foundation/Foundation.h>
#import "LLMWrapper.h"

//using namespace MNN::Transformer;
#define TEST_TOKEN 200

@implementation SwiftLLMWrapper {
    std::shared_ptr<LLMWrapper> model;
    int current_kv_len;
    int max_kv_len;
}

- (instancetype)initEngine:(NSString *)engineName modelPath:(NSString *)modelPath backendName:(NSString *)backendName tmpFile:(NSString *)tmpFile prefillThreadNum:(NSString *)prefillThreadNum decodeThreadNum:(NSString *)decodeThreadNum prefillPowerMode:(NSString *)prefillPowerMode decodePowerMode:(NSString *)decodePowerMode decodeCorePlan:(NSString *)decodeCorePlan tuneTimes:(NSString *)tuneTimes completion:(CompletionHandler)completion {
    self = [super init];
    if (self) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            // MARK: Test Local Model
            BOOL success = [self loadNative:engineName modelPath:modelPath backendName:backendName tmpFile:tmpFile prefillThreadNum:prefillThreadNum decodeThreadNum:decodeThreadNum prefillPowerMode:prefillPowerMode decodePowerMode:decodePowerMode decodeCorePlan:decodeCorePlan tuneTimes:tuneTimes];

            dispatch_async(dispatch_get_main_queue(), ^{
                completion(success);
            });
        });
    }
    return self;
}



- (BOOL)loadNative:(NSString *)engineName modelPath:(NSString *)modelPath backendName:(NSString *)backendName tmpFile:(NSString *)tmpFile prefillThreadNum:(NSString *)prefillThreadNum decodeThreadNum:(NSString *)decodeThreadNum prefillPowerMode:(NSString *)prefillPowerMode decodePowerMode:(NSString *)decodePowerMode decodeCorePlan:(NSString *)decodeCorePlan tuneTimes:(NSString *)tuneTimes {
    // init current kv len
    current_kv_len = 0;
    max_kv_len = 2048;
    // currently MNN only
    if (std::string([engineName UTF8String])=="MNN") {
        if (!model) {
            // 0. interpret params
            std::string model_path = std::string([modelPath UTF8String]);
            std::string backend_name = std::string([backendName UTF8String]);
            std::string tmp_path = std::string([tmpFile UTF8String]);
            std::string engine_name = std::string([engineName UTF8String]);
            std::string prefill_thread_num = std::string([prefillThreadNum UTF8String]);
            std::string decode_thread_num = std::string([decodeThreadNum UTF8String]);
            std::string prefill_power_mode = std::string([prefillPowerMode UTF8String]);
            std::string decode_power_mode = std::string([decodePowerMode UTF8String]);
            std::string decode_core_plan = std::string([decodeCorePlan UTF8String]);
            std::string tune_times = std::string([tuneTimes UTF8String]);
            if (prefill_power_mode == "tune_prefill") prefill_power_mode = "high";
            if (prefill_power_mode == "(default)") prefill_power_mode = "";
            if (decode_power_mode == "(default)") decode_power_mode = "";
            if ([engineName isEqualToString:@"MNN"]) {
                if (backend_name=="CPU") {
                    backend_name = "cpu";
                } else if (backend_name=="GPU") {
                    backend_name = "metal";
                } else {
                    backend_name = "cpu"; // cpu by default
                }
                model.reset(LLMWrapper::createMNNWrapper(model_path.c_str(), backend_name, tmp_path, engine_name, prefill_thread_num, decode_thread_num, prefill_power_mode, decode_power_mode, decode_core_plan, tune_times));
            }
//                else if ([engineName isEqualToString:@"llama.cpp"]) {
//                model.reset(LLMWrapper::createLLAMACPPWrapper(model_path.c_str(), backend_name, tmp_path, engine_name, prefill_thread_num, decode_thread_num, prefill_power_mode, decode_power_mode, decode_core_plan, tune_times));
//            } else if ([engineName isEqualToString:@"executorch"]) {
//                model.reset(LLMWrapper::createETWrapper(model_path.c_str(), backend_name, tmp_path, engine_name, prefill_thread_num, decode_thread_num, prefill_power_mode, decode_power_mode, decode_core_plan, tune_times));
//            }
        }
    }
    return YES;
}

- (void)Trace {
    model->trace();
}

- (void)startDecodeTune:(int)tolerance {
    model->startDecodeTune(tolerance);
}

- (BOOL)endDecodeTune:(int *)core tolerance:(int)tolerance {
    if (tolerance<0) {
        return true; // no tuning
    }
    std::vector<int> core_plan;
    bool tune_end = model->endDecodeTune(core_plan, nullptr, (int) tolerance);
    if (tune_end) {
        (*core) = core_plan[0];
    }
    return tune_end;
}

- (void)Forward:(int)length is_prefill:(BOOL)is_prefill is_first_prefill:(BOOL)is_first_prefill {
    int res = TEST_TOKEN;
    if ((bool) is_prefill) {
        // test prefill
        std::vector<int> test_prompt((int) length, TEST_TOKEN);
        res = model->forward(test_prompt, is_prefill, is_first_prefill);
    } else {
        // test decode, decode for length times
        std::vector<int> test_prompt(1, TEST_TOKEN);
        for (int i = 0; i < (int) length; ++i) {
            test_prompt[0] = res;
            res = model->forward(test_prompt, is_prefill, is_first_prefill);
        }
    }
    printf("res: %d", res);
    printf("After Foward!");
    return;
}

- (void)Reset {
    current_kv_len = 0;
    model->reset();
}


- (int)getStringTokenSize:(NSString*)input {
    return (int)model->tokenizer_encode(std::string([input UTF8String]), false, false, "").size();
}

- (int)getMaxKVLen {
    return max_kv_len;
}

- (int)getCurrentKVLen {
    return current_kv_len;
}

@end
