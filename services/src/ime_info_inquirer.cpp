/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ime_info_inquirer.h"

#include <algorithm>
#include <string>

#include "application_info.h"
#include "bundle_mgr_client_impl.h"
#include "global.h"
#include "if_system_ability_manager.h"
#include "ime_cfg_manager.h"
#include "input_method_info.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "string_ex.h"
#include "system_ability.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {
namespace {
using json = nlohmann::json;
using namespace OHOS::AppExecFwk;
constexpr const char *SUBTYPE_PROFILE_METADATA_NAME = "ohos.extension.input_method";
constexpr uint32_t SUBTYPE_PROFILE_NUM = 1;
constexpr uint32_t MAX_SUBTYPE_NUM = 10;
constexpr const char *DEFAULT_IME_KEY = "persist.sys.default_ime";
constexpr int32_t CONFIG_LEN = 128;
constexpr uint32_t RETRY_INTERVAL = 100;
constexpr uint32_t BLOCK_RETRY_TIMES = 1000;
} // namespace
ImeInfoInquirer &ImeInfoInquirer::GetInstance()
{
    static ImeInfoInquirer instance;
    return instance;
}

bool ImeInfoInquirer::QueryImeExtInfos(const int32_t userId, std::vector<ExtensionAbilityInfo> &infos)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        IMSA_HILOGE("GetBundleMgr failed");
        return false;
    }
    return bundleMgr->QueryExtensionAbilityInfos(ExtensionAbilityType::INPUTMETHOD, userId, infos);
}

int32_t ImeInfoInquirer::GetExtInfosByBundleName(
    const int32_t userId, const std::string &bundleName, std::vector<AppExecFwk::ExtensionAbilityInfo> &extInfos)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> tempExtInfos;
    BlockRetry(RETRY_INTERVAL, BLOCK_RETRY_TIMES,
        [this, &userId, &tempExtInfos]() -> bool { return QueryImeExtInfos(userId, tempExtInfos); });
    for (const auto &extInfo : tempExtInfos) {
        if (extInfo.bundleName == bundleName) {
            extInfos.emplace_back(extInfo);
        }
    }
    if (extInfos.empty()) {
        IMSA_HILOGE("bundleName: %{public}s extInfos is empty", bundleName.c_str());
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetImeInfo(
    const int32_t userId, const std::string &bundleName, const std::string &subName, ImeInfo &info)
{
    // if current ime, get info from currentImeInfos_
    std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
    if (currentImeInfo_ != nullptr && bundleName == currentImeInfo_->prop.name) {
        return GetImeInfoFromNative(userId, subName, info);
    }
    return GetImeInfoFromBundleMgr(userId, bundleName, subName, info);
}

int32_t ImeInfoInquirer::GetImeInfoFromNative(const int32_t userId, const std::string &subName, ImeInfo &info)
{
    IMSA_HILOGD("userId: %{public}d, subName: %{public}s", userId, subName.c_str());
    std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
    if (currentImeInfo_ == nullptr) {
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    info.subProps = currentImeInfo_->subProps;
    info.prop = currentImeInfo_->prop;
    info.isNewIme = currentImeInfo_->isNewIme;
    if (subName == currentImeInfo_->subProp.id) {
        info.subProp = currentImeInfo_->subProp;
        return ErrorCode::NO_ERROR;
    }

    auto it = std::find_if(currentImeInfo_->subProps.begin(), currentImeInfo_->subProps.end(),
        [&subName](const SubProperty &subProp) { return subProp.id == subName; });
    if (it == currentImeInfo_->subProps.end()) {
        IMSA_HILOGE("Find subName: %{public}s failed", subName.c_str());
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    info.subProp = *it;
    // old ime, make the id of prop same with the id of subProp.
    if (!info.isNewIme) {
        info.prop.id = info.subProp.id;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::GetImeInfoFromBundleMgr(
    const int32_t userId, const std::string &bundleName, const std::string &subName, ImeInfo &info)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, subName: %{public}s", userId, bundleName.c_str(),
        subName.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, bundleName.c_str());
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    info.prop.name = extInfos[0].bundleName;
    info.prop.id = extInfos[0].name;
    info.prop.label =
        GetStringById(extInfos[0].bundleName, extInfos[0].moduleName, extInfos[0].applicationInfo.labelId, userId);
    info.prop.labelId = extInfos[0].applicationInfo.labelId;
    info.prop.iconId = extInfos[0].applicationInfo.iconId;

    std::vector<SubProperty> subProps;
    info.isNewIme = IsNewExtInfos(extInfos);
    ret = info.isNewIme ? ListInputMethodSubtype(userId, extInfos[0], subProps)
                        : ListInputMethodSubtype(userId, extInfos, subProps);
    if (ret != ErrorCode::NO_ERROR || subProps.empty()) {
        IMSA_HILOGE("userId: %{public}d listInputMethodSubtype failed", userId);
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    info.subProps = subProps;
    if (subName.empty()) {
        info.subProp = subProps[0];
    } else {
        auto it = std::find_if(subProps.begin(), subProps.end(),
            [&subName](const SubProperty &subProp) { return subProp.id == subName; });
        if (it == subProps.end()) {
            IMSA_HILOGE("Find subName: %{public}s failed", subName.c_str());
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        info.subProp = *it;
    }
    // old ime, make the id of prop same with the id of subProp.
    if (!info.isNewIme) {
        info.prop.id = info.subProp.id;
    }
    return ErrorCode::NO_ERROR;
}

void ImeInfoInquirer::SetCurrentImeInfo(const ImeInfo &info)
{
    std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
    currentImeInfo_ = std::make_shared<ImeInfo>(info);
}

void ImeInfoInquirer::SetCurrentImeInfo(const int32_t userId)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    ImeInfo info;
    auto ret = GetImeInfoFromBundleMgr(userId, currentImeCfg->bundleName, currentImeCfg->subName, info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d, bundleName: %{public}s, subName: %{public}s getImeInfoFromBundleMgr failed",
            userId, currentImeCfg->bundleName.c_str(), currentImeCfg->subName.c_str());
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
    currentImeInfo_ = std::make_shared<ImeInfo>(info);
}

void ImeInfoInquirer::ResetCurrentImeInfo()
{
    std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
    currentImeInfo_ = nullptr;
}

std::string ImeInfoInquirer::GetInputMethodParam(const int32_t userId)
{
    auto properties = ListInputMethodInfo(userId);
    if (properties.empty()) {
        return "";
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    bool isBegin = true;
    std::string params = "{\"imeList\":[";
    for (const auto &property : properties) {
        params += isBegin ? "" : "},";
        isBegin = false;

        std::string imeId = property.mPackageName + "/" + property.mAbilityName;
        params += "{\"ime\": \"" + imeId + "\",";
        params += "\"labelId\": \"" + std::to_string(property.labelId) + "\",";
        params += "\"descriptionId\": \"" + std::to_string(property.descriptionId) + "\",";
        std::string isCurrentIme = currentImeCfg->imeId == imeId ? "true" : "false";
        params += "\"isCurrentIme\": \"" + isCurrentIme + "\",";
        params += "\"label\": \"" + property.label + "\",";
        params += "\"description\": \"" + property.description + "\"";
    }
    params += "}]}";
    return params;
}

std::vector<InputMethodInfo> ImeInfoInquirer::ListInputMethodInfo(const int32_t userId)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    if (!QueryImeExtInfos(userId, extensionInfos)) {
        IMSA_HILOGE("userId: %{public}d queryImeExtInfos failed", userId);
        return {};
    }
    std::vector<InputMethodInfo> properties;
    for (const auto &extension : extensionInfos) {
        auto applicationInfo = extension.applicationInfo;
        auto label = GetStringById(extension.bundleName, extension.moduleName, applicationInfo.labelId, userId);
        auto description =
            GetStringById(extension.bundleName, extension.moduleName, applicationInfo.descriptionId, userId);
        InputMethodInfo property;
        property.mPackageName = extension.bundleName;
        property.mAbilityName = extension.name;
        property.labelId = applicationInfo.labelId;
        property.descriptionId = applicationInfo.descriptionId;
        property.label = label;
        property.description = description;
        properties.emplace_back(property);
    }
    return properties;
}

int32_t ImeInfoInquirer::ListInputMethod(
    const int32_t userId, const InputMethodStatus status, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d, status: %{public}d", userId, status);
    if (status == InputMethodStatus::ALL) {
        return ListInputMethod(userId, props);
    }
    if (status == InputMethodStatus::ENABLE) {
        return ListEnabledInputMethod(userId, props);
    }
    if (status == InputMethodStatus::DISABLE) {
        return ListDisabledInputMethod(userId, props);
    }
    return ErrorCode::ERROR_BAD_PARAMETERS;
}

int32_t ImeInfoInquirer::ListInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    std::vector<ExtensionAbilityInfo> extensionInfos;
    BlockRetry(RETRY_INTERVAL, BLOCK_RETRY_TIMES,
        [this, &userId, &extensionInfos]() -> bool { return QueryImeExtInfos(userId, extensionInfos); });
    for (const auto &extension : extensionInfos) {
        auto it = std::find_if(props.begin(), props.end(),
            [&extension](const Property &prop) { return prop.name == extension.bundleName; });
        if (it != props.end()) {
            continue;
        }
        props.push_back({ .name = extension.bundleName,
            .id = extension.name,
            .label =
                GetStringById(extension.bundleName, extension.moduleName, extension.applicationInfo.labelId, userId),
            .labelId = extension.applicationInfo.labelId,
            .iconId = extension.applicationInfo.iconId });
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListEnabledInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto prop = GetCurrentInputMethod(userId);
    if (prop == nullptr) {
        IMSA_HILOGI("userId: %{public}d getCurrentInputMethod failed", userId);
        return ErrorCode::ERROR_NULL_POINTER;
    }
    props = { *prop };
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListDisabledInputMethod(const int32_t userId, std::vector<Property> &props)
{
    IMSA_HILOGD("userId: %{public}d", userId);
    auto ret = ListInputMethod(userId, props);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d listInputMethod failed", userId);
        return ret;
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    for (auto iter = props.begin(); iter != props.end();) {
        if (iter->name == currentImeCfg->bundleName) {
            iter = props.erase(iter);
            continue;
        }
        ++iter;
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(
    const int32_t userId, const std::string &bundleName, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    {
        std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
        if (currentImeInfo_ != nullptr && currentImeInfo_->prop.name == bundleName) {
            subProps = currentImeInfo_->subProps;
            return ErrorCode::NO_ERROR;
        }
    }
    std::vector<ExtensionAbilityInfo> extInfos;
    auto ret = GetExtInfosByBundleName(userId, bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE("userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, bundleName.c_str());
        return ret;
    }
    return IsNewExtInfos(extInfos) ? ListInputMethodSubtype(userId, extInfos[0], subProps)
                                   : ListInputMethodSubtype(userId, extInfos, subProps);
}

int32_t ImeInfoInquirer::ListCurrentInputMethodSubtype(const int32_t userId, std::vector<SubProperty> &subProps)
{
    // If the local currentImeInfo_ exists, the value is taken directly form currentImeInfo_
    {
        std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
        if (currentImeInfo_ != nullptr) {
            subProps = currentImeInfo_->subProps;
            return ErrorCode::NO_ERROR;
        }
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s", currentImeCfg->imeId.c_str());
    return ListInputMethodSubtype(userId, currentImeCfg->bundleName, subProps);
}

bool ImeInfoInquirer::IsNewExtInfos(const std::vector<ExtensionAbilityInfo> &extInfos)
{
    if (extInfos.empty()) {
        IMSA_HILOGE("extInfos is empty");
        return false;
    }
    auto iter = std::find_if(extInfos[0].metadata.begin(), extInfos[0].metadata.end(),
        [](const Metadata &metadata) { return metadata.name == SUBTYPE_PROFILE_METADATA_NAME; });
    return iter != extInfos[0].metadata.end();
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(
    const int32_t userId, const std::vector<ExtensionAbilityInfo> &extInfos, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("oldIme, userId: %{public}d", userId);
    for (const auto &extInfo : extInfos) {
        SubProperty subProperty;
        subProperty.labelId = extInfo.labelId;
        subProperty.label = GetStringById(extInfo.bundleName, extInfo.moduleName, extInfo.labelId, userId);
        subProperty.id = extInfo.name;
        subProperty.name = extInfo.bundleName;
        subProperty.iconId = extInfo.iconId;
        std::vector<Metadata> extends = extInfo.metadata;
        auto property = GetExtends(extends);
        subProperty.language = property.language;
        subProperty.mode = property.mode;
        subProperty.locale = property.locale;
        subProperty.icon = property.icon;
        subProps.emplace_back(subProperty);
    }
    return ErrorCode::NO_ERROR;
}

int32_t ImeInfoInquirer::ListInputMethodSubtype(
    const int32_t userId, const ExtensionAbilityInfo &extInfo, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("newIme, userId: %{public}d", userId);
    auto iter = std::find_if(extInfo.metadata.begin(), extInfo.metadata.end(),
        [](const Metadata &metadata) { return metadata.name == SUBTYPE_PROFILE_METADATA_NAME; });
    if (iter == extInfo.metadata.end()) {
        IMSA_HILOGE("find metadata name:SUBTYPE_PROFILE_METADATA_NAME failed");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    OHOS::AppExecFwk::BundleMgrClientImpl clientImpl;
    std::vector<std::string> profiles;
    if (!clientImpl.GetResConfigFile(extInfo, iter->name, profiles)) {
        IMSA_HILOGE("GetProfileFromExtension failed");
        return ErrorCode::ERROR_PACKAGE_MANAGER;
    }
    if (!ParseSubProp(profiles, subProps)) {
        IMSA_HILOGE("ParseSubProp failed");
        return ErrorCode::ERROR_BAD_PARAMETERS;
    }
    IMSA_HILOGD("subProps size: %{public}zu", subProps.size());
    for (auto &subProp : subProps) {
        subProp.name = extInfo.bundleName;
        auto pos = subProp.label.find(':');
        if (pos != std::string::npos && pos + 1 < subProp.label.size()) {
            subProp.labelId = atoi(subProp.label.substr(pos + 1).c_str());
            subProp.label = GetStringById(extInfo.bundleName, extInfo.moduleName, subProp.labelId, userId);
        }
        pos = subProp.icon.find(':');
        if (pos != std::string::npos && pos + 1 < subProp.icon.size()) {
            subProp.iconId = atoi(subProp.icon.substr(pos + 1).c_str());
        }
    }
    return ErrorCode::NO_ERROR;
}

std::string ImeInfoInquirer::GetStringById(
    const std::string &bundleName, const std::string &moduleName, int32_t labelId, int32_t userId)
{
    auto bundleMgr = GetBundleMgr();
    return bundleMgr == nullptr ? "" : bundleMgr->GetStringById(bundleName, moduleName, labelId, userId);
}

SubProperty ImeInfoInquirer::GetExtends(const std::vector<Metadata> &metaData)
{
    SubProperty property;
    for (const auto &data : metaData) {
        if (data.name == "language") {
            property.language = data.value;
            continue;
        }
        if (data.name == "mode") {
            property.mode = data.value;
            continue;
        }
        if (data.name == "locale") {
            property.locale = data.value;
            continue;
        }
        if (data.name == "icon") {
            property.icon = data.value;
        }
    }
    return property;
}

std::shared_ptr<Property> ImeInfoInquirer::GetCurrentInputMethod(const int32_t userId)
{
    // If the local currentImeInfo_ exists, the value is taken directly form currentImeInfo_
    {
        std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
        if (currentImeInfo_ != nullptr) {
            return std::make_shared<Property>(currentImeInfo_->prop);
        }
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s", currentImeCfg->imeId.c_str());
    std::vector<AppExecFwk::ExtensionAbilityInfo> extInfos;
    auto ret = ImeInfoInquirer::GetInstance().GetExtInfosByBundleName(userId, currentImeCfg->bundleName, extInfos);
    if (ret != ErrorCode::NO_ERROR || extInfos.empty()) {
        IMSA_HILOGE(
            "userId: %{public}d getExtInfosByBundleName %{public}s failed", userId, currentImeCfg->bundleName.c_str());
        return nullptr;
    }
    Property prop = { .name = extInfos[0].bundleName,
        .id = currentImeCfg->extName, // if old ime, the extInfos[0].name maybe not same with currentImeExtName
        .label =
            GetStringById(extInfos[0].bundleName, extInfos[0].moduleName, extInfos[0].applicationInfo.labelId, userId),
        .labelId = extInfos[0].applicationInfo.labelId,
        .iconId = extInfos[0].applicationInfo.iconId };
    return std::make_shared<Property>(prop);
}

std::shared_ptr<SubProperty> ImeInfoInquirer::GetCurrentInputMethodSubtype(const int32_t userId)
{
    // If the local currentImeInfo_ exists, the value is taken directly form currentImeInfo_
    {
        std::lock_guard<std::recursive_mutex> lock(currentImeInfoLock_);
        if (currentImeInfo_ != nullptr) {
            return std::make_shared<SubProperty>(currentImeInfo_->subProp);
        }
    }
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("currentIme: %{public}s", currentImeCfg->imeId.c_str());
    std::vector<SubProperty> subProps = {};
    auto ret = ListInputMethodSubtype(userId, currentImeCfg->bundleName, subProps);
    if (ret != ErrorCode::NO_ERROR || subProps.empty()) {
        IMSA_HILOGE("userId: %{public}d listInputMethodSubtype by bundleName: %{public}s failed", userId,
            currentImeCfg->bundleName.c_str());
        return nullptr;
    }
    auto it = std::find_if(subProps.begin(), subProps.end(),
        [&currentImeCfg](const SubProperty &subProp) { return subProp.id == currentImeCfg->subName; });
    if (it != subProps.end()) {
        return std::make_shared<SubProperty>(*it);
    }
    IMSA_HILOGE("Find subName: %{public}s failed", currentImeCfg->subName.c_str());
    return std::make_shared<SubProperty>(subProps[0]);
}

bool ImeInfoInquirer::IsImeInstalled(const int32_t userId, const std::string &bundleName, const std::string &extName)
{
    IMSA_HILOGD("userId: %{public}d, bundleName: %{public}s, extName: %{public}s", userId, bundleName.c_str(),
        extName.c_str());
    std::vector<OHOS::AppExecFwk::ExtensionAbilityInfo> extInfos;
    GetExtInfosByBundleName(userId, bundleName, extInfos);
    auto iter = std::find_if(extInfos.begin(), extInfos.end(),
        [&bundleName, &extName](const OHOS::AppExecFwk::ExtensionAbilityInfo &extInfo) {
            return extInfo.bundleName == bundleName && extName == extInfo.name;
        });
    if (iter == extInfos.end()) {
        IMSA_HILOGE("false");
        return false;
    }
    IMSA_HILOGI("true");
    return true;
}

std::string ImeInfoInquirer::GetStartedIme(const int32_t userId)
{
    auto currentImeCfg = ImeCfgManager::GetInstance().GetCurrentImeCfg(userId);
    IMSA_HILOGD("userId: %{public}d, currentIme: %{public}s", userId, currentImeCfg->imeId.c_str());
    if (currentImeCfg->imeId.empty() || !IsImeInstalled(userId, currentImeCfg->bundleName, currentImeCfg->extName)) {
        auto newUserIme = GetDefaultIme();
        std::string subName;
        auto info = GetDefaultImeInfo(userId);
        if (info == nullptr) {
            IMSA_HILOGE("GetDefaultImeInfo failed");
            subName = "";
        } else {
            subName = info->subProp.id;
            SetCurrentImeInfo(*info);
        }
        currentImeCfg->imeId.empty() ? ImeCfgManager::GetInstance().AddImeCfg({ userId, newUserIme, subName })
                                     : ImeCfgManager::GetInstance().ModifyImeCfg({ userId, newUserIme, subName });
        return newUserIme;
    }
    // service start, user switch, set the currentImeInfo_.
    SetCurrentImeInfo(userId);
    return currentImeCfg->imeId;
}

std::shared_ptr<ImeInfo> ImeInfoInquirer::GetDefaultImeInfo(const int32_t userId)
{
    auto ime = GetDefaultIme();
    auto pos = ime.find('/');
    if (pos == std::string::npos || pos + 1 >= ime.size()) {
        IMSA_HILOGE("defaultIme: %{public}s is abnormal", ime.c_str());
        return nullptr;
    }
    auto bundleName = ime.substr(0, pos);
    auto extName = ime.substr(pos + 1);
    {
        std::lock_guard<std::mutex> lock(defaultImeInfoLock_);
        if (defaultImeInfo_ != nullptr && defaultImeInfo_->prop.name == bundleName
            && defaultImeInfo_->prop.id == extName) {
            return defaultImeInfo_;
        }
    }
    ImeInfo info;
    auto ret = GetImeInfoFromBundleMgr(userId, bundleName, "", info);
    if (ret != ErrorCode::NO_ERROR) {
        IMSA_HILOGE(
            "userId: %{public}d, bundleName: %{public}s getImeInfoFromBundleMgr failed", userId, bundleName.c_str());
        return nullptr;
    }
    if (!info.isNewIme) {
        info.prop.id = extName;
        auto it = std::find_if(info.subProps.begin(), info.subProps.end(),
            [&extName](const SubProperty &subProp) { return subProp.id == extName; });
        if (it != info.subProps.end()) {
            info.subProp = *it;
        }
    }
    std::lock_guard<std::mutex> lock(defaultImeInfoLock_);
    defaultImeInfo_ = std::make_shared<ImeInfo>(info);
    return defaultImeInfo_;
}

std::string ImeInfoInquirer::GetDefaultIme()
{
    char value[CONFIG_LEN] = { 0 };
    auto code = GetParameter(DEFAULT_IME_KEY, "", value, CONFIG_LEN);
    return code > 0 ? value : "";
}

sptr<OHOS::AppExecFwk::IBundleMgr> ImeInfoInquirer::GetBundleMgr()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        IMSA_HILOGE("systemAbilityManager is nullptr");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        IMSA_HILOGE("remoteObject is nullptr");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

std::shared_ptr<SubProperty> ImeInfoInquirer::GetImeSubProp(
    const std::vector<SubProperty> &subProps, const Condition &condition)
{
    auto it = subProps.end();
    switch (condition) {
        case Condition::UPPER: {
            it = std::find_if(
                subProps.begin(), subProps.end(), [](const SubProperty &subProp) { return subProp.mode == "upper"; });
            break;
        }
        case Condition::LOWER: {
            it = std::find_if(
                subProps.begin(), subProps.end(), [](const SubProperty &subProp) { return subProp.mode == "lower"; });
            break;
        }
        case Condition::LANGUAGE_EN: {
            it = std::find_if(subProps.begin(), subProps.end(),
                [](const SubProperty &subProp) { return subProp.language == "english" && subProp.mode == "lower"; });
            break;
        }
        case Condition::LANGUAGE_CH: {
            it = std::find_if(subProps.begin(), subProps.end(),
                [](const SubProperty &subProp) { return subProp.language == "chinese"; });
            break;
        }
        case Condition::LOCALE_CH: {
            it = std::find_if(subProps.begin(), subProps.end(),
                [](const SubProperty &subProp) { return subProp.locale == "zh_CN"; });
            break;
        }
        case Condition::LOCALE_EN: {
            it = std::find_if(subProps.begin(), subProps.end(),
                [](const SubProperty &subProp) { return subProp.locale == "en_US" && subProp.mode == "lower"; });
            break;
        }
        default: {
            break;
        }
    }
    if (it == subProps.end()) {
        return nullptr;
    }
    return std::make_shared<SubProperty>(*it);
}

bool ImeInfoInquirer::ParseSubProp(const std::vector<std::string> &profiles, std::vector<SubProperty> &subProps)
{
    if (profiles.empty() || profiles.size() != SUBTYPE_PROFILE_NUM) {
        IMSA_HILOGE("profiles size: %{public}zu", profiles.size());
        return false;
    }
    json jsonSubProps;
    SubProperty subProp;
    IMSA_HILOGD("profiles[0]: %{public}s", profiles[0].c_str());
    jsonSubProps = json::parse(profiles[0], nullptr, false);
    if (jsonSubProps.is_null() || jsonSubProps.is_discarded() || !jsonSubProps.contains("subtypes")
        || !jsonSubProps["subtypes"].is_array() || jsonSubProps["subtypes"].empty()) {
        IMSA_HILOGE("json parse failed");
        return false;
    }
    ParseSubProp(jsonSubProps, subProps);
    return true;
}

void ImeInfoInquirer::ParseSubProp(const json &jsonSubProps, std::vector<SubProperty> &subProps)
{
    IMSA_HILOGD("subType num: %{public}zu", jsonSubProps["subtypes"].size());
    for (auto &jsonCfg : jsonSubProps["subtypes"]) {
        if (subProps.size() >= MAX_SUBTYPE_NUM) {
            break;
        }
        SubProperty subProp;
        ParseSubProp(jsonCfg, subProp);
        subProps.push_back(subProp);
    }
}

void ImeInfoInquirer::ParseSubProp(const json &jsonSubProp, SubProperty &subProp)
{
    // label: 子类型对外显示名称
    if (jsonSubProp.find("label") != jsonSubProp.end() && jsonSubProp["label"].is_string()) {
        jsonSubProp.at("label").get_to(subProp.label);
    }
    if (jsonSubProp.find("labelId") != jsonSubProp.end() && jsonSubProp["labelId"].is_number()) {
        jsonSubProp.at("labelId").get_to(subProp.labelId);
    }
    // id: 子类型名
    if (jsonSubProp.find("id") != jsonSubProp.end() && jsonSubProp["id"].is_string()) {
        jsonSubProp.at("id").get_to(subProp.id);
    }

    if (jsonSubProp.find("icon") != jsonSubProp.end() && jsonSubProp["icon"].is_string()) {
        jsonSubProp.at("icon").get_to(subProp.icon);
    }
    if (jsonSubProp.find("iconId") != jsonSubProp.end() && jsonSubProp["iconId"].is_number()) {
        jsonSubProp.at("iconId").get_to(subProp.iconId);
    }
    if (jsonSubProp.find("mode") != jsonSubProp.end() && jsonSubProp["mode"].is_string()) {
        jsonSubProp.at("mode").get_to(subProp.mode);
    }
    if (jsonSubProp.find("locale") != jsonSubProp.end() && jsonSubProp["locale"].is_string()) {
        jsonSubProp.at("locale").get_to(subProp.locale);
    }
}
} // namespace MiscServices
} // namespace OHOS