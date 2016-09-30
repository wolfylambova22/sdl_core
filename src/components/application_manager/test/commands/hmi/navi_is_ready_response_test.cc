/*
 * Copyright (c) 2016, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <string>

#include "gtest/gtest.h"
#include "application_manager/commands/commands_test.h"
#include "application_manager/commands/command_request_test.h"
#include "application_manager/mock_hmi_capabilities.h"
#include "application_manager/mock_application_manager.h"
#include "application_manager/mock_event_dispatcher.h"
#include "application_manager/commands/hmi/navi_is_ready_response.h"

namespace test {
namespace components {
namespace commands_test {
namespace hmi_commands_test {
namespace navi_is_ready_responce {

namespace am = ::application_manager;
namespace commands = am::commands;

using ::testing::ReturnRef;
using ::testing::Eq;
using ::utils::SharedPtr;
using commands::MessageSharedPtr;
using commands::ResponseFromHMI;

typedef SharedPtr<ResponseFromHMI> ResponseFromHMIPtr;

namespace {
const bool kNaviIsAvailable = true;
const bool kNaviIsNotAvailable = false;
}  // namespace

class NaviIsReadyResponseTest
    : public CommandRequestTest<CommandsTestMocks::kIsNice> {
 public:
  void TestRunWithSpecificNaviState(const bool is_available) {
    application_manager_test::MockHMICapabilities mock_hmi_capabilities;
    MessageSharedPtr command_msg(CreateMessage(smart_objects::SmartType_Map));
    (*command_msg)[am::strings::msg_params][am::strings::available] =
        is_available;

    ResponseFromHMIPtr command(
        CreateCommand<commands::NaviIsReadyResponse>(command_msg));

    EXPECT_CALL(mock_app_manager_, hmi_capabilities())
        .WillOnce(ReturnRef(mock_hmi_capabilities));
    EXPECT_CALL(mock_hmi_capabilities, set_is_navi_cooperating(is_available));

    command->Run();
  }
};

TEST_F(NaviIsReadyResponseTest, NaviIsReadyResponse_Run_SUCCESS) {
  TestRunWithSpecificNaviState(kNaviIsAvailable);
}

TEST_F(NaviIsReadyResponseTest, NaviIsReadyResponse_Run_UNSUCCESS) {
  TestRunWithSpecificNaviState(kNaviIsNotAvailable);
}

}  // namespace navi_is_ready_responce
}  // namespace hmi_commands_test
}  // namespace commands_test
}  // namespace components
}  // namespace test
