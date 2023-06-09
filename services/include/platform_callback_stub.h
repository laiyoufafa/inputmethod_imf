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

#ifndef SERVICES_INCLUDE_PLATFORM_CALLBACK_STUB_H
#define SERVICES_INCLUDE_PLATFORM_CALLBACK_STUB_H

#include <string>
#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"
#include "i_platform_callback.h"

namespace OHOS {
namespace MiscServices {
    class PlatformCallbackStub : public IRemoteStub<IPlatformCallback> {
    public:
        PlatformCallbackStub();
        ~PlatformCallbackStub();

        int  OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                     MessageOption &option) override;
        void notifyEvent(int eventId, int userId, const std::vector<std::u16string>& eventContent) override;
    };
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_PLATFORM_CALLBACK_STUB_H
