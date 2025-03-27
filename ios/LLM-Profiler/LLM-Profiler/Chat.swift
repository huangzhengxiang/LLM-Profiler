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
                         callback: @escaping () -> Void) {
        Task { @MainActor in
            let prefillStartTime = CFAbsoluteTimeGetCurrent()
            llm?.forward(Int32(prefill_len), is_prefill: true, is_first_prefill: true)
            let prefillEndTime = CFAbsoluteTimeGetCurrent()
            let decodeStartTime = CFAbsoluteTimeGetCurrent()
            llm?.forward(Int32(decode_len), is_prefill: false, is_first_prefill: false)
            let decodeEndTime = CFAbsoluteTimeGetCurrent()
            llm?.reset()
            callback();
        }
    }
}
