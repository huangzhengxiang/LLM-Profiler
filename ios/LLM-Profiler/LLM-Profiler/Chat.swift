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
}
