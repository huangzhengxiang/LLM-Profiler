
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
            // BOOL success = [self loadModelFromPath:modelPath];
            // MARK: Test Local Model
            BOOL success = [self loadModel];

            dispatch_async(dispatch_get_main_queue(), ^{
                completion(success);
            });
        });
    }
    return self;
}

- (BOOL)loadModel {
    if (!llm) {
        NSString *bundleDirectory = [[NSBundle mainBundle] bundlePath];
        std::string model_dir = [bundleDirectory UTF8String];
        std::string config_path = model_dir + "/config.json";
        llm.reset(Llm::createLLM(config_path));
        NSString *tempDirectory = NSTemporaryDirectory();
        llm->set_config("{\"tmp_path\":\"" + std::string([tempDirectory UTF8String]) + "\", \"use_mmap\":false}");
        llm->load();
    }
    return YES;
}


@end