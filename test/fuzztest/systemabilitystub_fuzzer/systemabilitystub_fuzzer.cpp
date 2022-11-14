/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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


#include "systemabilitystub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "input_method_system_ability.h"
#include "global.h"

#include "message_parcel.h"

using namespace OHOS::MiscServices;
namespace OHOS {
    constexpr size_t THRESHOLD = 10;
    constexpr int32_t OFFSET = 4;
    const std::u16string SYSTEMABILITY_INTERFACE_TOKEN = u"ohos.miscservices.inputmethod.IInputMethodSystemAbility";

    uint32_t ConvertToUint32(const uint8_t *ptr)
    {
        if (ptr == nullptr) {
            return 0;
        }
        uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
        return bigVar;
    }
    bool FuzzInputMethodSystemAbility(const uint8_t* rawData, size_t size)
    {
        uint32_t code = ConvertToUint32(rawData);
        rawData = rawData + OFFSET;
        size = size - OFFSET;

        MessageParcel data;
        data.WriteInterfaceToken(SYSTEMABILITY_INTERFACE_TOKEN);
        data.WriteBuffer(rawData, size);
        data.RewindRead(0);
        MessageParcel reply;
        MessageOption option;

        sptr<InputMethodSystemAbility> ability = new InputMethodSystemAbility();
        ability->OnRemoteRequest(code, data, reply, option);

        return true;
    }
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzInputMethodSystemAbility(data, size);
    return 0;
}