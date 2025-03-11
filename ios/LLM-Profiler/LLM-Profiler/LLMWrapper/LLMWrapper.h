
#ifndef LLMWrapper_h
#define LLMWrapper_h


// LLMWrapper.h
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^CompletionHandler)(BOOL success);

@interface LLMWrapper : NSObject

- (instancetype)initWithModelPath:(NSString *)modelPath completion:(CompletionHandler)completion;

// - (void)processInput:(NSString *)input withOutput:(OutputHandler)output;

// - (void)addPromptsFromArray:(NSArray<NSDictionary *> *)array;

@end

NS_ASSUME_NONNULL_END


#endif
