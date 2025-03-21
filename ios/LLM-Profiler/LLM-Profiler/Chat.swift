// swift counterpart of Chat.java

class Chat: ObservableObject {
    private var llm: LLMWrapper?
    @Published var isModelLoaded = false
    init() {}
    func setupLLM(modelPath: String, completion: @escaping (String) -> Void) {
        llm = LLMWrapper(modelPath: modelPath) { [weak self] success in
            Task { @MainActor in
                self?.isModelLoaded = success
                completion("model is loaded")
            }
        }
    }
}
