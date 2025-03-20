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
            // need reinit if the decode length is fixed at another length.
            // merge such reinit into Forward.
            return true;
        } else {
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
            return javaLLMWrapper.Forward(activity, prefill_length, decode_length);
        } else {
            Bundle data = new Bundle();
            activity.startTracing();
            ForwardNative(prefill_length, true, true);
            activity.endTracing();
            data.putInt("prefill_len", prefill_length);
            data.putInt("prefill_current", activity.getAvgCurrent());
            data.putFloat("prefill_power", activity.getAvgPower());
            data.putFloat("prefill_time", activity.getTime());

            activity.startTracing();
            ForwardNative(decode_length, false, false);
            activity.endTracing();
            data.putInt("decode_len", decode_length);
            data.putInt("decode_current", activity.getAvgCurrent());
            data.putFloat("decode_power", activity.getAvgPower());
            data.putFloat("decode_time", activity.getTime());
            return data;
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

    public native int loadDataset(String data);
    public native int getDatasetSize();
    public native boolean getDialogUser();
    public native boolean getDialogAssistant();
    public native void datasetNext();
    public native void resetDataset();
    public native int DatasetResponse(boolean is_prefill, boolean is_first_prefill);

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

    static {
        System.loadLibrary("iot");
    }
}