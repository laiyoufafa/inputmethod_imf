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

#ifndef SERVICES_INCLUDE_I_INPUT_METHOD_SYSTEM_ABILITY_H
#define SERVICES_INCLUDE_I_INPUT_METHOD_SYSTEM_ABILITY_H

#include <errors.h>

#include <memory>
#include <vector>

#include "global.h"
#include "i_input_client.h"
#include "i_input_data_channel.h"
#include "i_input_method_core.h"
#include "input_attribute.h"
#include "input_method_property.h"
#include "input_method_status.h"
#include "iremote_broker.h"
#include "keyboard_type.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
    class IInputMethodSystemAbility : public IRemoteBroker {
    public:
        enum {
            PREPARE_INPUT = 0,
            RELEASE_INPUT,
            START_INPUT,
            STOP_INPUT,
            HIDE_CURRENT_INPUT,
            SHOW_CURRENT_INPUT,
            SET_INPUT_METHOD_CORE,
            GET_DISPLAY_MODE,
            GET_KEYBOARD_WINDOW_HEIGHT,
            GET_CURRENT_INPUT_METHOD,
            GET_CURRENT_KEYBOARD_TYPE,
            LIST_INPUT_METHOD,
            LIST_KEYBOARD_TYPE,
            SET_CORE_AND_AGENT,
            DISPLAY_OPTIONAL_INPUT_METHOD,
            SWITCH_INPUT_METHOD,
            SHOW_CURRENT_INPUT_DEPRECATED,
            HIDE_CURRENT_INPUT_DEPRECATED,
            DISPLAY_OPTIONAL_INPUT_METHOD_DEPRECATED,
            SET_CORE_AND_AGENT_DEPRECATED,
        };

        DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputMethodSystemAbility");

        virtual void prepareInput(MessageParcel& data) = 0;
        virtual void releaseInput(MessageParcel& data) = 0;
        virtual void startInput(MessageParcel& data) = 0;
        virtual void stopInput(MessageParcel& data) = 0;
        virtual void SetCoreAndAgent(MessageParcel& data) = 0;
        virtual int32_t HideCurrentInput(MessageParcel& data) = 0;
        virtual int32_t ShowCurrentInput(MessageParcel& data) = 0;

        virtual int32_t displayOptionalInputMethod(MessageParcel& data) = 0;
        virtual int32_t getDisplayMode(int32_t &retMode) = 0;
        virtual int32_t getKeyboardWindowHeight(int32_t &retHeight) = 0;
        virtual std::shared_ptr<InputMethodProperty> GetCurrentInputMethod() = 0;
        virtual int32_t getCurrentKeyboardType(KeyboardType *retType) = 0;
        virtual std::vector<InputMethodProperty> ListInputMethod(InputMethodStatus stauts) = 0;
        virtual int32_t listKeyboardType(const std::u16string& imeId, std::vector<KeyboardType*> *types) = 0;
        virtual int32_t SwitchInputMethod(const InputMethodProperty &target) = 0;

        virtual void SetCoreAndAgentDeprecated(MessageParcel &data) = 0;
        virtual int32_t HideCurrentInputDeprecated(MessageParcel &data) = 0;
        virtual int32_t ShowCurrentInputDeprecated(MessageParcel &data) = 0;
        virtual int32_t DisplayOptionalInputMethodDeprecated(MessageParcel &data) = 0;
    };
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_I_INPUT_METHOD_SYSTEM_ABILITY_H
