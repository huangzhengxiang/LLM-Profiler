package com.iot.audio;

import android.content.Context;

import java.io.Serializable;
import java.util.ArrayList;

public class Chat implements Serializable {
    private MediaPipeWrapper mediaPipeWrapper;
    public boolean Init(Context context,
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
        if (engineName.equals("mediapipe")) {
            // mediapipe goes to java init.
            mediaPipeWrapper = new MediaPipeWrapper(context,
                                                    modelDir,
                                                    backendName);
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