/* Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once

#include <codecvt>
#include <locale>
#include <memory>

#include <cstring>
#include <gfesdk/bindings/cpp/isdk_cpp_impl.h>
#include <gfesdk/bindings/cpp/highlights/ihighlights_cpp_impl.h>

namespace Json
{
class Value;
}

class GfeSdkWrapper
{
public:
    GfeSdkWrapper();

    void Init(char const *gameName, GfeSDK::NVGSDK_Highlight *highlights,size_t numHighlights);
    void DeInit();
    void OnTick();
    void OnNotification(GfeSDK::NVGSDK_NotificationType type, GfeSDK::NotificationBase const&);
    void OnOpenGroup(std::string const& id);
    void OnCloseGroup(std::string const& id, bool destroy = false);
    void OnSaveScreenshot(std::string const& highlightId, std::string const& groupId);
    void OnSaveVideo(std::string const& highlightId, std::string const& groupId, int startDelta, int endDelta);
    void OnGetNumHighlights(std::string const& groupId, GfeSDK::NVGSDK_HighlightSignificance sigFilter, GfeSDK::NVGSDK_HighlightType tagFilter);
    void OnOpenSummary(char const* groupIds[], size_t numGroups, GfeSDK::NVGSDK_HighlightSignificance sigFilter, GfeSDK::NVGSDK_HighlightType tagFilter);
    void OnRequestLanguage();
    void OnRequestUserSettings();

    wchar_t const* GetCurrentPermissionStr();
    wchar_t const* GetLastOverlayEvent();
    wchar_t const* GetLastResult();
    wchar_t const* GetLastQueryResult();

private:
    void ConfigureHighlights(char const* defaultLocale, GfeSDK::NVGSDK_Highlight* highlights, size_t numHighlights);
    void UpdateLastResultString(GfeSDK::NVGSDK_RetCode rc);

    std::unique_ptr<GfeSDK::Core> m_gfesdk;
    std::unique_ptr<GfeSDK::Highlights> m_highlights;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> m_converter;
    std::wstring m_currentPermission;
    std::wstring m_lastOverlayEvent;
    std::wstring m_lastResult;
    std::wstring m_lastQueryResult;
};

inline void InitGfeSdkWrapper(GfeSdkWrapper* hl) {}
