package com.iot.audio;

import android.content.Context;
import android.os.Bundle;
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
import java.util.concurrent.CountDownLatch;

public class MediaPipeWrapper implements Serializable {
    private CountDownLatch mLatch;
    private final Context mContext;
    private final String mModelDir;
    private final String mBackend;
    private MainActivity mActivity;
    // test profiles
    private int mDecodeLen = 10;
    private int mDecodeToken = 0;
    private Bundle testProfile;
    private String mDecodeResult = "";

    public Bundle getTestProfile() {
        return testProfile;
    }

    public int decodedTokens() {
        return mDecodeToken;
    }

    private LlmInference model;
    private LlmInferenceSession session;
    public MediaPipeWrapper(MainActivity activity,
                            Context context,
                            String modelDir,
                            String backendName) {
        mContext = context;
        mModelDir = modelDir;
        mBackend = backendName;
        mActivity = activity;
        testProfile = new Bundle();
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
                OutputHandler.ProgressListener<String> listener = new OutputHandler.ProgressListener<String>() {
                    @Override
                    public void run(String partialResult, boolean done) {
                        if (mDecodeToken==0) {
                            // prefill starts
                            mActivity.endTracing();
                            testProfile.putInt("prefill_current", mActivity.getAvgCurrent());
                            testProfile.putFloat("prefill_power", mActivity.getAvgPower());
                            testProfile.putFloat("prefill_time", mActivity.getTime());
                            mActivity.startTracing();
                        }
                        if (mDecodeToken==mDecodeLen || done) {
                            // shall finish
                            mActivity.endTracing();
                            testProfile.putInt("decode_current", mActivity.getAvgCurrent());
                            testProfile.putFloat("decode_power", mActivity.getAvgPower());
                            testProfile.putFloat("decode_time", mActivity.getTime());
                        }
                        mDecodeToken += 1;
                        mDecodeResult += partialResult;
                        if (done) {
                            // message the test thread
                            mLatch.countDown();
                        }
                    }
                };
                // Return the listener wrapped in an Optional
                return Optional.of(listener);
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
            session = LlmInferenceSession.createFromOptions(model, LlmInferenceSession.LlmInferenceSessionOptions.builder().build());
        } catch (Exception e) {
            Log.i("mediapipe failure", String.format("createFromOptions failed: %s", e.toString()));
            e.printStackTrace();
        }
        return true;
    }
    public void setDecodeLen(int length) {
        mDecodeLen = length;
    }
    public void Reset() {
        mDecodeToken = 0;
        mDecodeResult = "";
        testProfile.clear();
        // reset the session
        if (session != null) {
            session.close();
        }
        session = LlmInferenceSession.createFromOptions(model, LlmInferenceSession.LlmInferenceSessionOptions.builder().build());
    }
    public String Response(String inputText) {
        mLatch = new CountDownLatch(1);
        session.addQueryChunk(inputText);
        session.generateResponseAsync();
        try {
            // Wait for the latch to be released
            mLatch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        return mDecodeResult;
    }
    public int countToken(String inputText) {
        return session.sizeInTokens(inputText);
    }
}