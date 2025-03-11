package com.iot.audio;

import android.content.Context;
import android.util.Log;

import java.io.Serializable;
import java.util.ArrayList;

import com.google.common.collect.ImmutableList;
import com.google.mediapipe.tasks.core.ErrorListener;
import com.google.mediapipe.tasks.core.OutputHandler;
import com.google.mediapipe.tasks.genai.llminference.LlmInference;
import com.google.mediapipe.tasks.genai.llminference.LlmInferenceSession;
import com.google.mediapipe.tasks.genai.llminference.VisionModelOptions;

import java.io.File;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

public class MediaPipeWrapper implements Serializable {
    private final Context mContext;
    private final String mModelDir;
    private final String mBackend;
    private LlmInference model;
    public MediaPipeWrapper(Context context,
                            String modelDir,
                            String backendName) {
        mContext = context;
        mModelDir = modelDir;
        mBackend = backendName;
        Init(1024);
    }
    public boolean Init(int maxToken) {
        LlmInference.LlmInferenceOptions options = new LlmInference.LlmInferenceOptions() {
            @Override
            public String modelPath() {
                return mModelDir;
            }

            @Override
            public int maxTokens() {
                return maxToken;
            }

            @Override
            public int maxTopK() {
                return 40;
            }

            @Override
            public List<Integer> supportedLoraRanks() {
                return Collections.emptyList();
            }

            @Override
            public Optional<OutputHandler.ProgressListener<String>> resultListener() {
                return Optional.empty();
            }

            @Override
            public Optional<ErrorListener> errorListener() {
                return Optional.empty();
            }

            @Override
            public Optional<VisionModelOptions> visionModelOptions() {
                return Optional.empty();
            }

            @Override
            public Builder toBuilder() {
                return null;
            }
        };
        try {
            model = LlmInference.createFromOptions(mContext, options);
            String res = model.generateResponse("Hello! What's GPU?");
            Log.i("MediaPipe response:", res);
        } catch (Exception e) {
            Log.i("mediapipe failure", String.format("createFromOptions failed: %s", e.toString()));
            e.printStackTrace();
        }
        return true;
    }
    public void tunePrefill() {}
    public void startDecodeTune(int tolerance) {}
    public boolean endDecodeTune(ArrayList<Integer> decodeCorePlan, float power, int tolerance) {
        return true;
    }
    public void Trace() {}
    public void Forward(int length, boolean is_prefill, boolean is_first_prefill) {}
    public void Reset() {}
}