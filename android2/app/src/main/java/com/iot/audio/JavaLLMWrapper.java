package com.iot.audio;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Serializable;
import java.nio.file.Paths;
import java.util.Random;
import java.util.concurrent.CountDownLatch;

public class JavaLLMWrapper implements Serializable {
    private String mEngine;
    private MediaPipeWrapper mediaPipeWrapper;
    private String test_prompt;
    JavaLLMWrapper(MainActivity activity,
                   Context context,
                   String engineName,
                   String modelDir,
                   String backendName,
                   String tmpFile,
                   String prefillThreadNum,
                   String decodeThreadNum,
                   String prefillPowerMode,
                   String decodePowerMode,
                   String deocdeCorePlan,
                   String tuneTimes) {
        mEngine = engineName;
        if (engineName.equals("mediapipe")) {
            mediaPipeWrapper = new MediaPipeWrapper(activity, context, modelDir, backendName);
        }
        try {
            InputStream stream = activity.getAssets().open(Paths.get("story", "story.txt").toString());
            BufferedReader reader = new BufferedReader(new InputStreamReader(stream));
            StringBuilder stringBuilder = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                stringBuilder.append(line).append("\n");
            }
            test_prompt = stringBuilder.toString();
        } catch (IOException e) {
            Log.e("JavaLLMWrapper Error", "JavaLLMWrapper Error: can't fill test prompt!");
        }
    }
    public Bundle Forward(MainActivity activity, int prefill_length, int decode_length) {
        setDecodeLen(decode_length);
        String inputStr = getFixedLengthInput(prefill_length);
        Log.i("java input", inputStr);
        activity.startTracing();
        String res = Response(inputStr);
        Log.i("java output", res);
        if (mEngine.equals("mediapipe")) {
            if (mediaPipeWrapper.decodedTokens()<decode_length) {
                Log.i("Mediapipe Warning", String.format("Warning: decode only %d tokens", mediaPipeWrapper.decodedTokens()));
            }
            Bundle bundle = mediaPipeWrapper.getTestProfile();
            Log.i("Mediapipe Warning", String.format("prefill time: %.4f", bundle.getFloat("prefill_time")));
            Log.i("Mediapipe Warning", String.format("decode time: %.4f", bundle.getFloat("decode_time")));
            bundle.putInt("prefill_len", prefill_length);
            return bundle;
        }
        return null;
    }
    public Bundle testResponse(MainActivity activity, String inputStr) {
        setDecodeLen(1024); // set decode length to max: 1024
        String res = Response(inputStr);
        Log.i("java output", res);
        if (mEngine.equals("mediapipe")) {
            Bundle bundle = mediaPipeWrapper.getTestProfile();
            Log.i("Mediapipe Warning", String.format("prefill time: %.4f", bundle.getFloat("prefill_time")));
            Log.i("Mediapipe Warning", String.format("decode time: %.4f", bundle.getFloat("decode_time")));
            bundle.putInt("prefill_len", countToken(inputStr));
            return bundle;
        }
        return null;
    }

    public String getFixedLengthInput(int length) {
        String[] words = test_prompt.split("\\s+");
        StringBuilder result = new StringBuilder();
        int start = (int)(Math.random()*300);
        for (String word : words) {
            if (start>0) {start--;continue;}
            // Check if adding the next word would exceed the maximum length
            if (countToken(result.toString()) < length) {
                // Append the word to the result
                if (result.length() > 0) {
                    result.append(" ");
                }
                result.append(word);
            } else {
                break;
            }
        }
        return result.toString();
    }
    public void setDecodeLen(int length) {
        if (mEngine.equals("mediapipe")) {
            mediaPipeWrapper.setDecodeLen(length);
        }
    }
    public String Response(String inputText) {
        if (mEngine.equals("mediapipe")) {
            return mediaPipeWrapper.Response(inputText);
        }
        return "";
    }
    public int countToken(String inputText) {
        if (mEngine.equals("mediapipe")) {
            return mediaPipeWrapper.countToken(inputText);
        }
        return 0;
    }
    public int getCurrentKVLen() {
        if (mEngine.equals("mediapipe")) {
            return mediaPipeWrapper.getCurrentKVLen();
        }
        return 0;
    }
    public int getMaxKVLen() {
        if (mEngine.equals("mediapipe")) {
            return mediaPipeWrapper.getMaxKVLen();
        }
        return 1024;
    }
    public void Reset() {
        if (mEngine.equals("mediapipe")) {
            mediaPipeWrapper.Reset();
        }
    }
}
