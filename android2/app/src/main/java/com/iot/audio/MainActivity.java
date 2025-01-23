package com.iot.audio;
import android.content.pm.PackageManager;
import android.graphics.Interpolator;
import android.media.AudioFormat;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Bundle;
import android.os.BatteryManager;
import android.content.Context;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Button;
import android.graphics.Color;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.content.ContentValues;
import android.provider.MediaStore;


import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

import static android.Manifest.permission.RECORD_AUDIO;
import static android.Manifest.permission.WRITE_EXTERNAL_STORAGE;
import static android.os.SystemClock.elapsedRealtime;

public class MainActivity extends AppCompatActivity {
    // <energy profile begin
    private Timer energyTimer;
    private EnergyMonitor energyMonitor;
    private final long energySampleInterval = 100L; // 100ms
    // energy profile end>

    // <time profile begin
    private long mElapsedBegin;
    private long mElapsedEnd;
    // time profile end>

    // <file and path begin
    private File recordDir;
    private File tmpDir;
    private String recordFilePath;
    // file and path end>

    // <media record begin
//    private final String systemPrompt = "<|im_start|>system\nYou are a helpful assistant.<|im_end|>\n";
    private wavClass mRecorder;
    private MediaPlayer mPlayer;

    // constant for storing audio permission
    public static final int REQUEST_AUDIO_PERMISSION_CODE = 1;
    // media record end>

    // <view begin
    private EditText modelPathTV, configNameTV, threadNumTV, powerModeTV, prefillLenTV, decodeLenTV;
    private TextView prefillSpeedTV, prefillEnergyTV, decodeSpeedTV, decodeEnergyTV, statusTV;
    private Button mLoadButton, testButton;
    private Handler mHandler;
    // view end>

    // <LLM model begin
    private Chat mChat;
    private final String mSearchPath = "/data/local/tmp/llm/model/";
    private String mModelName = "qwen2_5-1_5b-instruct-mnn";
    private String mConfigName = "/config.json";
    private String mModelDir = mSearchPath + mModelName + mConfigName;
    // LLM model end>

    // <model profiling config begin
    private int prefill_len = 100;
    private int decode_len = 200;
    private float prefill_token_speed = -1f; // tok/s
    private float decode_token_speed = -1f; // tok/s
    private float prefill_capacity = -1f; // tok/mAh
    private float decode_capacity = -1f; // tok/mAh
    private final int test_times = 3;
    // model profiling config end>

    private void startEnergyTracing() {
        energyTimer = new Timer();
        energyMonitor = new EnergyMonitor(this);
        energyMonitor.resetInfo();
        energyTimer.scheduleAtFixedRate(energyMonitor, 0, energySampleInterval);
    }

    private void endEnergyTracing() {
        try {
            energyTimer.cancel();
        } catch (Exception e) {
            Log.e("endEnergyTracing", e.getMessage());
        }
    }

    private int getAvgCurrent() {
        return energyMonitor.getAvgCurrent();
    }

    private void startTimeTracing() {
        mElapsedBegin = elapsedRealtime();
    }

    private void endTimeTracing() {
        mElapsedEnd = elapsedRealtime();
    }

    private float getTime() {
        return ((float)(mElapsedEnd-mElapsedBegin))/1000f; // convert from ms to s.
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // initialize all variables with their layout items.
        mLoadButton = findViewById(R.id.load_button);
        statusTV = findViewById(R.id.idTVstatus);
        prefillSpeedTV = findViewById(R.id.PrefillSpeed);
        prefillEnergyTV = findViewById(R.id.PrefillEnergy);
        decodeSpeedTV = findViewById(R.id.DecodeSpeed);
        decodeEnergyTV = findViewById(R.id.DecodeEnergy);
        testButton = findViewById(R.id.startTest);
        modelPathTV = findViewById(R.id.modelPath);
        configNameTV = findViewById(R.id.configName);
        threadNumTV = findViewById(R.id.threadNum);
        powerModeTV = findViewById(R.id.powerMode);
        prefillLenTV = findViewById(R.id.prefillLen);
        decodeLenTV = findViewById(R.id.decodeLen);
        modelPathTV.setText(mModelName);
        configNameTV.setText(mConfigName);
        prefillSpeedTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
        prefillEnergyTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
        decodeSpeedTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
        decodeEnergyTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
        testButton.setBackgroundColor(getResources().getColor(R.color.purple_200));
        testButton.setClickable(false);
        recordDir = getExternalFilesDir("Recordings");
        if (!recordDir.exists()) {
            recordDir.mkdirs();
        }
        tmpDir = getExternalFilesDir("tmp");
        if (!tmpDir.exists()) {
            tmpDir.mkdirs();
        }
        mHandler = new Handler(Looper.getMainLooper()) {
            @Override
            public void handleMessage(Message message) {
                if (message.getData().getString("call").equals("testRun")) {
                    testButton.setBackgroundColor(getResources().getColor(R.color.purple_200));
                    testButton.setClickable(true);
                    prefillSpeedTV.setText(String.format("prefill speed:\n %.4f tok/s", message.getData().getFloat("prefill_token_speed")));
                    prefillEnergyTV.setText(String.format("prefill energy:\n %.4f uAh/tok", message.getData().getFloat("prefill_capacity")));
                    decodeSpeedTV.setText(String.format("decode speed:\n %.4f tok/s", message.getData().getFloat("decode_token_speed")));
                    decodeEnergyTV.setText(String.format("decode energy:\n %.4f uAh/tok", message.getData().getFloat("decode_capacity")));
                    statusTV.setText("Test Finished!");
                } else if (message.getData().getString("call").equals("loadModel")) {
                    statusTV.setText("模型加载完成！");
                    mLoadButton.setText("模型已加载");
                    testButton.setClickable(true);
                }
            }
        };
//        IntentFilter ifilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
//        Intent batteryStatus = this.registerReceiver(null, ifilter);
//        int level = batteryStatus.getIntExtra(, -1);
//        Log.i("Battery", String.format("System Battery: %d", level));
//        Log.i("Current", String.format( "Battery Current: %d", ((BatteryManager) getSystemService(Context.BATTERY_SERVICE)).getIntProperty(BatteryManager.BATTERY_PROPERTY_CURRENT_NOW)));
//        mAlarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
//        mAlarmIntent = new Intent(this, EnergyMonitor.class);

//        startTV.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v)  {
//                // start recording method will
//                // start the recording of audio.
//                try {
//                    startRecording();
//                } catch (Exception e){
//                    Log.e("recording didn't start!", e.getMessage());
//                }
//            }
//        });
//        stopTV.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                // pause Recording method will
//                // pause the recording of audio.
//                pauseRecording();
//
//            }
//        });
    }
    private boolean checkModelsReady() {
        File dir = new File(mModelDir);
        return dir.exists();
    }

    private void onCheckModels() {
        boolean modelReady = checkModelsReady();

        statusTV.setText(String.format("%s加载中", mModelName));
        mLoadButton.setText("模型加载中");

    }

    public void loadModel(View view) {
        onCheckModels();
        mLoadButton.setClickable(false);
        mLoadButton.setBackgroundColor(Color.parseColor("#2454e4"));
        mLoadButton.setText("模型加载中 ...");

        mModelName = modelPathTV.getText().toString();
        mConfigName = configNameTV.getText().toString();
        mModelDir = mSearchPath + mModelName + mConfigName;
        Log.i("LLM Model Path", mModelDir);

        new Thread(() -> {
            mChat = new Chat();
            mChat.Init(mModelDir, tmpDir.getPath(), threadNumTV.getText().toString(), powerModeTV.getText().toString());
            Message message=new Message();
            Bundle data=new Bundle();
            data.putString("call", "loadModel");
            message.setData(data);
            mHandler.sendMessage(message);
        }).start();
    }

//    private void startRecording() throws IOException {
//        // check permission method is used to check
//        // that the user has granted permission
//        // to record and store the audio.
//        if (CheckPermissions()) {
//
//            // setbackgroundcolor method will change
//            // the background color of text view.
//            stopTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
//            startTV.setBackgroundColor(getResources().getColor(R.color.gray));
//
//
//            mRecorder = new wavClass(recordDir);
//            recordFilePath = mRecorder.getWavFilePath();
//            mRecorder.startRecording();
//            statusTV.setText("Recording Started");
//        } else {
//            // if audio recording permissions are
//            // not granted by user below method will
//            // ask for runtime permission for mic and storage.
//            RequestPermissions();
//        }
//    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        // this method is called when user will
        // grant the permission for audio recording.
        switch (requestCode) {
            case REQUEST_AUDIO_PERMISSION_CODE:
                if (grantResults.length > 0) {
                    boolean permissionToRecord = grantResults[0] == PackageManager.PERMISSION_GRANTED;
                    boolean permissionToStore = grantResults[1] == PackageManager.PERMISSION_GRANTED;
                    if (permissionToRecord) {
                        Toast.makeText(getApplicationContext(), "Permission Granted", Toast.LENGTH_LONG).show();
                    } else {
                        Toast.makeText(getApplicationContext(), "Permission Denied", Toast.LENGTH_LONG).show();
                    }
                }
                break;
        }
    }

    public boolean CheckPermissions() {
        // this method is used to check permission
        int result = ContextCompat.checkSelfPermission(getApplicationContext(), WRITE_EXTERNAL_STORAGE);
        int result1 = ContextCompat.checkSelfPermission(getApplicationContext(), RECORD_AUDIO);
        return result == PackageManager.PERMISSION_GRANTED && result1 == PackageManager.PERMISSION_GRANTED;
    }

    private void RequestPermissions() {
        // this method is used to request the
        // permission for audio recording and storage.
        ActivityCompat.requestPermissions(MainActivity.this, new String[]{RECORD_AUDIO, WRITE_EXTERNAL_STORAGE}, REQUEST_AUDIO_PERMISSION_CODE);
    }

    public void pauseRecording() {
        // below method will stop
        // the audio recording.
        mRecorder.stopRecording();
        mRecorder = null;
        statusTV.setText("Recording Stopped");

        mChat.Submit("");
        byte[] ret = mChat.Response();
        Log.i("LLM", String.format("response len: %d, response content: %s", ret.length, new String(ret)));
        String reply = new String(ret);
    }


    public float avgIntArray(ArrayList<Integer> arrayList) {
        int res = 0;
        for (int i=0; i<arrayList.size(); ++i) {
            res += arrayList.get(i);
        }
        return (float)res/arrayList.size();
    }

    public float avgFloatArray(ArrayList<Float> arrayList) {
        float res = 0f;
        for (int i=0; i<arrayList.size(); ++i) {
            res += arrayList.get(i);
        }
        return res/arrayList.size();
    }

    public float getAvgEnergyInmAh(ArrayList<Integer> mAList, ArrayList<Float> sList) {
        float res = 0f;
        for (int i=0; i<mAList.size(); ++i) {
            res += mAList.get(i)*sList.get(i)/3600f; // turn second into hour.
        }
        return res/mAList.size();
    }

    public void testRun(View v) {
        testButton.setBackgroundColor(getResources().getColor(R.color.gray));
        testButton.setClickable(false);
        statusTV.setText("Testing...");

        // get prefill len and decode len
        try {
            prefill_len = Integer.parseInt(prefillLenTV.getText().toString());
        }
        catch (NumberFormatException e) {
            prefill_len = 100;
            statusTV.setText("Warning: prefill len shall be int");
        }
        try {
            decode_len = Integer.parseInt(decodeLenTV.getText().toString());
        } catch (NumberFormatException e) {
            decode_len = 200;
            statusTV.setText("Warning: decode len shall be int");
        }


        new Thread(() -> {
            // tracing
            mChat.Trace();
            ArrayList<Integer> mAPrefillList = new ArrayList<Integer>(); // in milli-ampere
            ArrayList<Integer> mADecodeList = new ArrayList<Integer>(); // in milli-ampere
            ArrayList<Float> timePrefillList = new ArrayList<Float>(); // in s
            ArrayList<Float> timeDecodeList = new ArrayList<Float>(); // in s
            for (int i = 0; i < test_times; ++i) {
                startEnergyTracing();
                startTimeTracing();
                mChat.Forward(prefill_len, true);
                endTimeTracing();
                endEnergyTracing();
                mAPrefillList.add(getAvgCurrent() / 1000);
                timePrefillList.add(getTime());

                startEnergyTracing();
                startTimeTracing();
                mChat.Forward(decode_len, false);
                endTimeTracing();
                endEnergyTracing();
                mADecodeList.add(getAvgCurrent() / 1000);
                timeDecodeList.add(getTime());

                mChat.Reset();
                mChat.Done();
            }
            prefill_token_speed = prefill_len / avgFloatArray(timePrefillList);
            decode_token_speed = decode_len / avgFloatArray(timeDecodeList);
            Log.i("debug", String.format("%.4f", avgFloatArray(timeDecodeList)));
            prefill_capacity = -getAvgEnergyInmAh(mAPrefillList, timePrefillList) * 1000 / prefill_len; // negate it, because it's doomed to be negative.
            decode_capacity = -getAvgEnergyInmAh(mADecodeList, timeDecodeList) * 1000 / decode_len; // negate it, because it's doomed to be negative.
            Log.i("prefill", String.format("prefill speed: %.4f tok/s", prefill_token_speed));
            Log.i("prefill", String.format("prefill energy: %.4f uAh/tok", prefill_capacity));
            Log.i("decode", String.format("decode speed: %.4f tok/s", decode_token_speed));
            Log.i("decode", String.format("decode energy: %.4f uAh/tok", decode_capacity));
            Message message=new Message();
            Bundle data=new Bundle();
            data.putFloat("prefill_token_speed", prefill_token_speed);
            data.putFloat("prefill_capacity", prefill_capacity);
            data.putFloat("decode_token_speed", decode_token_speed);
            data.putFloat("decode_capacity", decode_capacity);
            data.putString("call", "testRun");
            message.setData(data);
            mHandler.sendMessage(message);
        }).start();
    }
}