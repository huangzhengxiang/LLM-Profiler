// Android headers
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>
// system headers
#include <jni.h>
#include <string>
#include <vector>
#include <sstream>
#include <thread>

#include "dataset.hpp"
#include "llm_wrapper.h"

static int dataset_itr=0;
static int cnv_itr=0;
static int last_res=0;
static std::vector<int> input_prompt;
static std::vector<std::vector<std::vector<PromptItem>>> test_dataset;
static std::unique_ptr<LLMWrapper> model(nullptr);

#define TEST_TOKEN 200

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnLoad");
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnUnload");
}

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_InitNative(JNIEnv *env, jobject thiz,
                                                              jstring engineName,
                                                              jstring modelDir,
                                                              jstring backendName,
                                                              jstring tmpFile,
                                                              jstring prefillThreadNum,
                                                              jstring decodeThreadNum,
                                                              jstring prefillPowerMode,
                                                              jstring decodePowerMode,
                                                              jstring decodeCorePlan,
                                                              jstring tuneTimes) {
    const char *model_dir = env->GetStringUTFChars(modelDir, 0);
    std::string engine_name = std::string(env->GetStringUTFChars(engineName, 0));
    std::string backend_name = std::string (env->GetStringUTFChars(backendName, 0));
    std::string tmp_path = std::string(env->GetStringUTFChars(tmpFile, 0));
    std::string prefill_thread_num = std::string(env->GetStringUTFChars(prefillThreadNum, 0));
    std::string decode_thread_num = std::string(env->GetStringUTFChars(decodeThreadNum, 0));
    std::string prefill_power_mode = std::string(env->GetStringUTFChars(prefillPowerMode, 0));
    std::string decode_power_mode = std::string(env->GetStringUTFChars(decodePowerMode, 0));
    std::string decode_cores = std::string(env->GetStringUTFChars(decodeCorePlan, 0));
    std::string decode_tune_times = std::string(env->GetStringUTFChars(tuneTimes, 0));
    if (prefill_power_mode == "tune_prefill") prefill_power_mode = "high";
    if (prefill_power_mode == "(default)") prefill_power_mode = "";
    if (decode_power_mode == "(default)") decode_power_mode = "";
    __android_log_print(ANDROID_LOG_INFO, "MNN_DEBUG", "Loading engine: %s\n", engine_name.c_str());
    model.reset(LLMWrapper::createWrapper(model_dir,
                                          backend_name,
                                          tmp_path,
                                          engine_name,
                                          prefill_thread_num,
                                          decode_thread_num,
                                          prefill_power_mode,
                                          decode_power_mode,
                                          decode_cores,
                                          decode_tune_times));
    if (model.get() == nullptr) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_Ready(JNIEnv *env, jobject thiz) {
    if (model->isReady()) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}
JNIEXPORT void JNICALL Java_com_iot_audio_Chat_TraceNative(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Trace");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return;
    }
    model->trace();
}
JNIEXPORT void JNICALL Java_com_iot_audio_Chat_tunePrefillNative(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "tunePrefill");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return;
    }
    model->tunePrefill();
}
JNIEXPORT void JNICALL
Java_com_iot_audio_Chat_startDecodeTuneNative(JNIEnv *env, jobject thiz, jint tolerance) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "startDecodeTune");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return;
    }
    model->startDecodeTune((int) tolerance);
}

JNIEXPORT jboolean JNICALL
Java_com_iot_audio_Chat_endDecodeTuneNative(JNIEnv *env, jobject thiz, jobject arrayList, jfloat energy,
                                      jint tolerance) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Trace");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return JNI_TRUE;
    }
    float cenergy = (float) energy;
    std::vector<int> core_plan;
    bool tune_end = model->endDecodeTune(core_plan, &cenergy, (int) tolerance);
    // modify arrayList
    if (tune_end) {
        jclass arrayListClass = env->GetObjectClass(arrayList);
        jclass integerClass = env->FindClass("java/lang/Integer");
        jmethodID addMethodID = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
        for (int core: core_plan) {
            jobject newElement = env->NewObject(integerClass,
                                                env->GetMethodID(integerClass, "<init>", "(I)V"),
                                                core);
            env->CallBooleanMethod(arrayList, addMethodID, newElement);
            env->DeleteLocalRef(newElement);
        }
    }
    return (jboolean) tune_end;
}


JNIEXPORT void JNICALL
Java_com_iot_audio_Chat_ForwardNative(JNIEnv *env, jobject thiz, jint length, jboolean is_prefill,
                                jboolean is_first_prefill) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Forward");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return;
    }
    int res = TEST_TOKEN;
    if ((bool) is_prefill) {
        // test prefill
        std::vector<int> test_prompt((int) length, TEST_TOKEN);
        res = model->forward(test_prompt, is_prefill, is_first_prefill);
    } else {
        // test decode, decode for length times
        std::vector<int> test_prompt(1, TEST_TOKEN);
        for (int i = 0; i < (int) length; ++i) {
            test_prompt[0] = res;
            res = model->forward(test_prompt, is_prefill, is_first_prefill);
        }
    }
    __android_log_print(ANDROID_LOG_INFO, "MNN_PROFILE", "res: %d", res);
    __android_log_print(ANDROID_LOG_INFO, "MNN_DEBUG", "After Foward!");
    return;
}

JNIEXPORT jint JNICALL Java_com_iot_audio_Chat_DatasetResponseNative(JNIEnv *env, jobject thiz, jboolean is_prefill,
                                                               jboolean is_first_prefill) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Forward");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return 0;
    }
    int res = last_res;
    if ((bool) is_prefill) {
        // test prefill
        res = model->forward(input_prompt, is_prefill, is_first_prefill);
        last_res = res;
    } else {
        // test decode, decode for length times
        for (int i = 0; i < (int) input_prompt.size(); ++i) {
            res = model->forward({input_prompt[i]}, is_prefill, is_first_prefill);
        }
    }
    __android_log_print(ANDROID_LOG_INFO, "MNN_PROFILE", "res: %d", res);
    __android_log_print(ANDROID_LOG_INFO, "MNN_DEBUG", "After Response!");
    return input_prompt.size();
}

JNIEXPORT jstring JNICALL Java_com_iot_audio_Chat_ResponseNative(JNIEnv *env, jobject thiz, jstring input, jboolean is_first_prefill) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Forward");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return env->NewStringUTF("");
    }
    int res = last_res;
    std::string system_prompt = "";
    bool need_antiprompt = false;
    if (is_first_prefill) {
        system_prompt = "You're a helpful assistant."; // first time need system prompt
    } else {
        need_antiprompt = true; // followings need antiprompt
    }
    std::string inputStr = std::string(env->GetStringUTFChars(input, 0));
    if (!inputStr.empty()) {
        // test prefill
        res = model->forward(model->tokenizer_encode(inputStr, true, need_antiprompt, system_prompt), true, is_first_prefill);
        last_res = res;
    }

    // test decode, decode for length times
    std::vector<int> outTokens = {res};
    while (!model->isStop(res)) {
        res = model->forward({res}, false, is_first_prefill);
        outTokens.push_back(res);
    }

    __android_log_print(ANDROID_LOG_INFO, "MNN_PROFILE", "res: %d", res);
    __android_log_print(ANDROID_LOG_INFO, "MNN_DEBUG", "After Response!");
    return env->NewStringUTF(model->tokenizer_decode(outTokens).c_str());
}

JNIEXPORT void JNICALL Java_com_iot_audio_Chat_ResetNative(JNIEnv *env, jobject thiz) {
    model->reset();
}


JNIEXPORT jint JNICALL Java_com_iot_audio_Chat_StringTokenSizeNative(JNIEnv *env, jobject thiz, jstring input) {
    return (jint)model->tokenizer_encode(std::string(env->GetStringUTFChars(input, 0)), false, false, "").size();
}

JNIEXPORT jint JNICALL Java_com_iot_audio_Chat_loadDataset(JNIEnv *env, jobject thiz, jstring data) {
    dataset_itr=0; cnv_itr=0;
    std::string dataset_string = std::string(env->GetStringUTFChars(data, 0));
    parse_json(dataset_string, test_dataset);
    return (jint)test_dataset.size();
}

JNIEXPORT jint JNICALL Java_com_iot_audio_Chat_getDatasetSize(JNIEnv *env, jobject thiz) {
    return (jint)test_dataset.size();
}

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_getDialogUser(JNIEnv *env, jobject thiz) {
    auto& cnv = test_dataset[dataset_itr];
    if (cnv.size()<=cnv_itr) {
        return false;
    }
    auto& dialog = cnv[cnv_itr];
    std::string system_prompt = "";
    bool need_antiprompt = false;
    bool gotData = false;
    if (cnv_itr==0) {
        system_prompt = "You're a helpful assistant."; // first time need system prompt
    } else {
        need_antiprompt = true; // followings need antiprompt
    }
    for (auto& item : dialog) {
        if (item.first=="user") {
            input_prompt = model->tokenizer_encode(item.second, true, need_antiprompt, system_prompt);
            gotData = true;
            break;
        }
    }
    return (jboolean)gotData;
}

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_getDialogAssistant(JNIEnv *env, jobject thiz) {
    auto& cnv = test_dataset[dataset_itr];
    if (cnv.size()<=cnv_itr) {
        return false;
    }
    auto& dialog = cnv[cnv_itr];
    bool gotData = false;
    for (auto& item : dialog) {
        // do not get controlled assistant
        break;
        // if (item.first=="assistant") {
        //     input_prompt = model->tokenizer_encode(item.second, false, false, "");
        //     gotData = true;
        //     break;
        // }
    }
    cnv_itr++;
    return (jboolean)gotData;
}



JNIEXPORT void JNICALL Java_com_iot_audio_Chat_datasetNext(JNIEnv *env, jobject thiz) {
    dataset_itr++;
    cnv_itr=0;
    return;
}


JNIEXPORT void JNICALL Java_com_iot_audio_Chat_resetDataset(JNIEnv *env, jobject thiz) {
    dataset_itr=0;
    cnv_itr=0;
    return;
}

} // extern "C"