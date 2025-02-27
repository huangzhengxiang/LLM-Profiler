package com.iot.audio;

import java.io.Serializable;
import java.util.ArrayList;

public class Chat implements Serializable {
    public native boolean Init(String engineName, 
                               String modelDir,
                               String backendName,
                               String tmpFile,
                               String prefillThreadNum,
                               String decodeThreadNum,
                               String prefillPowerMode,
                               String decodePowerMode,
                               String deocdeCorePlan,
                               String tuneTimes);
    public native void tunePrefill();
    public native void startDecodeTune(int tolerance);
    public native boolean endDecodeTune(ArrayList<Integer> decodeCorePlan, float power, int tolerance);
    public native void Trace();
    public native void Forward(int length, boolean is_prefill, boolean is_first_prefill);
    public native void Reset();

    public native int loadDataset(String data);
    public native int getDatasetSize();
    public native boolean getDialogUser();
    public native boolean getDialogAssistant();
    public native void datasetNext();
    public native void resetDataset();
    public native int DatasetResponse(boolean is_prefill, boolean is_first_prefill);
    public native int StringTokenSize(String input);

    public native String Response(String input, boolean is_first_prefill);

    static {
        System.loadLibrary("iot");
    }
}