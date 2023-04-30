#include "library.h"

#define JSTR_TO_CSTR(env, jstr) (env)->GetStringUTFChars((jstr), NULL)

namespace Highlights {
    GfeSdkWrapper g_highlights;

    void Java_dev_fxe_highlights_Highlights_init(JNIEnv *env, jobject, jstring gameName, jobject highlights) {
        InitGfeSdkWrapper(&g_highlights);
        auto gName = JSTR_TO_CSTR(env, gameName);

        jclass listClass = env->FindClass("java/util/List");
        jmethodID listSizeMethod = env->GetMethodID(listClass, "size", "()I");
        jmethodID listGetMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");

        auto listSize = static_cast<int>(env->CallIntMethod(highlights, listSizeMethod));

        jclass highlightClass = env->FindClass("dev/fxe/highlights/Highlight");
        jmethodID idMethod = env->GetMethodID(highlightClass, "id", "()Ljava/lang/String;");
        jmethodID userInterestMethod = env->GetMethodID(highlightClass, "userInterest", "()Z");
        jmethodID highlightTagMethod = env->GetMethodID(highlightClass, "highlightTag",
                                                        "()Ldev/fxe/highlights/NVGSDK_HighlightType;");
        jmethodID significanceMethod = env->GetMethodID(highlightClass, "significance",
                                                        "()Ldev/fxe/highlights/NVGSDK_HighlightSignificance;");

        jmethodID nameTableMethod = env->GetMethodID(highlightClass, "nameTable", "()Ljava/util/List;");

        jclass highlightTypeEnum = env->FindClass("dev/fxe/highlights/NVGSDK_HighlightType");
        jmethodID highlightTypeValueMethod = env->GetMethodID(highlightTypeEnum, "value", "()I");

        jclass significanceEnum = env->FindClass("dev/fxe/highlights/NVGSDK_HighlightSignificance");
        jmethodID significanceValueMethod = env->GetMethodID(significanceEnum, "value", "()I");

        jclass localizedPairClass = env->FindClass("dev/fxe/highlights/LocalizedPair");
        jmethodID localeCodeMethod = env->GetMethodID(localizedPairClass, "localeCode", "()Ljava/lang/String;");
        jmethodID localizedStringMethod = env->GetMethodID(localizedPairClass, "localizedString",
                                                           "()Ljava/lang/String;");

        auto *highlight = new GfeSDK::NVGSDK_Highlight[listSize];
        for (int i = 0; i < listSize; ++i) {
            jobject highlightObj = env->CallObjectMethod(highlights, listGetMethod, i);
            auto idJStr = (jstring) env->CallObjectMethod(highlightObj, idMethod);
            auto userInterestBool = env->CallBooleanMethod(highlightObj, userInterestMethod) == JNI_TRUE;
            jobject highlightTagObj = env->CallObjectMethod(highlightObj, highlightTagMethod);
            jobject significanceObj = env->CallObjectMethod(highlightObj, significanceMethod);
            jobject nameTableObj = env->CallObjectMethod(highlightObj, nameTableMethod);

            auto nameTableSize = static_cast<int>(env->CallIntMethod(nameTableObj, listSizeMethod));
            auto *nameTable = new GfeSDK::NVGSDK_LocalizedPair[nameTableSize];
            for (jint j = 0; j < nameTableSize; ++j) {
                jobject localizedPairObj = env->CallObjectMethod(nameTableObj, listGetMethod, j);
                auto localeCodeJStr = (jstring) env->CallObjectMethod(localizedPairObj, localeCodeMethod);
                auto localizedStringJStr = (jstring) env->CallObjectMethod(localizedPairObj, localizedStringMethod);

                nameTable[j] = {
                        JSTR_TO_CSTR(env, localeCodeJStr),
                        JSTR_TO_CSTR(env, localizedStringJStr)
                };

            }

            highlight[i] = {
                JSTR_TO_CSTR(env, idJStr),
                userInterestBool,
                static_cast<GfeSDK::NVGSDK_HighlightType>(
                    env->CallIntMethod(highlightTagObj, highlightTypeValueMethod)),
                static_cast<GfeSDK::NVGSDK_HighlightSignificance>(
                    env->CallIntMethod(significanceObj, significanceValueMethod)),
                nameTable,
                static_cast<size_t>(nameTableSize)
            };
        }

        Highlights::g_highlights.Init(gName, highlight, listSize);


        (env)->ReleaseStringUTFChars(gameName, gName);
        (env)->DeleteLocalRef(listClass);
        (env)->DeleteLocalRef(highlightClass);
        (env)->DeleteLocalRef(highlightTypeEnum);
        (env)->DeleteLocalRef(significanceEnum);
        (env)->DeleteLocalRef(localizedPairClass);
    }

    void Java_dev_fxe_highlights_Highlights_shutdown(JNIEnv *, jobject) {
        Highlights::g_highlights.DeInit();
    }

    void Java_dev_fxe_highlights_Highlights_tick(JNIEnv *, jobject) {
        Highlights::g_highlights.OnTick();
    }

    void Java_dev_fxe_highlights_Highlights_openGroup(JNIEnv *env, jobject, jstring groupId) {
        auto groupStr = JSTR_TO_CSTR(env, groupId);
        Highlights::g_highlights.OnOpenGroup(groupStr);
        (env)->ReleaseStringUTFChars(groupId, groupStr);
    }

    void Java_dev_fxe_highlights_Highlights_closeGroup(JNIEnv *env, jobject, jstring groupId, jboolean destroy) {
        auto groupStr = JSTR_TO_CSTR(env, groupId);
        Highlights::g_highlights.OnCloseGroup(groupStr, destroy);
        (env)->ReleaseStringUTFChars(groupId, groupStr);
    }

    void Java_dev_fxe_highlights_Highlights_saveScreenShot(JNIEnv *env, jobject, jstring highlightId, jstring groupId) {
        auto highlightStr = JSTR_TO_CSTR(env, highlightId);
        auto groupStr = JSTR_TO_CSTR(env, groupId);
        Highlights::g_highlights.OnSaveScreenshot(highlightStr, groupStr);

        (env)->ReleaseStringUTFChars(highlightId, highlightStr);
        (env)->ReleaseStringUTFChars(groupId, groupStr);
    }

    void Java_dev_fxe_highlights_Highlights_saveVideo(JNIEnv *env, jobject, jstring highlightId, jstring groupId,
                                                      jint startDelta, jint endDelta) {
        auto highlightStr = JSTR_TO_CSTR(env, highlightId);
        auto groupStr = JSTR_TO_CSTR(env, groupId);
        Highlights::g_highlights.OnSaveVideo(highlightStr, groupStr, static_cast<int>(startDelta),
                                             static_cast<int>(endDelta));

        (env)->ReleaseStringUTFChars(highlightId, highlightStr);
        (env)->ReleaseStringUTFChars(groupId, groupStr);
    }

    // TODO Check this works
    void Java_dev_fxe_highlights_Highlights_getNumHighlights(JNIEnv *env, jobject, jstring groupId, jobject sigFilter,
                                                             jobject tagFilter) {
        auto groupStr = JSTR_TO_CSTR(env, groupId);
        jclass sigFilterClass = env->GetObjectClass(sigFilter);
        jmethodID sigFilterValueMethodId = env->GetMethodID(sigFilterClass, "value", "()I");
        auto sigFilterValue = static_cast<int>(env->CallIntMethod(sigFilter, sigFilterValueMethodId));
        auto sigFilterEnum = (GfeSDK::NVGSDK_HighlightSignificance) sigFilterValue;

        jclass tagFilterClass = env->GetObjectClass(tagFilter);
        jmethodID tagFilterValueMethodId = env->GetMethodID(tagFilterClass, "value", "()I");
        auto tagFilterValue = static_cast<int>(env->CallIntMethod(tagFilter, tagFilterValueMethodId));
        auto tagFilterEnum = (GfeSDK::NVGSDK_HighlightType) tagFilterValue;

        Highlights::g_highlights.OnGetNumHighlights(groupStr, sigFilterEnum, tagFilterEnum);
        (env)->ReleaseStringUTFChars(groupId, groupStr);
        (env)->DeleteLocalRef(sigFilterClass);
        (env)->DeleteLocalRef(tagFilterClass);
    }

    // TODO Check this works
    void Java_dev_fxe_highlights_Highlights_openSummary(JNIEnv *env, jobject, jobjectArray groupIds, jint numGroups,
                                                        jobject sigFilter, jobject tagFilter) {
        jclass sigFilterClass = env->GetObjectClass(sigFilter);
        jmethodID sigFilterValueMethodId = env->GetMethodID(sigFilterClass, "value", "()I");
        auto sigFilterValue = static_cast<int>(env->CallIntMethod(sigFilter, sigFilterValueMethodId));
        auto sigFilterEnum = (GfeSDK::NVGSDK_HighlightSignificance) sigFilterValue;

        jclass tagFilterClass = env->GetObjectClass(tagFilter);
        jmethodID tagFilterValueMethodId = env->GetMethodID(tagFilterClass, "value", "()I");
        auto tagFilterValue = static_cast<int>(env->CallIntMethod(tagFilter, tagFilterValueMethodId));
        auto tagFilterEnum = (GfeSDK::NVGSDK_HighlightType) tagFilterValue;

        auto groupIdsArray = new const char *[numGroups];
        for (int i = 0; i < numGroups; i++) {
            auto groupId = (jstring) env->GetObjectArrayElement(groupIds, i);
            groupIdsArray[i] = JSTR_TO_CSTR(env, groupId);
            (env)->ReleaseStringUTFChars(groupId, groupIdsArray[i]);
        }

        Highlights::g_highlights.OnOpenSummary(groupIdsArray, numGroups, sigFilterEnum, tagFilterEnum);

        delete[] groupIdsArray;
        (env)->DeleteLocalRef(sigFilterClass);
        (env)->DeleteLocalRef(tagFilterClass);
    }

    void Java_dev_fxe_highlights_Highlights_requestLanguage(JNIEnv *, jobject) {
        g_highlights.OnRequestLanguage();
    }

    void Java_dev_fxe_highlights_Highlights_requestUserSettings(JNIEnv *, jobject) {
        g_highlights.OnRequestUserSettings();
    }

}