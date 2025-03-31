//
//  ContentView.swift
//  LLM-Profiler
//
//  Created by zhengxiang huang on 2025/3/9.
//

import SwiftUI

struct ContentView: View {
    // Chat
    @StateObject private var chat = Chat()
    
    // State variables for dropdown selections
    @State private var engineSelector = "select engine"
    @State private var modelSelector = "select model"
    @State private var backendSelector = "select backend"
    @State private var prefillPowerMode = "prefill power"
    @State private var decodePowerMode = "decode power"
    @State private var Load = false
    @State private var Testing = false
    
    // State variables for text fields
    @State private var prefillThreadNum = ""
    @State private var decodeThreadNum = ""
    @State private var prefillCorePlan = ""
    @State private var decodeCorePlan = ""
    @State private var tuneTimes = ""
    @State private var decodeTol = ""
    @State private var prefillLen = ""
    @State private var decodeLen = ""
    @State private var testTimes = ""
    @State private var modelOptions: [String] = [] // List of files and directories
    
    let engineOptions = ["select engine", "MNN", "llama.cpp", "mediapipe", "mllm", "executorch"]
    let backendOptions = ["select backend", "CPU", "GPU", "NPU"]  
    let prefillPowerOptions = ["prefill power", "(default)", "normal", "high", "memory", "tune_prefill", "exhaustive"]
    let decodePowerOptions = ["decode power", "(default)", "normal", "high", "memory", "tune_prefill", "exhaustive"]
    
    // Text displays
    @State private var statusText: String = "Status"
    @State private var warningText: String = "Warning"

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 20) {
                
                Text("LLM-Profiler").font(.largeTitle).fontWeight(.semibold).multilineTextAlignment(.center).lineLimit(nil).padding(.all, 10.0).background(Color.purple).foregroundColor(.white).frame(maxWidth: .infinity)
                
                Rectangle().fill(Color.black).frame(height:  1).padding(.vertical, 5)
                
                // Dropdown Selection Boxes
                Picker("Select Engine", selection: $engineSelector) {
                    ForEach(engineOptions, id: \.self) {
                        Text($0)
                    }
                }
                .pickerStyle(.menu).frame(maxWidth: .infinity).padding(.horizontal, 20)
                
                Picker("Select Model", selection: $modelSelector) {
                    ForEach(modelOptions, id: \.self) {
                        Text($0)
                    }
                }
                .pickerStyle(.menu).frame(maxWidth: .infinity).padding(.horizontal, 20).onAppear {
                    populateModels()
                }
                
                Picker("Select Backend", selection: $backendSelector) {
                    ForEach(backendOptions, id: \.self) {
                        Text($0)
                    }
                }
                .pickerStyle(.menu).frame(maxWidth: .infinity).padding(.horizontal, 20)
                
                // Fill-in Blanks
                HStack {
                    TextField("prefill thread", text: $prefillThreadNum)
                        .textFieldStyle(.roundedBorder)
                    
                    TextField("decode thread", text: $decodeThreadNum)
                        .textFieldStyle(.roundedBorder)
                }
                
                HStack {
                    Picker("Select Prefill Power", selection: $prefillPowerMode) {
                        ForEach(prefillPowerOptions, id: \.self) {
                            Text($0)
                        }
                    }
                    .pickerStyle(.menu).frame(maxWidth: .infinity)
                    
                    Picker("Select Decode Power", selection: $decodePowerMode) {
                        ForEach(decodePowerOptions
                                , id: \.self) {
                            Text($0)
                        }
                    }
                    .pickerStyle(.menu).frame(maxWidth: .infinity)
                }
                
                
                HStack {
                    TextField("prefill cores", text: $prefillCorePlan)
                        .textFieldStyle(.roundedBorder)
                    
                    TextField("decode cores", text: $decodeCorePlan)
                        .textFieldStyle(.roundedBorder)
                }
                
                HStack {
                    TextField("tune times", text: $tuneTimes)
                        .textFieldStyle(.roundedBorder)
                    
                    TextField("decode tol", text: $decodeTol)
                        .textFieldStyle(.roundedBorder)
                }
                
                
                Rectangle().fill(Color.black).frame(height:  1).padding(.vertical, 5).frame(maxWidth: .infinity)
                
                
                HStack {
                    TextField("prefill_len", text: $prefillLen)
                        .textFieldStyle(.roundedBorder)
                    
                    TextField("decode_len", text: $decodeLen)
                        .textFieldStyle(.roundedBorder)
                }
                
                TextField("test times", text: $testTimes)
                    .textFieldStyle(.roundedBorder)
                
                
                Rectangle().fill(Color.black).frame(height:  1).padding(.vertical, 5).frame(maxWidth: .infinity)
                
                HStack {
                    // Buttons
                    Button(action: {
                        // Action for Button 1
                        print("LoadModel tapped")
                        loadModel();
                        Load = true
                    }) {
                        Text("Load Model")
                            .frame(maxWidth: .infinity)
                            .padding()
                            .background(Color.purple).font(.headline)
                            .foregroundColor(.white)
                            .cornerRadius(10)
                    }.disabled(Load)
                    
                    Button(action: {
                        // Action for Button 2
                        print("start test")
                        let prefill_len = Int(prefillLen) ?? 64
                        let decode_len = Int(decodeLen) ?? 128
                        let test_times = Int(testTimes) ?? 100
                        FixedLengthTestRun(prefill_len: prefill_len, decode_len: decode_len, test_times: test_times)
                        Testing = true
                    }) {
                        Text("start test")
                            .frame(maxWidth: .infinity)
                            .padding()
                            .background(Color.purple).font(.headline)
                            .foregroundColor(.white)
                            .cornerRadius(10)
                    }.disabled(Testing)
                }

                Text(statusText).frame(maxWidth: .infinity).padding(.horizontal, 10)

                Text(warningText).frame(maxWidth: .infinity).padding(.horizontal, 10)
            }
            .padding()
        }
        .navigationTitle("Scrollable Form")
    }

    func setStatus(text: String) {
        statusText = "Status: " + text;
    }

    func setWarning(text: String) {
        warningText = "Warning: " + text;
    }

    func loadModel() {
        // Get the main bundle's resource path
        if let resourcePath = Bundle.main.resourcePath {
            let localModelPath = URL(fileURLWithPath: resourcePath).appendingPathComponent("LocalModel").appendingPathComponent(modelSelector).path()
            // Call setupLLM on the chat instance
            chat.setupLLM(engineName: engineSelector,
                          modelPath: localModelPath,
                          backendName: backendSelector,
                          tmpFile: FileManager.default.temporaryDirectory.path,
                          prefillThreadNum: prefillThreadNum,
                          decodeThreadNum: decodeThreadNum,
                          prefillPowerMode: prefillPowerMode,
                          decodePowerMode: decodePowerMode,
                          decodeCorePlan: decodeCorePlan,
                          tuneTimes: tuneTimes,
                          completion: setStatus)
        } else {
            print("Resource path not found")
        }
    }
    

    func populateModels() {
        // Get the main bundle's resource path
        if let resourcePath = Bundle.main.resourcePath {
            let localModelPath = URL(fileURLWithPath: resourcePath).appendingPathComponent("LocalModel").path
            do {
                // Get the contents of the directory
                let contents = try FileManager.default.contentsOfDirectory(atPath: localModelPath)
                // Update the options state variables
                modelOptions = contents
                modelOptions.insert("select model", at: 0)
            } catch {
                print("Error reading directory contents: \(error)")
            }
        } else {
            print("Resource path not found")
        }
    }
    
    func FixedLengthTestRun(prefill_len: Int, decode_len: Int, test_times: Int) {
        chat.FixedLengthTestRun(prefill_len: prefill_len, decode_len: decode_len, test_times: test_times, callback: saveTimeStamp)
    }
    
    func saveTimeStamp() {
        print("test finished!\n")
        Testing = false
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
#Preview {
    ContentView()
}
