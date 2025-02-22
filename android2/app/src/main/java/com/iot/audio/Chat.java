package com.iot.audio;

import java.io.Serializable;
import java.util.ArrayList;

public class Chat implements Serializable {
    public native boolean Init(String engineName, 
                               String modelDir,
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

    static {
        System.loadLibrary("iot");
    }
}