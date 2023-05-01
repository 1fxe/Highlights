#pragma once

#ifndef _Included_fxe_highlights_Highlights
#define _Included_fxe_highlights_Highlights

#include <jni.h>
#include <codecvt>
#include <locale>
#include <memory>

#include <cstring>
#include <gfesdk/bindings/cpp/isdk_cpp_impl.h>
#include <gfesdk/bindings/cpp/highlights/ihighlights_cpp_impl.h>

#include <windows.h>

extern "C" {

namespace Highlights {
    class Wrapper {
    public:
        Wrapper();

        void init(JNIEnv *env, jstring gameName, jobject highlightsList);

        void shutdown();

        void tick() const;

        void openGroup(JNIEnv *env, jstring groupId);

        void closeGroup(JNIEnv *env, jstring groupId, jboolean destroy);

        void saveScreenShot(JNIEnv *env, jstring highlightId, jstring groupId);

        void saveVideo(JNIEnv *env, jstring highlightId, jstring groupId, jint startDelta, jint endDelta);

        void getNumHighlights(JNIEnv *env, jstring groupId, jobject sigFilter, jobject tagFilter);

        void openSummary(JNIEnv *env, jobjectArray groupIds, jint numGroups, jobject sigFilter, jobject tagFilter);

        void requestLanguage();

        void requestUserSettings();

        [[nodiscard]] wchar_t const *GetCurrentPermissionStr() const;

        [[nodiscard]] wchar_t const *GetLastOverlayEvent() const;

        [[nodiscard]] wchar_t const *GetLastResult() const;

        [[nodiscard]] wchar_t const *GetLastQueryResult() const;

        void ConfigureHighlights(char const *defaultLocale, GfeSDK::NVGSDK_Highlight *highlights, size_t numHighlights);

        void UpdateLastResultString(GfeSDK::NVGSDK_RetCode rc);

        std::unique_ptr<GfeSDK::Core> m_gfesdk;
        std::unique_ptr<GfeSDK::Highlights> m_highlights;

        std::wstring_convert<std::codecvt_utf8<wchar_t>> m_converter;
        std::wstring m_currentPermission;
        std::wstring m_lastOverlayEvent;
        std::wstring m_lastResult;
        std::wstring m_lastQueryResult;

        void OnNotification(GfeSDK::NVGSDK_NotificationType type, const GfeSDK::NotificationBase &notification);

    };

    inline void InitGfeSdkWrapper(Highlights::Wrapper *hl) {}

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
#endif