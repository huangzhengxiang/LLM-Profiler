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

// MNN headers
#include "llm/llm.hpp"
// llama.cpp headers
#include "arg.h"
#include "common.h"
#include "log.h"
#include "sampling.h"
#include "llama.h"

using namespace MNN;
using namespace MNN::Express;
using namespace MNN::Transformer;

static std::unique_ptr<Llm> llm(nullptr);
static std::stringstream response_buffer;
static std::string engine_name;

static std::unique_ptr<common_params> llama_params;
static std::unique_ptr<common_init_result> llama_model;

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnLoad");
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnUnload");
}

void trace() {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Trace");
    if (engine_name=="MNN") {
        if (!llm.get()) {
            __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm not ready!");
            return;
        } else {
            __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm is ready!");
        }
        llm->trace(true);
        std::vector<int> test_prompt(30, 200);
        llm->switchMode(Llm::Prefill);
        llm->setKVCacheInfo(test_prompt.size(), 0);
        llm->forward(test_prompt);
        llm->trace(false);
        // reset
        llm->reset();
        llm->setKVCacheInfo(0, llm->getCurrentHistory());
    }
}

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_Init(JNIEnv* env, jobject thiz,
                                                        jstring engineName,
                                                        jstring modelDir,
                                                        jstring tmpFile,
                                                        jstring prefillThreadNum,
                                                        jstring decodeThreadNum,
                                                        jstring prefillPowerMode,
                                                        jstring decodePowerMode,
                                                        jstring decodeCorePlan,
                                                        jstring tuneTimes) {
    const char* model_dir = env->GetStringUTFChars(modelDir, 0);
    engine_name = std::string(env->GetStringUTFChars(engineName, 0));
    std::string tmp_path = std::string(env->GetStringUTFChars(tmpFile, 0));
    std::string prefill_thread_num = std::string(env->GetStringUTFChars(prefillThreadNum, 0));
    std::string decode_thread_num = std::string(env->GetStringUTFChars(decodeThreadNum, 0));
    std::string prefill_power_mode = std::string(env->GetStringUTFChars(prefillPowerMode, 0));
    std::string decode_power_mode = std::string(env->GetStringUTFChars(decodePowerMode, 0));
    std::string decode_cores = std::string(env->GetStringUTFChars(decodeCorePlan, 0));
    std::string decode_tune_times = std::string(env->GetStringUTFChars(tuneTimes, 0));
    if (prefill_power_mode=="tune_prefill") prefill_power_mode="high";
    if (engine_name=="MNN") {
        if (!llm.get()) {
            llm.reset(Llm::createLLM(model_dir));
            llm->set_config("{\"tmp_path\":\"" + tmp_path + "\"}"); // tmp_path (string, need quotation marks)
            if (!prefill_thread_num.empty()) { llm->set_config("{\"prefill_thread_num\":" + prefill_thread_num + "}"); } // thread_num (int, no quotation marks)
            if (!decode_thread_num.empty()) { llm->set_config("{\"decode_thread_num\":" + decode_thread_num + "}"); } // thread_num (int, no quotation marks)
            if (!prefill_power_mode.empty()) { llm->set_config("{\"prefill_power\":\"" + prefill_power_mode + "\"}"); } // power (string: need quotation marks)
            if (!decode_power_mode.empty()) { llm->set_config("{\"decode_power\":\"" + decode_power_mode + "\"}"); } // power (string: need quotation marks)
            if (!decode_cores.empty()) { llm->set_config("{\"decode_cores\":\"" + decode_cores + "\"}"); } // power (string: need quotation marks)
            if (!decode_tune_times.empty()) { llm->set_config("{\"decode_tune_times\":" + decode_tune_times + "}"); } // power (string: need quotation marks)
            llm->load();
            trace();
        }
    }
    if (engine_name=="llama.cpp") {
        
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_Ready(JNIEnv* env, jobject thiz) {
    if (llm.get()) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}
JNIEXPORT void JNICALL Java_com_iot_audio_Chat_Trace(JNIEnv* env, jobject thiz) {
    trace();
}
JNIEXPORT void JNICALL Java_com_iot_audio_Chat_tunePrefill(JNIEnv* env, jobject thiz) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "tunePrefill");
    if (!llm.get()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm not ready!");
        return;
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm is ready!");
    }
    llm->tuning(PREFILL_BIGLITTLE_CORE, {});
}
JNIEXPORT void JNICALL Java_com_iot_audio_Chat_startDecodeTune(JNIEnv* env, jobject thiz, jint tolerance) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Trace");
    if (!llm.get()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm not ready!");
        return;
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm is ready!");
    }
    std::vector<int> empty;
    llm->decode_tuning(empty, nullptr, (int)tolerance);
}

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_endDecodeTune(JNIEnv* env, jobject thiz, jobject arrayList, jfloat energy, jint tolerance) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Trace");
    if (!llm.get()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm not ready!");
        return true;
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm is ready!");
    }
    float cenergy = (float)energy;
    std::vector<int> core_plan;
    bool tune_end = llm->decode_tuning(core_plan, &cenergy, (int)tolerance);
    // modify arrayList
    if (tune_end) {
        jclass arrayListClass = env->GetObjectClass(arrayList);
        jclass integerClass = env->FindClass("java/lang/Integer");
        jmethodID addMethodID = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
        for (int core : core_plan) {
            jobject newElement = env->NewObject(integerClass, env->GetMethodID(integerClass, "<init>", "(I)V"), core);
            env->CallBooleanMethod(arrayList, addMethodID, newElement);
            env->DeleteLocalRef(newElement);
        }
    }
    return  (jboolean)tune_end;
}


JNIEXPORT void JNICALL Java_com_iot_audio_Chat_Forward(JNIEnv* env, jobject thiz, jint length, jboolean is_prefill, jboolean is_first_prefill) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Submit");
    if (!llm.get()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm not ready!");
        return;
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm is ready!");
    }
    std::vector<int> test_prompt((int)length, 200);
    VARP logits;
    float res = 0.f;
    if ((bool)is_prefill) {
        // test prefill
        llm->switchMode(Llm::Prefill);
        if ((bool)is_first_prefill) {
            llm->setKVCacheInfo(test_prompt.size(), llm->getCurrentHistory());
        } else {
            llm->setKVCacheInfo(test_prompt.size(), 0);
        }
        logits = llm->forward(test_prompt); // prefill a prompt of length length.
        res = logits->readMap<float>()[0];
    } else {
        // test decode, decode for length times
        llm->switchMode(Llm::Decode);
        for(int i=0; i<(int)length; ++i) {
            llm->setKVCacheInfo(1, 0);
            logits = llm->forward({200}); 
            res += logits->readMap<float>()[0]; 
        }
    }
    __android_log_print(ANDROID_LOG_INFO, "MNN_PROFILE", "res: %.4f", res);
    __android_log_print(ANDROID_LOG_INFO, "MNN_DEBUG", "After Foward!");
    return;
}

JNIEXPORT jstring JNICALL Java_com_iot_audio_Chat_Submit(JNIEnv* env, jobject thiz, jstring inputStr) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Submit");
    if (!llm.get()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm not ready!");
        return env->NewStringUTF("Failed, Chat is not ready!");
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm is ready!");
    }
    const char* input_str = env->GetStringUTFChars(inputStr, 0);
    llm->response(input_str, &response_buffer, "");
    jstring result = env->NewStringUTF("Submit success!");
    return result;
}

JNIEXPORT jbyteArray JNICALL Java_com_iot_audio_Chat_Response(JNIEnv* env, jobject thiz) {
    auto len = response_buffer.str().size();
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "length: %d", len);
    jbyteArray res = env->NewByteArray(len);
    env->SetByteArrayRegion(res, 0, len, (const jbyte*)response_buffer.str().c_str());
    return res;
}

JNIEXPORT void JNICALL Java_com_iot_audio_Chat_Done(JNIEnv* env, jobject thiz) {
    response_buffer.str("");
}

JNIEXPORT void JNICALL Java_com_iot_audio_Chat_Reset(JNIEnv* env, jobject thiz) {
    if (engine_name=="MNN") {
        llm->setKVCacheInfo(0, llm->getCurrentHistory());
        llm->reset();
    }
}

} // extern "C"