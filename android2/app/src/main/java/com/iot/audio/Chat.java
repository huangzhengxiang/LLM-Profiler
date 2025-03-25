package com.iot.audio;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;

import java.io.Serializable;
import java.security.interfaces.RSAKey;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.CountDownLatch;

public class Chat implements Serializable {
    private String mEngine;
    private final ArrayList<String> javaEngine = new ArrayList<String>(Arrays.asList("mediapipe", "MLC-LLM"));
    private JavaLLMWrapper javaLLMWrapper;
    public boolean Init(MainActivity activity,
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
        if (javaEngine.contains(mEngine)) {
            // java init.
            javaLLMWrapper = new JavaLLMWrapper(activity,
                                                context,
                                                engineName,
                                                modelDir,
                                                backendName,
                                                tmpFile,
                                                prefillThreadNum,
                                                decodeThreadNum,
                                                prefillPowerMode,
                                                decodePowerMode,
                                                deocdeCorePlan,
                                                tuneTimes);
            System.loadLibrary("dataset_jni");
            return true;
        } else {
            if (engineName.equals("MNN")) {
                System.loadLibrary("mnn_jni");
            } else if (engineName.equals("llama.cpp")) {
                System.loadLibrary("llama_cpp_jni");
            } else if (engineName.equals("mllm")) {
                System.loadLibrary("mllm_jni");
            } else if (engineName.equals("executorch")) {
                System.loadLibrary("et_jni");
            }
            // the rests goes to native init
            return InitNative(engineName,
                    modelDir,
                    backendName,
                    tmpFile,
                    prefillThreadNum,
                    decodeThreadNum,
                    prefillPowerMode,
                    decodePowerMode,
                    deocdeCorePlan,
                    tuneTimes);
        }
    }
    public native boolean InitNative(String engineName,
                                     String modelDir,
                                     String backendName,
                                     String tmpFile,
                                     String prefillThreadNum,
                                     String decodeThreadNum,
                                     String prefillPowerMode,
                                     String decodePowerMode,
                                     String deocdeCorePlan,
                                     String tuneTimes);

    public void tunePrefill() {
        if (!javaEngine.contains(mEngine)){
            tunePrefillNative();
        }
    }
    public native void tunePrefillNative();

    public void startDecodeTune(int tolerance) {
        if (!javaEngine.contains(mEngine)) {
            startDecodeTuneNative(tolerance);
        }
    }
    public native void startDecodeTuneNative(int tolerance);

    public boolean endDecodeTune(ArrayList<Integer> decodeCorePlan, float power, int tolerance) {
        if (!javaEngine.contains(mEngine)) {
            return endDecodeTuneNative(decodeCorePlan, power, tolerance);
        } else {
            return true;
        }
    }
    public native boolean endDecodeTuneNative(ArrayList<Integer> decodeCorePlan, float power, int tolerance);

    public void Trace() {
        if (!javaEngine.contains(mEngine)) {
            TraceNative();
        }
    }
    public native void TraceNative();

    public Bundle Forward(MainActivity activity, int prefill_length, int decode_length) {
        if (javaEngine.contains(mEngine)) {
            Bundle bundle = javaLLMWrapper.Forward(activity, prefill_length, decode_length);
            bundle.putFloat("prefill_time", bundle.getFloat("prefill_time")/bundle.getInt("prefill_len"));
            bundle.putFloat("decode_time", bundle.getFloat("decode_time")/bundle.getInt("decode_len"));
            return bundle;
        } else {
            Bundle bundle = new Bundle();
            activity.startTracing();
            ForwardNative(prefill_length, true, true);
            activity.endTracing();
            bundle.putInt("prefill_len", prefill_length);
            bundle.putInt("prefill_current", activity.getAvgCurrent());
            bundle.putFloat("prefill_power", activity.getAvgPower());
            bundle.putFloat("prefill_time", activity.getTime()/prefill_length);
            bundle.putFloat("prefill_peak_temp", activity.getPeakTemperature());
            bundle.putFloat("prefill_avg_temp", activity.getAvgTemperature());

            activity.startTracing();
            ForwardNative(decode_length, false, false);
            activity.endTracing();
            bundle.putInt("decode_len", decode_length);
            bundle.putInt("decode_current", activity.getAvgCurrent());
            bundle.putFloat("decode_power", activity.getAvgPower());
            bundle.putFloat("decode_time", activity.getTime()/decode_length);
            bundle.putFloat("decode_peak_temp", activity.getPeakTemperature());
            bundle.putFloat("decode_avg_temp", activity.getAvgTemperature());
            return bundle;
        }
    }
    public native void ForwardNative(int length, boolean is_prefill, boolean is_first_prefill);

    public void Reset() {
        if (javaEngine.contains(mEngine)) {
            javaLLMWrapper.Reset();
        } else {
            ResetNative();
        }
    }
    public native void ResetNative();

    public Bundle DatasetTestOnce(MainActivity activity) {
        // init cnv prefill token, time, energy, capacity.
        int prefill_tokens=0, decode_tokens=0;
        float prefill_time=0, decode_time=0;
        float prefill_energy=0, decode_energy=0;
        int prefill_capacity=0, decode_capacity=0;
        float prefill_peak_temp=0, prefill_avg_temp=0;
        float decode_peak_temp=0, decode_avg_temp=0;
        int turns = 0;
        boolean is_first_prefill = true;
        Bundle bundle;
        while (true) {
            boolean gotData = getDialogUser();
            if (!gotData) {
                break;
            }
            turns += 1;
            if (javaEngine.contains(mEngine)) {
                bundle = javaLLMWrapper.testResponse(activity, getDatasetCurrentInput());
                prefill_tokens += bundle.getInt("prefill_len");
                prefill_capacity += bundle.getInt("prefill_current");
                prefill_energy += bundle.getFloat("prefill_power");
                prefill_time += bundle.getFloat("prefill_time");
                prefill_peak_temp += bundle.getFloat("prefill_peak_temp");
                prefill_avg_temp += bundle.getFloat("prefill_avg_temp");
                decode_tokens += bundle.getInt("decode_len");
                decode_capacity += bundle.getInt("decode_current");
                decode_energy += bundle.getFloat("decode_power");
                decode_time += bundle.getFloat("decode_time");
                decode_peak_temp += bundle.getFloat("decode_peak_temp");
                decode_avg_temp += bundle.getFloat("decode_avg_temp");
                gotData = getDialogAssistant();
                if (gotData) {
                    Log.i("DatasetTest Warning: ", "Java LLM can't control output!");
                }
            } else {
                activity.startTracing();
                prefill_tokens += DatasetResponse(true, is_first_prefill);
                activity.endTracing();
                prefill_capacity += activity.getAvgCurrent();
                prefill_energy += activity.getAvgPower();
                prefill_time += activity.getTime();
                prefill_peak_temp += activity.getPeakTemperature();
                prefill_avg_temp += activity.getAvgTemperature();

                is_first_prefill = false;

                gotData = getDialogAssistant();
                if (!gotData) {
                    // free response
                    activity.startTracing();
                    String output = Response("", false); // decode phase only
                    decode_tokens += StringTokenSize(output);
                    activity.endTracing();
                    Log.i("response output", output);
                } else {
                    activity.startTracing();
                    decode_tokens += DatasetResponse(false, false);
                    activity.endTracing();
                }
                decode_capacity += activity.getAvgCurrent();
                decode_energy += activity.getAvgPower();
                decode_time += activity.getTime();
                decode_peak_temp += activity.getPeakTemperature();
                decode_avg_temp += activity.getAvgTemperature();
            }
        }
        bundle = new Bundle();
        bundle.putFloat("prefill_len", (float)prefill_tokens/turns); // tok/turn
        bundle.putInt("prefill_current", prefill_capacity); // uA
        bundle.putFloat("prefill_power", prefill_energy); // mW
        bundle.putFloat("prefill_time", prefill_time/prefill_tokens); // s/tok
        bundle.putFloat("prefill_time_turn", prefill_time/turns); // s/turn
        bundle.putFloat("prefill_peak_temp", prefill_peak_temp/turns);
        bundle.putFloat("prefill_avg_temp", prefill_avg_temp/turns);
        bundle.putFloat("decode_len", (float)decode_tokens/turns); // tok/turn
        bundle.putInt("decode_current",decode_capacity); // uA
        bundle.putFloat("decode_power",decode_energy); // mW
        bundle.putFloat("decode_time", decode_time/decode_tokens); // s/tok
        bundle.putFloat("decode_time_turn", decode_time/turns); // s/turn
        bundle.putFloat("decode_peak_temp", decode_peak_temp/turns);
        bundle.putFloat("decode_avg_temp", decode_avg_temp/turns);
        Log.i("Test Debug: ", String.format("prefill speed: %.4f tok/s", prefill_tokens/prefill_time));
        Log.i("Test Debug: ", String.format("decode speed: %.4f tok/s", decode_tokens/decode_time));
        return bundle;
    }
    public native int loadDataset(String data);
    public native int getDatasetSize();
    public native boolean getDialogUser();
    public native boolean getDialogAssistant();
    public native void datasetNext();
    public native void resetDataset();
    public int DatasetResponse(boolean is_prefill, boolean is_first_prefill) {
        if (javaEngine.contains(mEngine)) {
            String inputStr = getDatasetCurrentInput();
            javaLLMWrapper.Response(inputStr);
            return javaLLMWrapper.countToken(inputStr);
        } else {
            return DatasetResponseNative(is_prefill, is_first_prefill);
        }
    }
    public native int DatasetResponseNative(boolean is_prefill, boolean is_first_prefill);
    private native String getDatasetCurrentInput(); // used only for JavaLLMWrapper.

    public int StringTokenSize(String input) {
        if (javaEngine.contains(mEngine)) {
            return javaLLMWrapper.countToken(input);
        } else {
            return StringTokenSizeNative(input);
        }
    }
    public native int StringTokenSizeNative(String input);

    public String Response(String input, boolean is_first_prefill) {
        if (javaEngine.contains(mEngine)) {
            return javaLLMWrapper.Response(input);
        } else {
            return ResponseNative(input, is_first_prefill);
        }
    }
    public native String ResponseNative(String input, boolean is_first_prefill);
}