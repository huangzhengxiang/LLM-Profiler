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
    private final ArrayList<Integer> mAList;
    private final Context ctx;

    EnergyMonitor(Context parent) {
        mAList = new ArrayList<Integer>();
        ctx = parent;
    }

    public void resetInfo() {
        mAList.clear();
    }

    public int getAvgCurrent() {
        long size = mAList.size();
        long res = 0;
        for (int i=0; i < size; ++i) {
            res += mAList.get(i);
        }
        return  (int)(res/size);
    }

    @Override
    public void run() {
        int mBatteryCurrent = ((BatteryManager) ctx.getSystemService(Context.BATTERY_SERVICE)).getIntProperty(BatteryManager.BATTERY_PROPERTY_CURRENT_NOW);
        if (Math.abs(mBatteryCurrent) <= 10000) { mBatteryCurrent *= 1000; Log.i("Energy", String.format("%d", mBatteryCurrent)); } // convert mA -> uA. (device heterogeneity resolution.)
        mAList.add(mBatteryCurrent);
    }
}
