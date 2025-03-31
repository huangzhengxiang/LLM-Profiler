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
                completion("model is loaded")
            }
        }
    }
    
    func FixedLengthTestRun(prefill_len: Int,
                            decode_len: Int,
                            test_times: Int,
                            callback: @escaping () -> Void) {
        Task { @MainActor in
            
            var prefillStartTimes: [CFAbsoluteTime] = []
            var prefillEndTimes: [CFAbsoluteTime] = []
            var decodeStartTimes: [CFAbsoluteTime] = []
            var decodeEndTimes: [CFAbsoluteTime] = []

            // Define the loop count
            let test_times = 200  // Adjust this value as needed

            // Loop to execute the code repeatedly
            for _ in 0..<test_times {
                
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
                try? await Task.sleep(nanoseconds: 5_000_000_000)  // 3 seconds in nanoseconds
            }
            
            callback();
        }
    }
}
