
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>
#include <functional>
#include <MNN/llm/llm.hpp>
#include <vector>
#include <utility>

#import <Foundation/Foundation.h>
#import "LLMWrapper.h"

using namespace MNN::Transformer;

@implementation LLMWrapper {
    std::shared_ptr<Llm> llm;
}

- (instancetype)initWithModelPath:(NSString *)modelPath completion:(CompletionHandler)completion {
    self = [super init];
    if (self) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            // MARK: Test Local Model
            BOOL success = [self loadModel:modelPath];

            dispatch_async(dispatch_get_main_queue(), ^{
                completion(success);
            });
        });
    }
    return self;
}

- (BOOL)loadModel:(NSString *)modelPath {
    if (!llm) {
        std::string model_dir = [modelPath UTF8String];
        std::string config_path = model_dir + "/config.json";
        llm.reset(Llm::createLLM(config_path));
        NSString *tempDirectory = NSTemporaryDirectory();
        llm->set_config("{\"tmp_path\":\"" + std::string([tempDirectory UTF8String]) + "\", \"use_mmap\":false}");
        llm->load();
    }
    return YES;
}


@end