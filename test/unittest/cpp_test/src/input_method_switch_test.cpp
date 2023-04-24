/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "accesstoken_kit.h"
#include "global.h"
#include "input_method_controller.h"
#include "input_method_property.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace MiscServices {
class InputMethodSwitchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void GrantNativePermission();
    static void CheckCurrentProp(const std::string &extName);
    static void CheckCurrentSubProp(const std::string &extName);
    static void CheckCurrentSubProps();
    static std::mutex imeChangeFlagLock;
    static std::condition_variable conditionVar;
    static sptr<InputMethodController> imc_;
    static bool imeChangeFlag;
    static std::string newImeBundleName;
    static std::vector<std::string> newImeSubName;
    static std::string bundleName;
    static std::vector<std::string> extName;
    static std::vector<std::string> language;
};
std::mutex InputMethodSwitchTest::imeChangeFlagLock;
std::condition_variable InputMethodSwitchTest::conditionVar;
bool InputMethodSwitchTest::imeChangeFlag = false;
sptr<InputMethodController> InputMethodSwitchTest::imc_;
std::string InputMethodSwitchTest::newImeBundleName = "com.example.newTestIme";
std::vector<std::string> InputMethodSwitchTest::newImeSubName{ "lowerInput", "upperInput", "chineseInput" };
std::string InputMethodSwitchTest::bundleName = "com.example.testIme";
std::vector<std::string> InputMethodSwitchTest::extName{ "InputMethodExtAbility", "InputMethodExtAbility2" };
std::vector<std::string> InputMethodSwitchTest::language{ "chinese", "english" };
constexpr uint32_t IME_EXT_NUM = 2;
constexpr uint32_t NEW_IME_SUBTYPE_NUM = 3;
constexpr uint32_t TOTAL_IME_MIN_NUM = 2;
constexpr uint32_t ENABLE_IME_NUM = 1;
constexpr uint32_t SUBTYPE_SWITCH_DELAY_TIME = 20;
constexpr uint32_t IME_SWITCH_DELAY_TIME = 200;
constexpr uint32_t WAIT_IME_READY_TIME = 1;
class InputMethodSettingListenerImpl : public InputMethodSettingListener {
public:
    InputMethodSettingListenerImpl() = default;
    ~InputMethodSettingListenerImpl() = default;
    void OnImeChange(const Property &property, const SubProperty &subProperty)
    {
        {
            std::unique_lock<std::mutex> lock(InputMethodSwitchTest::imeChangeFlagLock);
            InputMethodSwitchTest::imeChangeFlag = true;
        }
        InputMethodSwitchTest::conditionVar.notify_one();
        IMSA_HILOGI("InputMethodSettingListenerImpl OnImeChange");
    }
};

void InputMethodSwitchTest::SetUpTestCase(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::SetUpTestCase");
    GrantNativePermission();
    imc_ = InputMethodController::GetInstance();
    imc_->SetSettingListener(std::make_shared<InputMethodSettingListenerImpl>());
}

void InputMethodSwitchTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::TearDownTestCase");
    InputMethodController::GetInstance()->Close();
}

void InputMethodSwitchTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::SetUp");
}

void InputMethodSwitchTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodSwitchTest::TearDown");
}

void InputMethodSwitchTest::GrantNativePermission()
{
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.CONNECT_IME_ABILITY";
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "inputmethod_imf",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    int res = SetSelfTokenID(tokenId);
    if (res == 0) {
        IMSA_HILOGI("SetSelfTokenID success!");
    } else {
        IMSA_HILOGE("SetSelfTokenID fail!");
    }
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

void InputMethodSwitchTest::CheckCurrentProp(const std::string &extName)
{
    std::shared_ptr<Property> property = imc_->GetCurrentInputMethod();
    ASSERT_TRUE(property != nullptr);
    EXPECT_EQ(property->name, bundleName);
    EXPECT_EQ(property->id, extName);
}

void InputMethodSwitchTest::CheckCurrentSubProp(const std::string &extName)
{
    auto subProperty = imc_->GetCurrentInputMethodSubtype();
    ASSERT_TRUE(subProperty != nullptr);
    EXPECT_EQ(subProperty->id, extName);
    EXPECT_EQ(subProperty->name, bundleName);
}

void InputMethodSwitchTest::CheckCurrentSubProps()
{
    std::vector<SubProperty> subProps;
    auto ret = imc_->ListCurrentInputMethodSubtype(subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_EQ(subProps.size(), IME_EXT_NUM);
    for (int i = 0; i < IME_EXT_NUM; i++) {
        EXPECT_EQ(subProps[i].id, extName[i]);
        EXPECT_EQ(subProps[i].name, bundleName);
        EXPECT_EQ(subProps[i].language, language[i]);
        EXPECT_EQ(subProps[i].locale, "");
    }
}

/**
* @tc.name: testImeSwitch
* @tc.desc: switch to testIme
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testImeSwitch, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testImeSwitch Test START");
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    // switch to ext testIme
    auto ret = imc_->SwitchInputMethod(bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(lock, std::chrono::milliseconds(IME_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_TRUE(imeChangeFlag);
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
    sleep(WAIT_IME_READY_TIME);
}

/**
* @tc.name: testSubTypeSwitch_001
* @tc.desc: switch subtype with extName1
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSubTypeSwitch_001, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSubTypeSwitch_001 Test START");
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName, extName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_FALSE(imeChangeFlag);
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitch_002
* @tc.desc: switch subtype with extName2
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSubTypeSwitch_002, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSubTypeSwitch_002 Test START");
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName, extName[1]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_TRUE(imeChangeFlag);
    CheckCurrentProp(extName[1]);
    CheckCurrentSubProp(extName[1]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitch_003
* @tc.desc: switch subtype with extName1
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSubTypeSwitch_003, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSubTypeSwitch_002 Test START");
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName, extName[0]);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_TRUE(imeChangeFlag);
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSubTypeSwitchWithErrorSubName
* @tc.desc: switch subtype with error subName.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSubTypeSwitchWithErrorSubName, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSubTypeSwitchWithErrorSubName Test START");
    int32_t ret = imc_->SwitchInputMethod(bundleName, "error subName");
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchToCurrentImeWithEmptySubName
* @tc.desc: switch to currentIme witch empty subName.
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSwitchToCurrentImeWithEmptySubName, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSwitchToCurrentImeWithEmptySubName Test START");
    std::unique_lock<std::mutex> lock(imeChangeFlagLock);
    imeChangeFlag = false;
    int32_t ret = imc_->SwitchInputMethod(bundleName);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodSwitchTest::conditionVar.wait_for(
        lock, std::chrono::milliseconds(SUBTYPE_SWITCH_DELAY_TIME), [] { return imeChangeFlag == true; });
    EXPECT_FALSE(imeChangeFlag);
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchImeWithErrorBundleName
* @tc.desc: switch ime witch error bundleName
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSwitchImeWithErrorBundleName, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSwitchImeWithErrorBundleName Test START");
    int32_t ret = imc_->SwitchInputMethod("error bundleName", extName[0]);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testSwitchImeWithErrorBundleNameWitchEmptySubName
* @tc.desc: switch ime witch error bundleName and empty subName
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testSwitchImeWithErrorBundleNameWitchEmptySubName, TestSize.Level0)
{
    IMSA_HILOGI("oldIme testSwitchImeWithErrorBundleNameWitchEmptySubName Test START");
    int32_t ret = imc_->SwitchInputMethod("error bundleName", " ");
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    CheckCurrentProp(extName[0]);
    CheckCurrentSubProp(extName[0]);
    CheckCurrentSubProps();
}

/**
* @tc.name: testIMCListInputMethod
* @tc.desc: IMC ListInputMethod
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCListInputMethod, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethod Test Start");
    std::vector<Property> properties = {};
    auto ret = imc_->ListInputMethod(properties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_TRUE(properties.size() >= TOTAL_IME_MIN_NUM);
    bool hasIme = false;
    bool hasNewIme = false;
    for (const auto &property : properties) {
        if (property.name == bundleName) {
            hasIme = true;
        }
        if (property.name == newImeBundleName) {
            hasNewIme = true;
        }
    }
    EXPECT_TRUE(hasIme && hasNewIme);
}

/**
* @tc.name: testIMCListInputMethodDisable
* @tc.desc: IMC ListInputMethod
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCListInputMethodDisable, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethodDisable Test Start");
    std::vector<Property> disableProperties = {};
    auto ret = imc_->ListInputMethod(false, disableProperties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    bool hasNewIme = false;
    for (const auto &disableProperty : disableProperties) {
        if (disableProperty.name == newImeBundleName) {
            hasNewIme = true;
            break;
        }
    }
    EXPECT_TRUE(hasNewIme);
}

/**
* @tc.name: testIMCListInputMethodEnable
* @tc.desc: IMC ListInputMethod
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, testIMCListInputMethodEnable, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethodEnable Test Start");
    std::vector<Property> enableProperties = {};
    auto ret = imc_->ListInputMethod(true, enableProperties);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    EXPECT_EQ(enableProperties.size(), ENABLE_IME_NUM);
    EXPECT_EQ(enableProperties[ENABLE_IME_NUM - 1].name, bundleName);
    EXPECT_EQ(enableProperties[ENABLE_IME_NUM - 1].id, extName[0]);
}

/**
* @tc.name: tesIMCtListInputMethodSubtype_001
* @tc.desc: ListInputMethodSubtype
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, tesIMCtListInputMethodSubtype_001, TestSize.Level0)
{
    IMSA_HILOGI("IMC tesIMCtListInputMethodSubtype_001 Test Start");
    Property property = { .name = newImeBundleName };
    std::vector<SubProperty> subProps;
    auto ret = imc_->ListInputMethodSubtype(property, subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_EQ(subProps.size(), NEW_IME_SUBTYPE_NUM);
    for (int i = 0; i < NEW_IME_SUBTYPE_NUM; i++) {
        EXPECT_EQ(subProps[i].id, newImeSubName[i]);
        EXPECT_EQ(subProps[i].name, newImeBundleName);
    }
}

/**
* @tc.name: tesIMCtListInputMethodSubtype_002
* @tc.desc: ListInputMethodSubtype
* @tc.type: FUNC
* @tc.require: issuesI62BHB
* @tc.author: chenyu
*/
HWTEST_F(InputMethodSwitchTest, tesIMCtListInputMethodSubtype_002, TestSize.Level0)
{
    IMSA_HILOGI("IMC tesIMCtListInputMethodSubtype_002 Test Start");
    Property property = { .name = bundleName };
    std::vector<SubProperty> subProps;
    auto ret = imc_->ListInputMethodSubtype(property, subProps);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ASSERT_EQ(subProps.size(), IME_EXT_NUM);
    for (uint32_t i = 0; i < IME_EXT_NUM; i++) {
        EXPECT_EQ(subProps[i].id, extName[i]);
        EXPECT_EQ(subProps[i].name, bundleName);
    }
}

/**
 * @tc.name: testIMCListInputMethodSubtypeWithErrorBundleName
 * @tc.desc: IMC ListInputMethodSubtype
 * @tc.type: FUNC
 * @tc.require: issuesI62BHB
* @tc.author: chenyu
 */
HWTEST_F(InputMethodSwitchTest, testIMCListInputMethodSubtypeWithErrorBundleName, TestSize.Level0)
{
    IMSA_HILOGI("IMC testIMCListInputMethodSubtypeWitchErrorBundleName Test START");
    std::shared_ptr<Property> property = std::make_shared<Property>();
    std::vector<SubProperty> properties = {};
    auto ret = imc_->ListInputMethodSubtype(*property, properties);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    EXPECT_TRUE(properties.empty());
}

/**
* @tc.name: testShowOptionalInputMethod
* @tc.desc: IMC ShowOptionalInputMethod
* @tc.type: FUNC
*/
HWTEST_F(InputMethodSwitchTest, testShowOptionalInputMethod, TestSize.Level2)
{
    IMSA_HILOGI("IMC ShowOptionalInputMethod Test START");
    int32_t ret = imc_->ShowOptionalInputMethod();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testDisplayOptionalInputMethod
* @tc.desc: IMC DisplayOptionalInputMethod
* @tc.type: FUNC
*/
HWTEST_F(InputMethodSwitchTest, testDisplayOptionalInputMethod, TestSize.Level2)
{
    IMSA_HILOGI("IMC DisplayOptionalInputMethod Test START");
    sleep(2);
    int32_t ret = imc_->DisplayOptionalInputMethod();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}
} // namespace MiscServices
} // namespace OHOS