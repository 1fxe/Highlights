#ifndef _Included_fxe_highlights_MinecraftHighlights
#define _Included_fxe_highlights_MinecraftHighlights

#include <jni.h>
#include "GfeSDKWrapper.hpp"
#include <windows.h>

namespace Highlights {
    extern "C" {
/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    init
 * Signature: (Ljava/lang/String;Ljava/util/List;)V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_init
            (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    shutdown
 * Signature: ()V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_shutdown
            (JNIEnv *, jobject);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    tick
 * Signature: ()V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_tick
            (JNIEnv *, jobject);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    openGroup
 * Signature: (Ljava/lang/String;)V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_openGroup
            (JNIEnv *, jobject, jstring);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    closeGroup
 * Signature: (Ljava/lang/String;Z)V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_closeGroup
            (JNIEnv *, jobject, jstring, jboolean);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    saveScreenShot
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_saveScreenShot
            (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    saveVideo
 * Signature: (Ljava/lang/String;Ljava/lang/String;II)V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_saveVideo
            (JNIEnv *, jobject, jstring, jstring, jint, jint);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    getNumHighlights
 * Signature: (Ljava/lang/String;Ldev/fxe/highlights/NVGSDK_HighlightSignificance;Ldev/fxe/highlights/NVGSDK_HighlightType;)V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_getNumHighlights
            (JNIEnv *, jobject, jstring, jobject, jobject);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    openSummary
 * Signature: ([Ljava/lang/String;ILdev/fxe/highlights/NVGSDK_HighlightSignificance;Ldev/fxe/highlights/NVGSDK_HighlightType;)V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_openSummary
            (JNIEnv *, jobject, jobjectArray, jint, jobject, jobject);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    requestLanguage
 * Signature: ()V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_requestLanguage
            (JNIEnv *, jobject);

/*
 * Class:     dev_fxe_highlights_Highlights
 * Method:    requestUserSettings
 * Signature: ()V
 */
    JNIEXPORT void JNICALL Java_dev_fxe_highlights_Highlights_requestUserSettings
            (JNIEnv *, jobject);

    }
}