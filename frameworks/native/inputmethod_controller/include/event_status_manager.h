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

#ifndef FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_LISTEN_EVENT_MANAGER_H
#define FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_LISTEN_EVENT_MANAGER_H

namespace OHOS {
namespace MiscServices {
enum EventType : uint32_t {
    IME_CHANGE = 0,
    IME_SHOW = 1,
    IME_HIDE = 2,
    IME_NONE
};
class EventStatusManager {
public:
    static constexpr uint32_t NO_EVENT_ON = 0;

    inline static bool IsImeShowOn(uint32_t eventFlag)
    {
        return (eventFlag & (1u << IME_SHOW)) == (1u << IME_SHOW);
    }
    inline static bool IsImeChangeOn(uint32_t eventFlag)
    {
        return (eventFlag & (1u << IME_CHANGE)) == (1u << IME_CHANGE);
    }
    inline static bool IsImeHideOn(uint32_t eventFlag)
    {
        return (eventFlag & (1u << IME_HIDE)) == (1u << IME_HIDE);
    }
};
} // namespace MiscServices
} // namespace OHOS

#endif //FRAMEWORKS_INPUTMETHOD_CONTROLLER_INCLUDE_LISTEN_EVENT_MANAGER_H