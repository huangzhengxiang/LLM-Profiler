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

#include "llm_wrapper.h"

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

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_Init(JNIEnv *env, jobject thiz,
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
JNIEXPORT void JNICALL Java_com_iot_audio_Chat_Trace(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Trace");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return;
    }
    model->trace();
}
JNIEXPORT void JNICALL Java_com_iot_audio_Chat_tunePrefill(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "tunePrefill");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return;
    }
    model->tunePrefill();
}
JNIEXPORT void JNICALL
Java_com_iot_audio_Chat_startDecodeTune(JNIEnv *env, jobject thiz, jint tolerance) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "startDecodeTune");
    if (!model->isReady()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "model not ready!");
        return;
    }
    model->startDecodeTune((int) tolerance);
}

JNIEXPORT jboolean JNICALL
Java_com_iot_audio_Chat_endDecodeTune(JNIEnv *env, jobject thiz, jobject arrayList, jfloat energy,
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
Java_com_iot_audio_Chat_Forward(JNIEnv *env, jobject thiz, jint length, jboolean is_prefill,
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

JNIEXPORT void JNICALL Java_com_iot_audio_Chat_Reset(JNIEnv *env, jobject thiz) {
    model->reset();
}

} // extern "C"