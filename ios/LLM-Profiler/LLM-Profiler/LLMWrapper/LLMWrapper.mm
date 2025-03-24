
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

@implementation SwiftLLMWrapper {
    std::shared_ptr<LLMWrapper> model;
    void* wrapperHandle;
    void* datasetHandle;
    Class LLMWrapper;
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
            if ([engineName isEqualToString:@"MNN"]) {
                model.reset(LLMWrapper::createMNNWrapper(model_path.c_str(), backend_name, tmp_path, engine_name, prefill_thread_num, decode_thread_num, prefill_power_mode, decode_power_mode, decode_core_plan, tune_times));
            }
//                else if ([engineName isEqualToString:@"llama.cpp"]) {
//                model.reset(LLMWrapper::createLLAMACPPWrapper(model_path.c_str(), backend_name, tmp_path, engine_name, prefill_thread_num, decode_thread_num, prefill_power_mode, decode_power_mode, decode_core_plan, tune_times));
//            } else if ([engineName isEqualToString:@"mllm"]) {
//                model.reset(LLMWrapper::createMLLMWrapper(model_path.c_str(), backend_name, tmp_path, engine_name, prefill_thread_num, decode_thread_num, prefill_power_mode, decode_power_mode, decode_core_plan, tune_times));
//            } else if ([engineName isEqualToString:@"executorch"]) {
//                model.reset(LLMWrapper::createETWrapper(model_path.c_str(), backend_name, tmp_path, engine_name, prefill_thread_num, decode_thread_num, prefill_power_mode, decode_power_mode, decode_core_plan, tune_times));
//            }
        }
    }
    return YES;
}


@end
