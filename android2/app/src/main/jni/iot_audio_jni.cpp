#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>

#include <jni.h>
#include <string>
#include <vector>
#include <sstream>
#include <thread>

#include "llm/llm.hpp"
using namespace MNN;
using namespace MNN::Express;
using namespace MNN::Transformer;

static std::unique_ptr<Llm> llm(nullptr);
static std::stringstream response_buffer;

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnLoad");
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnUnload");
}

JNIEXPORT jboolean JNICALL Java_com_iot_audio_Chat_Init(JNIEnv* env, jobject thiz, jstring modelDir, jstring tmpFile, jstring threadNum) {
    const char* model_dir = env->GetStringUTFChars(modelDir, 0);
    std::string tmp_path = std::string(env->GetStringUTFChars(tmpFile, 0));
    std::string thread_num = std::string(env->GetStringUTFChars(threadNum, 0));
    if (!llm.get()) {
        llm.reset(Llm::createLLM(model_dir));
        llm->set_config("{\"tmp_path\":\"" + tmp_path + "\"}");
        llm->set_config("{\"thread_num\":" + thread_num + "}");
        llm->load();
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
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "Trace");
    if (!llm.get()) {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm not ready!");
        return;
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "llm is ready!");
    }
    llm->trace(true);
    llm->forward({200, 200});
    llm->trace(false);
    llm->reset();
}

JNIEXPORT void JNICALL Java_com_iot_audio_Chat_Forward(JNIEnv* env, jobject thiz, jint length, jboolean is_prefill) {
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
        logits = llm->forward(test_prompt, true); // prefill a prompt of length length.
        res = logits->readMap<float>()[0];
    } else {
        // test decode, decode for length times
        for(int i=0; i<(int)length; ++i) { logits = llm->forward({200}, false); res += logits->readMap<float>()[0]; }
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
    std::vector<int> ids = llm->tokenizer_encode(input_str, false);
    std::string output_str = llm->generate(ids, &response_buffer, "");
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "%s", output_str.c_str());
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
    llm->reset();
}

} // extern "C"