package com.iot.audio;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Interpolator;
import android.media.AudioFormat;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Bundle;
import android.os.BatteryManager;
import android.os.HardwarePropertiesManager;
import android.content.Context;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Button;
import android.graphics.Color;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.content.ContentValues;
import android.provider.MediaStore;


import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;
import java.nio.file.Paths;

import static android.Manifest.permission.RECORD_AUDIO;
import static android.Manifest.permission.WRITE_EXTERNAL_STORAGE;
import static android.os.SystemClock.elapsedRealtime;

public class MainActivity extends AppCompatActivity {
    // <energy profile begin
    private Timer energyTimer;
    private EnergyMonitor energyMonitor;
    private final long energySampleInterval = 50L; // 50ms
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
    private Spinner mEngineSelectSpinner, mModelSelectSpinner, mBackendSelectSpinner, mPrefillPowerSelectSpinner, mDecodePowerSelectSpinner;
    private Spinner mTestModeSpinner, mDatasetSpinner;
    private LinearLayout FixedLengthTestLayout, DatasetTestLayout;
    private EditText prefillThreadNumTV, decodeThreadNumTV, decodeCorePlanInputTV,  prefillLenTV, decodeLenTV;
    private EditText tuneTimesTV, toleranceTV;
    private TextView prefillSpeedTV, prefillBatteryTV, prefillEnergyTV, decodeSpeedTV, decodeBatteryTV, decodeEnergyTV, statusTV, decodeCorePlanTV;
    private Button mLoadButton, testButton;
    private Handler mHandler;
    // view end>

    // <LLM model begin
    private Chat mChat;
    private String mEngineName, mBackendName, mBackend;
    private final String mSearchPath = "/data/local/tmp/llm/model/";
    private String mModelName = "qwen2_5-1_5b-instruct-int4";
    private String mConfigName = "/config.json";
    private String mModelDir = mSearchPath + mModelName + mConfigName;
    // LLM model end>

    // <model profiling config begin
    private int prefill_len = 100;
    private int decode_len = 200;
    private float prefill_token_speed = -1f; // tok/s
    private float decode_token_speed = -1f; // tok/s
    private float prefill_capacity = -1f; // uAh/tok
    private float decode_capacity = -1f; // uAh/tok
    private float prefill_energy= -1f; // mJ/tok
    private float decode_energy = -1f; // mJ/tok
    private final int test_times = 3;

    private ArrayList<Integer> decodeCorePlan;
    private int decode_tune_tolerance = 5;
    private float mBatteryVoltage = 0;
    private int mChargeNow = 0;
    // model profiling config end>

    private float getCPUTemperature() {
        float tempInCelsius = 0;
        try {
            for (int i=0; i<30; ++i) {
                BufferedReader reader = new BufferedReader(new FileReader(String.format("/sys/class/thermal/thermal_zone%d/type", i)));
                String type = reader.readLine();
                reader = new BufferedReader(new FileReader(String.format("/sys/class/thermal/thermal_zone%d/temp", i)));
                String temperature = reader.readLine(); // Read the first line of the file
                // Convert the temperature from millidegrees Celsius to degrees Celsius
                tempInCelsius = (float) Integer.parseInt(temperature);
                if (tempInCelsius>1000)  {
                    tempInCelsius = tempInCelsius/ 1000.0f;
                }
                if (type.contains("cpu-") || type.contains("cluster")) {
                    break;
                }
            }
        } catch (IOException e) {
            Log.e("Error", "Bad Read");
        }
        Log.i("Temperature", "CPU Temperature: " + tempInCelsius + "°C");
        return tempInCelsius;
    }

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

    private int getAvgCurrent() { return energyMonitor.getAvgCurrent(); } // in uA

    private float getAvgPower() { return energyMonitor.getAvgPower(); } // in uW

    private void startTimeTracing() {
        mElapsedBegin = elapsedRealtime();
    }

    private void endTimeTracing() {
        mElapsedEnd = elapsedRealtime();
    }

    private float getTime() {
        return ((float)(mElapsedEnd-mElapsedBegin))/1000f; // convert from ms to s.
    }

    public float getVoltage() {
        return mBatteryVoltage;
    }

    public int getChargeLevel() { return mChargeNow; }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // initialize all variables with their layout items.
        mEngineSelectSpinner = findViewById(R.id.EngineSelect);
        mModelSelectSpinner = findViewById(R.id.modelPath);
        mBackendSelectSpinner = findViewById(R.id.backendSelect);
        mPrefillPowerSelectSpinner = findViewById(R.id.PrefillPowerMode);
        mDecodePowerSelectSpinner = findViewById(R.id.DecodePowerMode);
        mLoadButton = findViewById(R.id.load_button);
        statusTV = findViewById(R.id.idTVstatus);
        prefillSpeedTV = findViewById(R.id.PrefillSpeed);
        prefillBatteryTV = findViewById(R.id.PrefillCapacity);
        prefillEnergyTV = findViewById(R.id.PrefillEnergy);
        decodeSpeedTV = findViewById(R.id.DecodeSpeed);
        decodeBatteryTV = findViewById(R.id.DecodeCapacity);
        decodeEnergyTV = findViewById(R.id.DecodeEnergy);
        mTestModeSpinner = findViewById(R.id.TestMode);
        mDatasetSpinner = findViewById(R.id.DatasetSelect);
        FixedLengthTestLayout = findViewById(R.id.FixedLengthTest);
        DatasetTestLayout = findViewById(R.id.DatasetTest);
        testButton = findViewById(R.id.startTest);
        prefillThreadNumTV = findViewById(R.id.PrefillThreadNum);
        decodeThreadNumTV = findViewById(R.id.DecodeThreadNum);
        decodeCorePlanInputTV = findViewById(R.id.DecodeCorePlan);
        tuneTimesTV = findViewById(R.id.tuneTimes);
        toleranceTV = findViewById(R.id.DecodeTol);
        prefillLenTV = findViewById(R.id.prefillLen);
        decodeLenTV = findViewById(R.id.decodeLen);
        decodeCorePlanTV = findViewById(R.id.DecodeCorePlanDisplay);
        prefillSpeedTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
        prefillEnergyTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
        decodeSpeedTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
        decodeEnergyTV.setBackgroundColor(getResources().getColor(R.color.purple_200));
        testButton.setBackgroundColor(getResources().getColor(R.color.purple_200));
        testButton.setClickable(false);
        populateEngineSpinner();
        populateModelSpinner();
        populateBackendSpinner();
        populatePowerSpinner(mPrefillPowerSelectSpinner);
        populatePowerSpinner(mDecodePowerSelectSpinner);
        populateTestSpinner();
        populateDatasetSpinner();


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
                    prefillBatteryTV.setText(String.format("prefill battery use:\n %.4f uAh/tok", message.getData().getFloat("prefill_capacity")));
                    prefillEnergyTV.setText(String.format("prefill energy:\n %.4f mJ/tok", message.getData().getFloat("prefill_energy")));
                    decodeSpeedTV.setText(String.format("decode speed:\n %.4f tok/s", message.getData().getFloat("decode_token_speed")));
                    decodeBatteryTV.setText(String.format("decode battery use:\n %.4f uAh/tok", message.getData().getFloat("decode_capacity")));
                    decodeEnergyTV.setText(String.format("decode energy:\n %.4f mJ/tok", message.getData().getFloat("decode_energy")));
                    statusTV.setText("Test Finished!");
                } else if (message.getData().getString("call").equals("loadModel")) {
                    decodeCorePlanTV.setText("decode core plan: " + message.getData().getString("decode_core_plan"));
                    statusTV.setText("模型加载完成！");
                    mLoadButton.setText("模型已加载");
                    testButton.setClickable(true);
                }
            }
        };
        IntentFilter intentfilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = this.registerReceiver(broadcastreceiver, intentfilter);

    }
    private boolean checkModelsReady() {
        File dir = new File(mModelDir);
        return dir.exists();
    }

    private BroadcastReceiver broadcastreceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            mBatteryVoltage = (float) (intent.getIntExtra(BatteryManager.EXTRA_VOLTAGE,0)) / 1000f; // mV -> V
            mChargeNow = (int) (intent.getIntExtra(BatteryManager.EXTRA_LEVEL,0)); // charge level
        }
    };

    private void onCheckModels() {
        boolean modelReady = checkModelsReady();

        mModelName = mModelSelectSpinner.getSelectedItem().toString();
        statusTV.setText(String.format("%s加载中", mModelName));
        mLoadButton.setText("模型加载中");

    }

    private void handleBackendName(View view) {
        if (mEngineName.equals("MNN")) {
            if (mBackendName.equals("CPU")) {
                mBackend = "cpu";
            } else if (mBackendName.equals("GPU")) {
                mBackend = "opencl";
            } else {
                mBackend = "cpu";
                statusTV.setText(String.format("Not support MNN+%s, use CPU", mBackend));
            }
        }
        if (mEngineName.equals("llama.cpp")) {
            if (!mBackendName.equals("CPU")) {
                statusTV.setText(String.format("Not support llama.cpp+%s, use CPU", mBackendName));
            }
        }
    }

    public void loadModel(View view) {
        onCheckModels();
        mLoadButton.setClickable(false);
        mLoadButton.setBackgroundColor(Color.parseColor("#2454e4"));
        mLoadButton.setText("模型加载中 ...");

        mModelName = mModelSelectSpinner.getSelectedItem().toString();
        mEngineName = mEngineSelectSpinner.getSelectedItem().toString();
        mBackendName = mBackendSelectSpinner.getSelectedItem().toString();
        handleBackendName(view);
        if (mEngineName.equals("MNN")) {
            mModelDir = mSearchPath + mModelName + mConfigName;
        } else {
            mModelDir = mSearchPath + mModelName;
        }
        Log.i("LLM Model Path", mModelDir);

        new Thread(() -> {
            mChat = new Chat();
            mChat.Init(mEngineName, mModelDir, mBackend,
                       tmpDir.getPath(),
                       prefillThreadNumTV.getText().toString(),
                       decodeThreadNumTV.getText().toString(),
                       mPrefillPowerSelectSpinner.getSelectedItem().toString(),
                       mDecodePowerSelectSpinner.getSelectedItem().toString(),
                       decodeCorePlanInputTV.getText().toString(),
                       tuneTimesTV.getText().toString());
            if (mPrefillPowerSelectSpinner.getSelectedItem().toString().equals("tune_prefill")) {
                mChat.tunePrefill();
            }
            decodeTune();
            String decode_core_plan = "";
            if (decodeCorePlan!=null) {
                for (int i = 0; i<decodeCorePlan.size(); ++i) {
                    decode_core_plan += String.format("%d ", decodeCorePlan.get(i));
                }
            }
            if (!decode_core_plan.isEmpty()) {
                decode_core_plan = decode_core_plan.substring(0, decode_core_plan.length() - 1); // remove the last ' '
            }
            Message message=new Message();
            Bundle data=new Bundle();
            data.putString("decode_core_plan", decode_core_plan);
            data.putString("call", "loadModel");
            message.setData(data);
            mHandler.sendMessage(message);
        }).start();
    }

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

    private ArrayList<String> getFileList(String path) {
        File directory = new File(path);
        File[] files = directory.listFiles();
        ArrayList<String> fileList = new ArrayList<String>();
        if (files != null) {
            for (File file : files) {
                fileList.add(file.getName());
            }
        }
        return fileList;
    }

    private void populateModelSpinner() {
        ArrayList<String> models = getFileList(mSearchPath);
        models.add(0, "select model");
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, models);
        mModelSelectSpinner.setAdapter(adapter);
    }

    private void populateEngineSpinner() {
        ArrayList<String> engines = new ArrayList<String>();
        engines.add(0, "select engine");
        engines.add("MNN");
        engines.add("llama.cpp");
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, engines);
        mEngineSelectSpinner.setAdapter(adapter);
    }

    private void populateBackendSpinner() {
        ArrayList<String> backends = new ArrayList<String>();
        backends.add(0, "select backend");
        backends.add("CPU");
        backends.add("GPU");
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, backends);
        mBackendSelectSpinner.setAdapter(adapter);
    }

    private void populatePowerSpinner(Spinner spinner) {
        ArrayList<String> powers = new ArrayList<String>();
        powers.add(0, spinner.getPrompt().toString());
        powers.add("(default)");
        powers.add("normal");
        powers.add("high");
        powers.add("memory");
        powers.add("tune_prefill");
        powers.add("exhaustive");
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, powers);
        spinner.setAdapter(adapter);
    }

    private void populateDatasetSpinner() {
        String[] paths;
        AssetManager assetManager=this.getAssets();
        try {
            paths = assetManager.list("samples");
        } catch (IOException e) {
            return;
        }
        if (paths==null) {
            return;
        }
        ArrayList<String> datasets = new ArrayList<String>();
        for (String path: paths) {
            try {
                String[] dirs = assetManager.list(Paths.get("samples", path).toString());
                if (dirs!=null && dirs.length!=0) {
                    datasets.add(path);
                }
            } catch (IOException e) {
                continue;
            }
        }
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, datasets);
        mDatasetSpinner.setAdapter(adapter);
    }

    private void populateTestSpinner() {
        ArrayList<String> tests = new ArrayList<String>();
        tests.add("Fixed Length Test");
        tests.add("Dataset Test");
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, tests);
        mTestModeSpinner.setAdapter(adapter);
        // Set the OnItemSelectedListener
        mTestModeSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                // Get the selected item
                String selectedItem = parent.getItemAtPosition(position).toString();
                if (selectedItem=="Fixed Length Test") {
                    DatasetTestLayout.setVisibility(View.GONE);
                    FixedLengthTestLayout.setVisibility(View.VISIBLE);
                } else if (selectedItem=="Dataset Test") {
                    FixedLengthTestLayout.setVisibility(View.GONE);
                    DatasetTestLayout.setVisibility(View.VISIBLE);
                }
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) {
                // Another interface callback
            }
        });
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

    public float getAvgCapacityInuAh(ArrayList<Integer> uAList, ArrayList<Float> sList) {
        float res = 0f;
        for (int i=0; i<uAList.size(); ++i) {
            res += uAList.get(i)*sList.get(i)/3600f; // turn second into hour, unit: uAh
        }
        return res/uAList.size();
    }

    public float getAvgEnergyInmJ(ArrayList<Float> mWList, ArrayList<Float> sList) {
        float res = 0f;
        for (int i=0; i<mWList.size(); ++i) {
            res += mWList.get(i)*sList.get(i)/1000f; // unit: mJ
        }
        return res/mWList.size();
    }

    public void decodeTune() {
        if (decode_tune_tolerance==-1) {
            // no tuning
            return;
        }
        boolean tune_end = false;
        decodeCorePlan = new ArrayList<Integer>();
        while (!tune_end) {
            startEnergyTracing();
            startTimeTracing();
            mChat.startDecodeTune(decode_tune_tolerance);
            endTimeTracing();
            endEnergyTracing();
            tune_end = mChat.endDecodeTune(decodeCorePlan, -getAvgPower()/1000f, decode_tune_tolerance); // unit: mW (negated)
        }
    }

    public void testRun(View v) {
        Log.i("Charge",String.format("Charge Level: %d", getChargeLevel()));
        if (getChargeLevel()<50) {
            statusTV.setText(String.format("Current Charge Level: %d %s, Please plug in and charge!", getChargeLevel(), "%"));
            return;
        }
        if (getCPUTemperature()>40) {
            statusTV.setText(String.format("Current CPU Temperature: %.1f °C, Please cool down!", getCPUTemperature()));
            return;
        }
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
        try {
            decode_tune_tolerance = Integer.parseInt(toleranceTV.getText().toString());
        } catch (NumberFormatException e) {
            decode_tune_tolerance = -1;
            statusTV.setText("Warning: no decode tuning!");
        }



        new Thread(() -> {
            // tracing
            ArrayList<Integer> uAPrefillList = new ArrayList<Integer>(); // in uA
            ArrayList<Integer> uADecodeList = new ArrayList<Integer>(); // in uA
            ArrayList<Float> timePrefillList = new ArrayList<Float>(); // in s
            ArrayList<Float> timeDecodeList = new ArrayList<Float>(); // in s
            ArrayList<Float> powerPrefillList = new ArrayList<Float>(); // in mW
            ArrayList<Float> powerDecodeList = new ArrayList<Float>(); // in mW
            for (int i = 0; i < test_times; ++i) {
                startEnergyTracing();
                startTimeTracing();
                mChat.Forward(prefill_len, true, true);
                endTimeTracing();
                endEnergyTracing();
                uAPrefillList.add(getAvgCurrent());
                powerPrefillList.add(getAvgPower());
                timePrefillList.add(getTime());

                startEnergyTracing();
                startTimeTracing();
                mChat.Forward(decode_len, false, false);
                endTimeTracing();
                endEnergyTracing();
                uADecodeList.add(getAvgCurrent());
                powerDecodeList.add(getAvgPower());
                timeDecodeList.add(getTime());

                mChat.Reset();
            }
            prefill_token_speed = prefill_len / avgFloatArray(timePrefillList);
            decode_token_speed = decode_len / avgFloatArray(timeDecodeList);
            prefill_capacity = -getAvgCapacityInuAh(uAPrefillList, timePrefillList) / prefill_len; // negate it, because it's doomed to be negative.
            decode_capacity = -getAvgCapacityInuAh(uADecodeList, timeDecodeList) / decode_len; // negate it, because it's doomed to be negative.
            prefill_energy = -getAvgEnergyInmJ(powerPrefillList, timePrefillList) / prefill_len; // negate it, because it's doomed to be negative.
            decode_energy = -getAvgEnergyInmJ(powerDecodeList, timeDecodeList) / decode_len; // negate it, because it's doomed to be negative.
            Log.i("prefill", String.format("prefill speed: %.4f tok/s", prefill_token_speed));
            Log.i("prefill", String.format("prefill battery use: %.4f uAh/tok", prefill_capacity));
            Log.i("prefill", String.format("prefill energy: %.4f mJ/tok", prefill_energy));
            Log.i("decode", String.format("decode speed: %.4f tok/s", decode_token_speed));
            Log.i("decode", String.format("decode battery use: %.4f uAh/tok", decode_capacity));
            Log.i("decode", String.format("decode energy: %.4f mJ/tok", decode_energy));
            Message message=new Message();
            Bundle data=new Bundle();
            data.putFloat("prefill_token_speed", prefill_token_speed);
            data.putFloat("prefill_capacity", prefill_capacity);
            data.putFloat("prefill_energy", prefill_energy);
            data.putFloat("decode_token_speed", decode_token_speed);
            data.putFloat("decode_capacity", decode_capacity);
            data.putFloat("decode_energy", decode_energy);
            data.putString("call", "testRun");
            message.setData(data);
            mHandler.sendMessage(message);
        }).start();
    }
}