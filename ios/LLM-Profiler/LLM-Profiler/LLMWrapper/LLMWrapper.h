
#ifndef LLMWrapper_h
#define LLMWrapper_h


// LLMWrapper.h
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^CompletionHandler)(BOOL success);

@interface SwiftLLMWrapper : NSObject

- (instancetype)initEngine:(NSString *)engineName modelPath:(NSString *)modelPath backendName:(NSString *)backendName tmpFile:(NSString *)tmpFile prefillThreadNum:(NSString *)prefillThreadNum decodeThreadNum:(NSString *)decodeThreadNum prefillPowerMode:(NSString *)prefillPowerMode decodePowerMode:(NSString *)decodePowerMode decodeCorePlan:(NSString *)decodeCorePlan tuneTimes:(NSString *)tuneTimes completion:(CompletionHandler)completion;

- (void)Trace;

- (void)startDecodeTune:(int)tolerance;

- (BOOL)endDecodeTune:(int *)core tolerance:(int)tolerance;

- (void)Forward:(int)length is_prefill:(BOOL)is_prefill is_first_prefill:(BOOL)is_first_prefill;

- (void)Reset;

- (int)getStringTokenSize:(NSString*)input;

- (int)getMaxKVLen;

- (int)getCurrentKVLen;

// - (void)processInput:(NSString *)input withOutput:(OutputHandler)output;

// - (void)addPromptsFromArray:(NSArray<NSDictionary *> *)array;

@end

NS_ASSUME_NONNULL_END


#endif
