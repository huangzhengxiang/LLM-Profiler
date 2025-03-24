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

    EnergyMonitor(Context parent) {
        uAList = new ArrayList<Integer>();
        VList = new ArrayList<Float>();
        tempList = new ArrayList<Float>();
        ctx = parent;
    }

    public void resetInfo() {
        uAList.clear();
        VList.clear();
        tempList.clear();
    }

    public int getAvgCurrent() {
        long size = uAList.size();
        long res = 0;
        for (int i=0; i < size; ++i) {
            res += uAList.get(i);
        }
        return  (int)(res/size);
    }

    public float getAvgPower() {
        long size = uAList.size();
        double res = 0;
        for (int i=0; i < size; ++i) {
            res += uAList.get(i)*VList.get(i);
        }
        return  (float)(res/size);
    }

    @Override
    public void run() {
        int mBatteryCurrent = ((BatteryManager) ctx.getSystemService(Context.BATTERY_SERVICE)).getIntProperty(BatteryManager.BATTERY_PROPERTY_CURRENT_NOW);
        float mBatteryVoltage = (float) ((MainActivity)ctx).getVoltage(); // V
        if (Math.abs(mBatteryCurrent) <= 10000) { mBatteryCurrent *= 1000; } // convert mA -> uA. (device heterogeneity resolution.)
        // current is default to be positive now!
        uAList.add(Math.abs(mBatteryCurrent));
        VList.add(Math.abs(mBatteryVoltage));
    }
}
