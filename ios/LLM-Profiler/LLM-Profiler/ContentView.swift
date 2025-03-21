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
    
    // State variables for text fields
    @State private var textField1 = ""
    @State private var textField2 = ""
    @State private var textField3 = ""
    @State private var textField4 = ""
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
                    TextField("prefill thread", text: $textField1)
                        .textFieldStyle(.roundedBorder)
                    
                    TextField("decode thread", text: $textField2)
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
                    TextField("prefill cores", text: $textField3)
                        .textFieldStyle(.roundedBorder)
                    
                    TextField("decode cores", text: $textField4)
                        .textFieldStyle(.roundedBorder)
                }
                
                Rectangle().fill(Color.black).frame(height:  1).padding(.vertical, 5).frame(maxWidth: .infinity)
                
                HStack {
                    // Buttons
                    Button(action: {
                        // Action for Button 1
                        print("LoadModel tapped")
                        loadModel();
                    }) {
                        Text("Load Model")
                            .frame(maxWidth: .infinity)
                            .padding()
                            .background(Color.purple).font(.headline)
                            .foregroundColor(.white)
                            .cornerRadius(10)
                    }
                    
                    Button(action: {
                        // Action for Button 2
                        print("Button 2 tapped")
                    }) {
                        Text("start test")
                            .frame(maxWidth: .infinity)
                            .padding()
                            .background(Color.purple).font(.headline)
                            .foregroundColor(.white)
                            .cornerRadius(10)
                    }
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
            chat.setupLLM(modelPath: localModelPath, completion: setStatus)
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
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
#Preview {
    ContentView()
}
