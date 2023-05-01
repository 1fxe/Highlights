#include "library.h"

#define JSTR_TO_CSTR(env, jstr) (env)->GetStringUTFChars((jstr), NULL)
#define UTF_16_to_8(str) std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(std::u16string((char16_t *) str))
#include <iostream>

static std::wstring permissionToStringW(GfeSDK::NVGSDK_Permission p) {
    switch (p) {
        case GfeSDK::NVGSDK_PERMISSION_MUST_ASK:
            return L"Must Ask";
        case GfeSDK::NVGSDK_PERMISSION_GRANTED:
            return L"Granted";
        case GfeSDK::NVGSDK_PERMISSION_DENIED:
            return L"Denied";
        default:
            return L"UNKNOWN";
    }
}

static Highlights::Wrapper wrapper;

Highlights::Wrapper::Wrapper() :
        m_currentPermission(permissionToStringW(GfeSDK::NVGSDK_PERMISSION_MUST_ASK)) {
}

void Highlights::Wrapper::init(JNIEnv *env, jstring gameName, jobject highlightsList) {
    InitGfeSdkWrapper(&wrapper);
    auto gName = JSTR_TO_CSTR(env, gameName);

    jclass listClass = env->FindClass("java/util/List");
    jmethodID listSizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID listGetMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");

    auto numHighlights = static_cast<int>(env->CallIntMethod(highlightsList, listSizeMethod));

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

    auto *highlight = new GfeSDK::NVGSDK_Highlight[numHighlights];
    for (int i = 0; i < numHighlights; ++i) {
        jobject highlightObj = env->CallObjectMethod(highlightsList, listGetMethod, i);
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

            auto localeCodeStr = JSTR_TO_CSTR(env, localeCodeJStr);
            auto localeCodeCopy = (char *) malloc(strlen(localeCodeStr) + 1);
            strcpy_s(localeCodeCopy, strlen(localeCodeStr) + 1, localeCodeStr);

            auto localizedStringStr = JSTR_TO_CSTR(env, localizedStringJStr);
            auto localizedStringStrCopy = (char *) malloc(strlen(localizedStringStr) + 1);
            strcpy_s(localeCodeCopy, strlen(localizedStringStr) + 1, localizedStringStr);

            nameTable[j] = {
                    localeCodeCopy,
                    localizedStringStrCopy
            };

            (env)->ReleaseStringUTFChars(localeCodeJStr, localeCodeStr);
            (env)->ReleaseStringUTFChars(localizedStringJStr, localizedStringStr);
        }

        auto id = JSTR_TO_CSTR(env, idJStr);
        auto idCopy = (char *) malloc(strlen(id) + 1);
        strcpy_s(idCopy, strlen(id) + 1, id);

        highlight[i] = {
                idCopy,
                userInterestBool,
                static_cast<GfeSDK::NVGSDK_HighlightType>(
                        env->CallIntMethod(highlightTagObj, highlightTypeValueMethod)),
                static_cast<GfeSDK::NVGSDK_HighlightSignificance>(
                        env->CallIntMethod(significanceObj, significanceValueMethod)),
                nameTable,
                static_cast<size_t>(nameTableSize)
        };

        (env)->ReleaseStringUTFChars(idJStr, id);
    }

    using namespace std::placeholders;

    auto defaultLocale = "en-US";
    GfeSDK::CreateInputParams createParams;
    createParams.appName = gName;
    createParams.pollForCallbacks = true;
    createParams.requiredScopes = {
            GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS,
            GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_VIDEO,
            GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_SCREENSHOT
    };
    createParams.notificationCallback = [this](auto &&PH1, auto &&PH2) {
        OnNotification(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
    };

    GfeSDK::CreateResponse response;

    GfeSDK::Core *gfesdkCore = GfeSDK::Core::Create(createParams, response);
    if (!GfeSDK::NVGSDK_SUCCEEDED(response.returnCode)) {
        // Return error
        return;
    }

    m_gfesdk.reset(gfesdkCore);
    UpdateLastResultString(response.returnCode);

    if (response.scopePermissions.find(GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_VIDEO) != response.scopePermissions.end()) {
        m_currentPermission = permissionToStringW(response.scopePermissions[GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_VIDEO]);
    }

    GfeSDK::RequestPermissionsParams requestPermissionsParams;

    for (auto &&entry: response.scopePermissions) {
        if (entry.second == GfeSDK::NVGSDK_PERMISSION_MUST_ASK) {
            requestPermissionsParams.scopes.push_back(entry.first);
        }
    }

    if (!requestPermissionsParams.scopes.empty()) {
        m_gfesdk->RequestPermissionsAsync(requestPermissionsParams,
                                          [this, defaultLocale, highlight, numHighlights](GfeSDK::NVGSDK_RetCode rc,
                                                                                          void *cbContext) {
                                              UpdateLastResultString(rc);
                                              if (GfeSDK::NVGSDK_SUCCEEDED(rc)) {
                                                  ConfigureHighlights(defaultLocale, highlight, numHighlights);
                                              }
                                          });
    } else {
        ConfigureHighlights(defaultLocale, highlight, numHighlights);
    }

    (env)->ReleaseStringUTFChars(gameName, gName);
}

void Highlights::Wrapper::OnNotification(GfeSDK::NVGSDK_NotificationType type,
                                         GfeSDK::NotificationBase const &notification) {
    switch (type) {
        case GfeSDK::NVGSDK_NOTIFICATION_PERMISSIONS_CHANGED: {
            auto const &n = static_cast<GfeSDK::PermissionsChangedNotification const &>(notification);

            auto hlPermission = n.scopePermissions.find(GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_VIDEO);
            if (hlPermission != n.scopePermissions.end()) {
                m_currentPermission = permissionToStringW(hlPermission->second);
            }
            break;
        }
        case GfeSDK::NVGSDK_NOTIFICATION_OVERLAY_STATE_CHANGED: {
            auto const &n = static_cast<GfeSDK::OverlayStateChangedNotification const &>(notification);

            m_lastOverlayEvent.clear();
            switch (n.state) {
                case GfeSDK::NVGSDK_OVERLAY_STATE_MAIN:
                    m_lastOverlayEvent += L"Main Overlay Window";
                    break;
                case GfeSDK::NVGSDK_OVERLAY_STATE_PERMISSION:
                    m_lastOverlayEvent += L"Permission Overlay Window";
                    break;
                case GfeSDK::NVGSDK_OVERLAY_STATE_HIGHLIGHTS_SUMMARY:
                    m_lastOverlayEvent += L"Highlights Summary Overlay Window";
                    break;
                default:
                    m_lastOverlayEvent += L"UNKNOWNWindow";
            }
            m_lastOverlayEvent += (n.open ? L" OPEN" : L" CLOSE");
            break;
        }
        case GfeSDK::NVGSDK_NOTIFICATION_MAX:
            break;
    }
}


void Highlights::Wrapper::shutdown() {
    GfeSDK::Core *gfesdkCore = m_gfesdk.release();
    delete gfesdkCore;
}

void Highlights::Wrapper::tick() const {
    if (!m_gfesdk) {
        return;
    }
    m_gfesdk->Poll();
}

void Highlights::Wrapper::openGroup(JNIEnv *env, jstring groupId) {
    if (!m_highlights) return;
    auto groupStr = (env)->GetStringCritical(groupId, nullptr);
    auto id = UTF_16_to_8(groupStr);
    (env)->ReleaseStringCritical(groupId, groupStr);

    GfeSDK::HighlightOpenGroupParams params;
    params.groupId = id;
    params.groupDescriptionLocaleTable["en-US"] = id;
    m_highlights->OpenGroupAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });
}

void Highlights::Wrapper::closeGroup(JNIEnv *env, jstring groupId, jboolean destroy) {
    if (!m_highlights) return;
    auto groupStr = (env)->GetStringCritical(groupId, nullptr);

    GfeSDK::HighlightCloseGroupParams params;
    params.groupId = UTF_16_to_8(groupStr);
    params.destroyHighlights = destroy == JNI_TRUE;
    m_highlights->CloseGroupAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });
    (env)->ReleaseStringCritical(groupId, groupStr);
}

void Highlights::Wrapper::saveScreenShot(JNIEnv *env, jstring highlightId, jstring groupId) {
    auto highlightStr = (env)->GetStringCritical(highlightId, nullptr);
    auto groupStr = (env)->GetStringCritical(groupId, nullptr);

    GfeSDK::ScreenshotHighlightParams params;
    params.groupId = UTF_16_to_8(groupStr);
    params.highlightId = UTF_16_to_8(highlightStr);
    m_highlights->SetScreenshotHighlightAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });

    (env)->ReleaseStringCritical(highlightId, highlightStr);
    (env)->ReleaseStringCritical(groupId, groupStr);
}

void Highlights::Wrapper::saveVideo(JNIEnv *env, jstring highlightId, jstring groupId, jint startDelta,
                                    jint endDelta) {
    auto highlightStr = (env)->GetStringCritical(highlightId, nullptr);
    auto groupStr = (env)->GetStringCritical(groupId, nullptr);

    GfeSDK::VideoHighlightParams params;
    params.startDelta = static_cast<int>(startDelta);
    params.endDelta = static_cast<int>(endDelta);
    params.groupId = UTF_16_to_8(groupStr);
    params.highlightId = UTF_16_to_8(highlightStr);

    m_highlights->SetVideoHighlightAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });

    (env)->ReleaseStringCritical(highlightId, highlightStr);
    (env)->ReleaseStringCritical(groupId, groupStr);
}

// TODO Check this works
void
Highlights::Wrapper::getNumHighlights(JNIEnv *env, jstring groupId, jobject sigFilter, jobject tagFilter) {
    // TODO CACHE these fieldscz
    jclass sigFilterClass = env->GetObjectClass(sigFilter);
    jmethodID sigFilterValueMethodId = env->GetMethodID(sigFilterClass, "value", "()I");
    auto sigFilterValue = static_cast<int>(env->CallIntMethod(sigFilter, sigFilterValueMethodId));
    auto sigFilterEnum = (GfeSDK::NVGSDK_HighlightSignificance) sigFilterValue;

    jclass tagFilterClass = env->GetObjectClass(tagFilter);
    jmethodID tagFilterValueMethodId = env->GetMethodID(tagFilterClass, "value", "()I");
    auto tagFilterValue = static_cast<int>(env->CallIntMethod(tagFilter, tagFilterValueMethodId));
    auto tagFilterEnum = (GfeSDK::NVGSDK_HighlightType) tagFilterValue;

    auto groupStr = (env)->GetStringCritical(groupId, nullptr);

    GfeSDK::GroupView v;
    v.groupId = UTF_16_to_8(groupStr);
    v.significanceFilter = sigFilterEnum;
    v.tagsFilter = tagFilterEnum;

    m_highlights->GetNumberOfHighlightsAsync(v, [this](GfeSDK::NVGSDK_RetCode rc,
                                                       GfeSDK::GetNumberOfHighlightsResponse const *response, void *) {
        UpdateLastResultString(rc);
        if (GfeSDK::NVGSDK_SUCCEEDED(rc)) {
            m_lastQueryResult = std::to_wstring(response->numHighlights);
        }
    });

    (env)->ReleaseStringCritical(groupId, groupStr);
}

// TODO Check this works
void Highlights::Wrapper::openSummary(JNIEnv *env, jobjectArray groupIds,
                                      jint numGroups,
                                      jobject sigFilter, jobject tagFilter) {
    jclass sigFilterClass = env->GetObjectClass(sigFilter);
    jmethodID sigFilterValueMethodId = env->GetMethodID(sigFilterClass, "value", "()I");
    auto sigFilterValue = static_cast<int>(env->CallIntMethod(sigFilter, sigFilterValueMethodId));
    auto sigFilterEnum = (GfeSDK::NVGSDK_HighlightSignificance) sigFilterValue;

    // TODO CACHE these fieldscz
    jclass tagFilterClass = env->GetObjectClass(tagFilter);
    jmethodID tagFilterValueMethodId = env->GetMethodID(tagFilterClass, "value", "()I");
    auto tagFilterValue = static_cast<int>(env->CallIntMethod(tagFilter, tagFilterValueMethodId));
    auto tagFilterEnum = (GfeSDK::NVGSDK_HighlightType) tagFilterValue;

    GfeSDK::SummaryParams params;

    for (int i = 0; i < numGroups; i++) {
        auto groupId = (jstring) env->GetObjectArrayElement(groupIds, i);
        auto groupIdStr = (env)->GetStringCritical(groupId, nullptr);
        (env)->ReleaseStringCritical(groupId, groupIdStr);

        GfeSDK::GroupView v;
        v.groupId = UTF_16_to_8(groupIdStr);
        v.significanceFilter = sigFilterEnum;
        v.tagsFilter = tagFilterEnum;
        params.groupViews.push_back(v);
    }

    m_highlights->OpenSummaryAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });
}

void Highlights::Wrapper::requestLanguage() {
    m_gfesdk->GetUILanguageAsync(
            [this](GfeSDK::NVGSDK_RetCode rc, GfeSDK::GetUILanguageResponse const *response, void *context) {
                // Passed this pointer through as context
                auto *thiz = reinterpret_cast<Highlights::Wrapper *>(context);

                UpdateLastResultString(rc);
                if (GfeSDK::NVGSDK_SUCCEEDED(rc)) {
                    m_lastQueryResult = m_converter.from_bytes(response->cultureCode);
                }
            }, this);
}

void Highlights::Wrapper::requestUserSettings() {
    m_highlights->GetUserSettingsAsync(
            [this](GfeSDK::NVGSDK_RetCode rc, GfeSDK::GetUserSettingsResponse const *response, void *) {
                UpdateLastResultString(rc);
                m_lastQueryResult = L"";
                if (GfeSDK::NVGSDK_SUCCEEDED(rc)) {
                    for (const auto &highlightSetting: response->highlightSettings) {
                        m_lastQueryResult +=
                                L"\n" + m_converter.from_bytes(highlightSetting.highlightId) +
                                (highlightSetting.enabled ? L": ON" : L": OFF");
                    }
                }
            });
}


wchar_t const *Highlights::Wrapper::GetCurrentPermissionStr() const {
    return m_currentPermission.c_str();
}

wchar_t const *Highlights::Wrapper::GetLastOverlayEvent() const {
    return m_lastOverlayEvent.c_str();
}

wchar_t const *Highlights::Wrapper::GetLastResult() const {
    return m_lastResult.c_str();
}

wchar_t const *Highlights::Wrapper::GetLastQueryResult() const {
    return m_lastQueryResult.c_str();
}

void Highlights::Wrapper::ConfigureHighlights(char const *defaultLocale, GfeSDK::NVGSDK_Highlight *highlights,
                                              size_t numHighlights) {
    // Create handle to highlights module
    m_highlights.reset(GfeSDK::Highlights::Create(m_gfesdk.get()));

    GfeSDK::HighlightConfigParams configParams;
    configParams.defaultLocale = defaultLocale;

    // Set up highlight definition table
    for (size_t i = 0; i < numHighlights; ++i) {
        GfeSDK::HighlightDefinition highlightDef;
        highlightDef.id = highlights[i].id;
        highlightDef.userDefaultInterest = highlights[i].userInterest;
        highlightDef.significance = highlights[i].significance;
        highlightDef.highlightTags = highlights[i].highlightTags;
        for (size_t j = 0; j < highlights[i].nameTableSize; ++j) {
            highlightDef.nameLocaleTable[highlights[i].nameTable[j].localeCode] = highlights[i].nameTable[j].localizedString;
        }

        configParams.highlightDefinitions.push_back(highlightDef);
    }

    m_highlights->ConfigureAsync(configParams, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });
}

void Highlights::Wrapper::UpdateLastResultString(GfeSDK::NVGSDK_RetCode rc) {
    m_lastResult = m_converter.from_bytes(GfeSDK::RetCodeToString(rc));
}


//               ▄█   ███▄▄▄▄      ▄█
//              ███   ███▀▀▀██▄   ███
//              ███   ███   ███   ███▌
//              ███   ███   ███   ███▌
//              ███   ███   ███   ███▌
//              ███   ███   ███   ███
//              ███   ███   ███   ███
//          █▄  ███    ▀█   █▀    █▀
//          ▀█▄▄█▀

void Highlights::Java_dev_fxe_highlights_Highlights_init(JNIEnv *env, jobject, jstring gameName, jobject highlights) {
    wrapper.init(env, gameName, highlights);
}

void Highlights::Java_dev_fxe_highlights_Highlights_shutdown(JNIEnv *, jobject) {
    wrapper.shutdown();
}

void Highlights::Java_dev_fxe_highlights_Highlights_tick(JNIEnv *, jobject) {
    wrapper.tick();
}

void Highlights::Java_dev_fxe_highlights_Highlights_openGroup(JNIEnv *env, jobject, jstring groupId) {
    wrapper.openGroup(env, groupId);
}

void
Highlights::Java_dev_fxe_highlights_Highlights_closeGroup(JNIEnv *env, jobject, jstring groupId, jboolean destroy) {
    wrapper.closeGroup(env, groupId, destroy);
}

void Highlights::Java_dev_fxe_highlights_Highlights_saveScreenShot(JNIEnv *env, jobject, jstring highlightId,
                                                                   jstring groupId) {
    wrapper.saveScreenShot(env, highlightId, groupId);
}

void
Highlights::Java_dev_fxe_highlights_Highlights_saveVideo(JNIEnv *env, jobject, jstring highlightId, jstring groupId,
                                                         jint startDelta, jint endDelta) {
    wrapper.saveVideo(env, highlightId, groupId, startDelta, endDelta);
}

void Highlights::Java_dev_fxe_highlights_Highlights_getNumHighlights(JNIEnv *env, jobject, jstring groupId,
                                                                     jobject sigFilter, jobject tagFilter) {
    wrapper.getNumHighlights(env, groupId, sigFilter, tagFilter);
}

void
Highlights::Java_dev_fxe_highlights_Highlights_openSummary(JNIEnv *env, jobject, jobjectArray groupIds, jint numGroups,
                                                           jobject sigFilter, jobject tagFilter) {
    wrapper.openSummary(env, groupIds, numGroups, sigFilter, tagFilter);
}

void Highlights::Java_dev_fxe_highlights_Highlights_requestLanguage(JNIEnv *, jobject) {
    wrapper.requestLanguage();
}

void Highlights::Java_dev_fxe_highlights_Highlights_requestUserSettings(JNIEnv *, jobject) {
    wrapper.requestUserSettings();
}
