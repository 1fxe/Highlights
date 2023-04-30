/* Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/


#include "GfeSDKWrapper.hpp"


#define LOG(fmt, ...) printf(fmt, __VA_ARGS__)

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

GfeSdkWrapper::GfeSdkWrapper() :
        m_currentPermission(permissionToStringW(GfeSDK::NVGSDK_PERMISSION_MUST_ASK)) {
}

void GfeSdkWrapper::Init(char const *gameName, GfeSDK::NVGSDK_Highlight *highlights,
                         size_t numHighlights) {
    using namespace std::placeholders;

    auto defaultLocale = "en-US";
    //! [Creation CPP]
    GfeSDK::CreateInputParams createParams;
    createParams.appName = gameName;
    createParams.pollForCallbacks = true;
    createParams.requiredScopes = {
            GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS,
            GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_VIDEO,
            GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_SCREENSHOT
    };
    createParams.notificationCallback = std::bind(&GfeSdkWrapper::OnNotification, this, _1, _2);

    GfeSDK::CreateResponse response;

    GfeSDK::Core *gfesdkCore = GfeSDK::Core::Create(createParams, response);
    if (GfeSDK::NVGSDK_SUCCEEDED(response.returnCode)) {
        // Valid handle has been returned
        LOG("Success: %s\n", GfeSDK::NVGSDK_RetCodeToString(response.returnCode));
        LOG("PC is running GFE version %s\n", response.nvidiaGfeVersion.c_str());
        LOG("PC is running GfeSDK version %d.%d\n", response.versionMajor, response.versionMinor);
        switch (response.returnCode) {
            case GfeSDK::NVGSDK_SUCCESS_VERSION_OLD_GFE:
                break;
            case GfeSDK::NVGSDK_SUCCESS_VERSION_OLD_SDK:
                break;
        }
    } else {
        // No valid handle
        LOG("Failure: %s\n", GfeSDK::NVGSDK_RetCodeToString(response.returnCode));
        switch (response.returnCode) {
            case GfeSDK::NVGSDK_ERR_SDK_VERSION:
                LOG("PC is running GFE version %s\n", response.nvidiaGfeVersion.c_str());
                LOG("PC is running GfeSDK version %d.%d\n", response.versionMajor, response.versionMinor);
                break;
            case GfeSDK::NVGSDK_SUCCESS_VERSION_OLD_SDK:
                LOG("PC is running GFE version %s\n", response.nvidiaGfeVersion.c_str());
                LOG("PC is running GfeSDK version %d.%d\n", response.versionMajor, response.versionMinor);
                break;
        }
        return;
    }
    //! [Creation CPP]

    m_gfesdk.reset(gfesdkCore);
    UpdateLastResultString(response.returnCode);

    if (response.scopePermissions.find(GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_VIDEO) != response.scopePermissions.end()) {
        m_currentPermission = permissionToStringW(response.scopePermissions[GfeSDK::NVGSDK_SCOPE_HIGHLIGHTS_VIDEO]);
    }

    //! [Permissions CPP]
    // Request Permissions if user hasn't decided yet
    GfeSDK::RequestPermissionsParams requestPermissionsParams;

    // 'response' came from create call. It tells us which permissions we requested during Create,
    // but the user hasn't yet made a decision on
    for (auto &&entry: response.scopePermissions) {
        if (entry.second == GfeSDK::NVGSDK_PERMISSION_MUST_ASK) {
            requestPermissionsParams.scopes.push_back(entry.first);
        }
    }

    if (!requestPermissionsParams.scopes.empty()) {
        // If the user hasn't given permission for recording yet, ask them to do so now via overlay
        m_gfesdk->RequestPermissionsAsync(requestPermissionsParams,
                                          [this, defaultLocale, highlights, numHighlights](GfeSDK::NVGSDK_RetCode rc,
                                                                                           void *cbContext) {
                                              UpdateLastResultString(rc);
                                              if (GfeSDK::NVGSDK_SUCCEEDED(rc)) {
                                                  ConfigureHighlights(defaultLocale, highlights, numHighlights);
                                              }
                                          });
    } else {
        // Otherwise, go ahead and set up now
        ConfigureHighlights(defaultLocale, highlights, numHighlights);
    }
    //! [Permissions CPP]
}

void GfeSdkWrapper::DeInit() {
    // Releasing from smart pointer just to document how to delete handle
    GfeSDK::Core *gfesdkCore = m_gfesdk.release();

    //! [Release CPP]
    delete gfesdkCore;
    //! [Release CPP]
}

void GfeSdkWrapper::OnTick() {
    if (!m_gfesdk) return;

    // Poll for callbacks in order to execute these callbacks from the game update thread,
    // not a third party thead owned by GfeSDK. This lets us update game state from the callbacks
    // without causing a threading problem
    m_gfesdk->Poll();
}

void GfeSdkWrapper::OnNotification(GfeSDK::NVGSDK_NotificationType type, GfeSDK::NotificationBase const &notification) {
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

void GfeSdkWrapper::OnOpenGroup(std::string const &id) {
    if (!m_highlights) return;

    LOG("Opened group %s\n", id.c_str());
    //! [OpenGroup CPP]
    GfeSDK::HighlightOpenGroupParams params;
    params.groupId = id;
    params.groupDescriptionLocaleTable["en-US"] = id;
    m_highlights->OpenGroupAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
        LOG("Open group %s", GfeSDK::RetCodeToString(rc));
    });
    //! [OpenGroup CPP]
}

void GfeSdkWrapper::OnCloseGroup(std::string const &id, bool destroy) {
    if (!m_highlights) return;

    //! [CloseGroup CPP]
    GfeSDK::HighlightCloseGroupParams params;
    params.groupId = id;
    params.destroyHighlights = destroy;
    m_highlights->CloseGroupAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });
    //! [CloseGroup CPP]

#ifdef DOXYGEN
    //! [Async Call No Callback]
    // If the caller doesn't care about the return value, no need to pass callbacks
    GfeSDK::HighlightCloseGroupParams params = { "GROUP_1" };
    m_highlights->CloseGroupAsync(params);
    //! [Async Call No Callback]
#endif
}

void GfeSdkWrapper::OnSaveScreenshot(std::string const &highlightId, std::string const &groupId) {
    GfeSDK::ScreenshotHighlightParams params;
    params.groupId = groupId;
    params.highlightId = highlightId;
    m_highlights->SetScreenshotHighlightAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });
}

void
GfeSdkWrapper::OnSaveVideo(std::string const &highlightId, std::string const &groupId, int startDelta, int endDelta) {
    //! [SaveVideo CPP]
    GfeSDK::VideoHighlightParams params;
    params.startDelta = startDelta;
    params.endDelta = endDelta;
    params.groupId = groupId;
    params.highlightId = highlightId;
    m_highlights->SetVideoHighlightAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
        LOG("Saving video %s", GfeSDK::RetCodeToString(rc));
    });
    //! [SaveVideo CPP]
}

void GfeSdkWrapper::OnGetNumHighlights(std::string const &groupId, GfeSDK::NVGSDK_HighlightSignificance sigFilter,
                                       GfeSDK::NVGSDK_HighlightType tagFilter) {
    GfeSDK::GroupView v;
    v.groupId = groupId;
    v.significanceFilter = sigFilter;
    v.tagsFilter = tagFilter;

    m_highlights->GetNumberOfHighlightsAsync(v, [this](GfeSDK::NVGSDK_RetCode rc,
                                                       GfeSDK::GetNumberOfHighlightsResponse const *response, void *) {
        UpdateLastResultString(rc);
        if (GfeSDK::NVGSDK_SUCCEEDED(rc)) {
            m_lastQueryResult = std::to_wstring(response->numHighlights);
        }
    });
}

void
GfeSdkWrapper::OnOpenSummary(char const *groupIds[], size_t numGroups, GfeSDK::NVGSDK_HighlightSignificance sigFilter,
                             GfeSDK::NVGSDK_HighlightType tagFilter) {
    //! [OpenSummary CPP]
    GfeSDK::SummaryParams params;

    // Can show more than one group at a time, each with their own filters if desired
    for (size_t i = 0; i < numGroups; ++i) {
        GfeSDK::GroupView v;
        v.groupId = groupIds[i];
        v.significanceFilter = sigFilter;
        v.tagsFilter = tagFilter;
        params.groupViews.push_back(v);
    }

    m_highlights->OpenSummaryAsync(params, [this](GfeSDK::NVGSDK_RetCode rc, void *) {
        UpdateLastResultString(rc);
    });
    //! [OpenSummary CPP]
}

void GfeSdkWrapper::OnRequestLanguage() {
    //! [Asynchonous Call]
    // We are passing two arguments to this function, the function lambda, and a user context. In this case, we're passing
    // the 'this' pointer as the user context. This gets passed through unmodified for use in the callback function.
    m_gfesdk->GetUILanguageAsync(
            [this](GfeSDK::NVGSDK_RetCode rc, GfeSDK::GetUILanguageResponse const *response, void *context) {
                // Passed this pointer through as context
                GfeSdkWrapper *thiz = reinterpret_cast<GfeSdkWrapper *>(context);

                UpdateLastResultString(rc);
                if (GfeSDK::NVGSDK_SUCCEEDED(rc)) {
                    m_lastQueryResult = m_converter.from_bytes(response->cultureCode);
                }
            }, this);
    //! [Asynchonous Call]
}

void GfeSdkWrapper::OnRequestUserSettings() {
    m_highlights->GetUserSettingsAsync(
            [this](GfeSDK::NVGSDK_RetCode rc, GfeSDK::GetUserSettingsResponse const *response, void *) {
                UpdateLastResultString(rc);
                m_lastQueryResult = L"";
                if (GfeSDK::NVGSDK_SUCCEEDED(rc)) {
                    for (const auto & highlightSetting : response->highlightSettings) {
                        m_lastQueryResult +=
                                L"\n" + m_converter.from_bytes(highlightSetting.highlightId) + (highlightSetting.enabled ? L": ON" : L": OFF");
                    }
                }
            });
}

wchar_t const *GfeSdkWrapper::GetCurrentPermissionStr() {
    return m_currentPermission.c_str();
}

wchar_t const *GfeSdkWrapper::GetLastOverlayEvent() {
    return m_lastOverlayEvent.c_str();
}

wchar_t const *GfeSdkWrapper::GetLastResult() {
    return m_lastResult.c_str();
}

wchar_t const *GfeSdkWrapper::GetLastQueryResult() {
    return m_lastQueryResult.c_str();
}

void GfeSdkWrapper::ConfigureHighlights(char const *defaultLocale, GfeSDK::NVGSDK_Highlight *highlights,
                                        size_t numHighlights) {
    //! [ConfigureHighlights CPP]
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
        LOG("Configure hightlights %s", GfeSDK::RetCodeToString(rc));
    });
    //! [ConfigureHighlights CPP]
}

void GfeSdkWrapper::UpdateLastResultString(GfeSDK::NVGSDK_RetCode rc) {
    m_lastResult = m_converter.from_bytes(GfeSDK::RetCodeToString(rc));
}
