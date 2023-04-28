/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "input_method_panel.h"

#include <condition_variable>
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#include "display_manager.h"
#include "global.h"
#include "input_method_controller.h"
#include "panel_status_listener.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
constexpr uint32_t IMC_WAIT_PANEL_STATUS_LISTEN_TIME = 200;
class InputMethodPanelTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void CheckIMCPanelStatusChange(const InputWindowStatus &status, const InputWindowInfo &windowInfo);

    class PanelStatusListenerImpl : public PanelStatusListener {
    public:
        ~PanelStatusListenerImpl() = default;
        void OnPanelStatus(uint32_t windowId, bool isShow)
        {
            showPanel_ = isShow;
            hidePanel_ = !isShow;
            InputMethodPanelTest::panelListenerCv_.notify_one();
            IMSA_HILOGI("PanelStatusListenerImpl OnPanelStatus in, isShow is %{public}s", isShow ? "true" : "false");
        }
    };
    static std::mutex imcPanelStatusListenerLock_;
    static std::condition_variable imcPanelStatusListenerCv_;
    static InputWindowStatus status_;
    static std::vector<InputWindowInfo> windowInfo_;
    static bool showPanel_;
    static bool hidePanel_;
    static std::condition_variable panelListenerCv_;
    static std::mutex panelListenerLock_;
    static constexpr uint32_t DEALY_TIME = 1;
};
class InputMethodSettingListenerImpl : public InputMethodSettingListener {
public:
    InputMethodSettingListenerImpl() = default;
    ~InputMethodSettingListenerImpl() = default;
    void OnImeChange(const Property &property, const SubProperty &subProperty)
    {
    }
    void OnPanelStatusChange(const InputWindowStatus &status, const std::vector<InputWindowInfo> &windowInfo)
    {
        IMSA_HILOGI("InputMethodPanelTest::OnPanelStatusChange");
        {
            std::unique_lock<std::mutex> lock(InputMethodPanelTest::imcPanelStatusListenerLock_);
            InputMethodPanelTest::status_ = status;
            InputMethodPanelTest::windowInfo_ = windowInfo;
        }
        InputMethodPanelTest::imcPanelStatusListenerCv_.notify_one();
    }
};
bool InputMethodPanelTest::showPanel_ = false;
bool InputMethodPanelTest::hidePanel_ = false;
std::condition_variable InputMethodPanelTest::panelListenerCv_;
std::mutex InputMethodPanelTest::panelListenerLock_;
std::condition_variable InputMethodPanelTest::imcPanelStatusListenerCv_;
std::mutex InputMethodPanelTest::imcPanelStatusListenerLock_;
InputWindowStatus InputMethodPanelTest::status_{ InputWindowStatus::HIDE };
std::vector<InputWindowInfo> InputMethodPanelTest::windowInfo_;
void InputMethodPanelTest::SetUpTestCase(void)
{
    auto listener = std::make_shared<InputMethodSettingListenerImpl>();
    InputMethodController::GetInstance()->SetSettingListener(listener);
    IMSA_HILOGI("InputMethodPanelTest::SetUpTestCase");
}

void InputMethodPanelTest::TearDownTestCase(void)
{
    IMSA_HILOGI("InputMethodPanelTest::TearDownTestCase");
}

void InputMethodPanelTest::SetUp(void)
{
    IMSA_HILOGI("InputMethodPanelTest::SetUp");
}

void InputMethodPanelTest::TearDown(void)
{
    IMSA_HILOGI("InputMethodPanelTest::TearDown");
}

void InputMethodPanelTest::CheckIMCPanelStatusChange(const InputWindowStatus &status, const InputWindowInfo &windowInfo)
{
    std::unique_lock<std::mutex> lock(imcPanelStatusListenerLock_);
    imcPanelStatusListenerCv_.wait_for(lock, std::chrono::milliseconds(IMC_WAIT_PANEL_STATUS_LISTEN_TIME),
        [&status, &windowInfo] { return status == status_; });
    EXPECT_EQ(status_, status);
    ASSERT_EQ(windowInfo_.size(), 1);
    //EXPECT_EQ(windowInfo_[0].name, windowInfo.name);
    //EXPECT_EQ(windowInfo_[0].top, windowInfo.top);
    //EXPECT_EQ(windowInfo_[0].left, windowInfo.left);
    EXPECT_EQ(windowInfo_[0].width, windowInfo.width);
    EXPECT_EQ(windowInfo_[0].height, windowInfo.height);
}

/**
* @tc.name: testSetUiContent
* @tc.desc: Test SetUiContent.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testCreatePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testCreatePanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testResizePanel
* @tc.desc: Test Resize panel. All panels can be resized.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testResizePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testResizePanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    EXPECT_TRUE(defaultDisplay != nullptr);
    int32_t width = defaultDisplay->GetWidth();
    int32_t height = defaultDisplay->GetHeight();

    ret = inputMethodPanel->Resize(width - 1, height / 2 - 1);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->Resize(width, height / 2);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->Resize(width + 1, height / 2);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);

    ret = inputMethodPanel->Resize(width, height / 2 + 1);
    EXPECT_EQ(ret, ErrorCode::ERROR_BAD_PARAMETERS);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testMovePanel
* @tc.desc: Test Move panel. SOFT_KEYBOARD panel with FLG_FIXED can not be moved.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testMovePanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testMovePanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->MoveTo(10, 100);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->MoveTo(10, 100);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    panelInfo.panelType = STATUS_BAR;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->MoveTo(10, 100);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testShowPanel
* @tc.desc: Test Show panel.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testShowPanel, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testShowPanel start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    // 0、not create panel, show panel failed.
    auto ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::ERROR_NULL_POINTER);
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    EXPECT_TRUE(statusListener != nullptr);
    inputMethodPanel->SetPanelStatusListener(statusListener);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // 2、show floating type panel.
    ret = inputMethodPanel->ChangePanelFlag(PanelFlag::FLG_FLOATING);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    // 4、show status bar.
    panelInfo.panelType = STATUS_BAR;
    ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testSetPanelStatusListener
* @tc.desc: Test SetPanelStatusListener.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testSetPanelStatusListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetPanelStatusListener start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    EXPECT_TRUE(inputMethodPanel != nullptr);
    auto statusListener = std::make_shared<InputMethodPanelTest::PanelStatusListenerImpl>();
    EXPECT_TRUE(statusListener != nullptr);
    inputMethodPanel->SetPanelStatusListener(statusListener);

    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);

    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::showPanel_ == true; });
    }
    EXPECT_TRUE(InputMethodPanelTest::showPanel_);

    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::showPanel_ == false; });
    }
    EXPECT_TRUE(!InputMethodPanelTest::showPanel_);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testGetPanelType
* @tc.desc: Test GetPanelType.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testGetPanelType, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testSetUiContent start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto type = inputMethodPanel->GetPanelType();
    EXPECT_EQ(type, panelInfo.panelType);
    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testRemovePanelListener
* @tc.desc: Test RemovePanelListener.
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testRemovePanelListener, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testRemovePanelListener start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FLOATING };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto type = inputMethodPanel->GetPanelType();
    EXPECT_EQ(type, panelInfo.panelType);

    std::string subscribeType = "show";
    inputMethodPanel->RemovePanelListener(subscribeType);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::showPanel_ == false; });
    }
    EXPECT_EQ(InputMethodPanelTest::showPanel_, false);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::hidePanel_ == true; });
    }
    EXPECT_EQ(InputMethodPanelTest::hidePanel_, true);
    InputMethodPanelTest::hidePanel_ = false;

    subscribeType = "hide";
    inputMethodPanel->RemovePanelListener(subscribeType);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::showPanel_ == false; });
    }
    EXPECT_EQ(InputMethodPanelTest::showPanel_, false);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(InputMethodPanelTest::panelListenerLock_);
        InputMethodPanelTest::panelListenerCv_.wait_for(lock, std::chrono::seconds(InputMethodPanelTest::DEALY_TIME),
            [] { return InputMethodPanelTest::hidePanel_ == false; });
    }
    EXPECT_EQ(InputMethodPanelTest::hidePanel_, false);

    ret = inputMethodPanel->DestroyPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
}

/**
* @tc.name: testImcPanelListening_001
* @tc.desc: SOFT_KEYBOARD  FLG_FIXED
* @tc.type: FUNC
*/
HWTEST_F(InputMethodPanelTest, testImcPanelListening_001, TestSize.Level0)
{
    IMSA_HILOGI("InputMethodPanelTest::testImcPanelListening_001 start.");
    auto inputMethodPanel = std::make_shared<InputMethodPanel>();
    PanelInfo panelInfo = { .panelType = SOFT_KEYBOARD, .panelFlag = FLG_FIXED };
    auto ret = inputMethodPanel->CreatePanel(nullptr, panelInfo);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    EXPECT_TRUE(defaultDisplay != nullptr);
    uint32_t width = defaultDisplay->GetWidth() - 1;
    uint32_t height = defaultDisplay->GetHeight() / 2 - 1;
    ret = inputMethodPanel->Resize(width, height);
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    ret = inputMethodPanel->ShowPanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::CheckIMCPanelStatusChange(InputWindowStatus::SHOW, { "", 0, 0, width, height });
    width = defaultDisplay->GetWidth() - 3;
    height = defaultDisplay->GetHeight() / 2 - 3;
    ret = inputMethodPanel->Resize(width, height);
    ret = inputMethodPanel->HidePanel();
    EXPECT_EQ(ret, ErrorCode::NO_ERROR);
    InputMethodPanelTest::CheckIMCPanelStatusChange(InputWindowStatus::HIDE, { "", 0, 0, width, height });
}
} // namespace MiscServices
} // namespace OHOS