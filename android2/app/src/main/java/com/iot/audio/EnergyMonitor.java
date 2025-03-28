package com.iot.audio;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.BatteryManager;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.TimerTask;
import java.lang.Math;

public class EnergyMonitor extends TimerTask {
    private final ArrayList<Integer> uAList;
    private final ArrayList<Float> VList;
    private final ArrayList<Float> tempList;
    private final Context ctx;
    private boolean rectified;

    EnergyMonitor(Context parent) {
        uAList = new ArrayList<Integer>();
        VList = new ArrayList<Float>();
        tempList = new ArrayList<Float>();
        ctx = parent;
        rectified = false;
    }

    public void resetInfo() {
        uAList.clear();
        VList.clear();
        tempList.clear();
        rectified = false;
    }

    public void rectifyCapacity() {
        if (rectified) { return; }
        rectified = true;
        long size = uAList.size();
//        for (int i=0; i < size; ++i) {
//            Log.i("MNNJNI", String.format("Current: %d uA", uAList.get(i)));
//        }
        int itr = 1, id = 0;
        int first = uAList.get(0), second = uAList.get(0);
        for (int i = 0; i <= size; ++i) {
            if (itr==1 && uAList.get(i)!=first) {
                second = uAList.get(i);
                if (uAList.get(i)>2*first) {
                    for (int j=id; j<i; ++j) {
                        uAList.set(j, second);
                    }
                }
                itr++; id=i;
            }
            if (itr==2 && uAList.get(i)!=second) {
                if (uAList.get(i)>2*second) {
                    for (int j=id; j<i; ++j) {
                        uAList.set(j, uAList.get(i));
                    }
                }
                break;
            }
        }
//        for (int i=0; i < size; ++i) {
//            Log.i("MNNJNI", String.format("Current: %d uA", uAList.get(i)));
//        }
    }

    public int getAvgCurrent() {
        rectifyCapacity();
        long size = uAList.size();
        long res = 0;
        for (int i=0; i < size; ++i) {
            res += uAList.get(i);
        }
        return  (int)(res/size);
    }

    public float getAvgPower() {
        rectifyCapacity();
        long size = uAList.size();
        double res = 0;
        for (int i=0; i < size; ++i) {
            res += uAList.get(i)*VList.get(i);
        }
        return  (float)(res/size);
    }

    public float getAvgTemperature() {
        long size = tempList.size();
        double res = 0;
        for (int i=0; i < size; ++i) {
            res += tempList.get(i);
        }
        return (float) (res/size);
    }

    public float getPeakTemperature() {
        long size = tempList.size();
        float res = 0;
        for (int i=0; i < size; ++i) {
            if (tempList.get(i)>res) {
                res = tempList.get(i);
            }
        }
        return res;
    }

    @Override
    public void run() {
        int mBatteryCurrent = ((BatteryManager) ctx.getSystemService(Context.BATTERY_SERVICE)).getIntProperty(BatteryManager.BATTERY_PROPERTY_CURRENT_NOW);
        float mBatteryVoltage = (float) ((MainActivity)ctx).getVoltage(); // V
        float mTemperature = ((MainActivity)ctx).getCPUTemperature(); // oC
        if (Math.abs(mBatteryCurrent) <= 10000) { mBatteryCurrent *= 1000; } // convert mA -> uA. (device heterogeneity resolution.)
        // current is default to be positive now!
        uAList.add(Math.abs(mBatteryCurrent));
        VList.add(Math.abs(mBatteryVoltage));
        tempList.add(mTemperature);
    }
}
