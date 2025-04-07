// swift counterpart of Chat.java

class Chat: ObservableObject {
    private var llm: SwiftLLMWrapper?
    @Published var isModelLoaded = false
    init() {}
    func setupLLM(engineName: String,
                  modelPath: String,
                  backendName: String,
                  tmpFile: String,
                  prefillThreadNum: String,
                  decodeThreadNum: String,
                  prefillPowerMode: String,
                  decodePowerMode: String,
                  decodeCorePlan: String,
                  tuneTimes: String,
                  completion: @escaping (String) -> Void) {
        llm = SwiftLLMWrapper(engine: engineName,
                         modelPath: modelPath,
                         backendName: backendName,
                         tmpFile: tmpFile,
                         prefillThreadNum: prefillThreadNum,
                         decodeThreadNum: decodeThreadNum,
                         prefillPowerMode: prefillPowerMode,
                         decodePowerMode: decodePowerMode,
                         decodeCorePlan: decodeCorePlan,
                         tuneTimes: tuneTimes)
        { [weak self] success in
            Task { @MainActor in
                self?.isModelLoaded = success
                completion("model is loaded, decode tuning")
            }
        }
    }
    
    func decodeTune(tolerance: String,
                    completion: @escaping (Int) -> Void) {
        Task { @MainActor in
            var decode_tune_tolerance: Int32 = -1
            if let tolerance_hint = Int32(tolerance) {
                decode_tune_tolerance = tolerance_hint
            }
            
            if (decode_tune_tolerance<0) {
                return // no decode tuning
            }
            
            var tune_end = false
            var decodeCorePlan:Int32 = 2
            
            while (!tune_end) {
                // sleep
                try? await Task.sleep(nanoseconds: 5_000_000_000)  // wait seconds in nanoseconds
                llm?.startDecodeTune(decode_tune_tolerance)
                if let res = llm?.endDecodeTune(&decodeCorePlan, tolerance: decode_tune_tolerance) {
                    tune_end = res
                } else {
                    tune_end = true
                }
            }
            completion(Int(decodeCorePlan))
        }
    }
    
    func FixedLengthTestRun(prefill_len: Int,
                            decode_len: Int,
                            test_times: Int,
                            statusCallBack: @escaping (String) -> Void,
                            resultsCallBack: @escaping (CFAbsoluteTime?, CFAbsoluteTime?) -> Void) {
        Task { @MainActor in
            
            var prefillStartTimes: [CFAbsoluteTime] = []
            var prefillEndTimes: [CFAbsoluteTime] = []
            var decodeStartTimes: [CFAbsoluteTime] = []
            var decodeEndTimes: [CFAbsoluteTime] = []

            // Loop to execute the code repeatedly
            for itr in 0..<test_times {
                
                let prefillStartTime = CFAbsoluteTimeGetCurrent()
                llm?.forward(Int32(prefill_len), is_prefill: true, is_first_prefill: true)
                let prefillEndTime = CFAbsoluteTimeGetCurrent()
                let decodeStartTime = CFAbsoluteTimeGetCurrent()
                llm?.forward(Int32(decode_len), is_prefill: false, is_first_prefill: false)
                let decodeEndTime = CFAbsoluteTimeGetCurrent()
                llm?.reset()
                
                // Append the timing values to the respective lists
                prefillStartTimes.append(prefillStartTime)
                prefillEndTimes.append(prefillEndTime)
                decodeStartTimes.append(decodeStartTime)
                decodeEndTimes.append(decodeEndTime)
                
                // sleep
                try? await Task.sleep(nanoseconds: 5_000_000_000)  // wait seconds in nanoseconds
                
                statusCallBack(String(format: "Fixed Len Test: %.1f%%", (Double(itr) / Double(test_times)) * 100))
            }
            
            resultsCallBack(prefillStartTimes.first, decodeEndTimes.last)
        }
    }
}
