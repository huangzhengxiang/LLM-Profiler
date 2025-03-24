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

static int dataset_itr=0;
static int cnv_itr=0;
static std::string input_prompt;
static std::vector<std::vector<std::vector<PromptItem>>> test_dataset;

#define TEST_TOKEN 200

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnLoad");
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "MNN_DEBUG", "JNI_OnUnload");
}


JNIEXPORT jstring JNICALL Java_com_iot_audio_Chat_getDatasetCurrentInput(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF(input_prompt.c_str());
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
    bool gotData = false;
    for (auto& item : dialog) {
        if (item.first=="user") {
            input_prompt = item.second;
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
        //     input_prompt = item.second;
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