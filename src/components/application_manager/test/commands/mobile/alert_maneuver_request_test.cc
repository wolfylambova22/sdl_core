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
#include "utils/shared_ptr.h"
#include "smart_objects/smart_object.h"
#include "application_manager/smart_object_keys.h"
#include "commands/commands_test.h"
#include "commands/command_request_test.h"
#include "application_manager/application.h"
#include "application_manager/mock_application_manager.h"
#include "application_manager/mock_application.h"
#include "application_manager/mock_message_helper.h"
#include "application_manager/event_engine/event.h"
#include "mobile/alert_maneuver_request.h"
#include "interfaces/MOBILE_API.h"
#include "application_manager/policies/policy_handler_interface.h"

namespace test {
namespace components {
namespace commands_test {
namespace mobile_commands_test {
namespace alert_maneuver_request {

namespace strings = am::strings;
namespace hmi_response = am::hmi_response;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
namespace am = ::application_manager;
using application_manager::MockMessageHelper;
using am::commands::AlertManeuverRequest;
using am::commands::MessageSharedPtr;
using am::event_engine::Event;

namespace {
const uint32_t kAppId = 1u;
const uint32_t kConnectionKey = 2u;
}  // namespace

typedef SharedPtr<AlertManeuverRequest> CommandPtr;

enum CommandType { NAVI, TTS };

class AlertManeuverRequestTest
    : public CommandRequestTest<CommandsTestMocks::kIsNice> {
 public:
  AlertManeuverRequestTest()
      : mock_message_helper_(*MockMessageHelper::message_helper_mock()) {}

  void OnEventTestHelper(mobile_apis::Result::eType result_code,
                         CommandType command_type,
                         hmi_apis::Common_Result::eType tts_result,
                         hmi_apis::Common_Result::eType navi_result,
                         mobile_apis::Result::eType mobile_result_code,
                         hmi_apis::FunctionID::eType function_id) {
    MessageSharedPtr msg = CreateMessage();
    (*msg)[strings::params][hmi_response::code] = result_code;
    (*msg)[strings::msg_params] = SmartObject(smart_objects::SmartType_Map);
    CommandPtr command(CreateCommand<AlertManeuverRequest>(msg));

    if (command_type == CommandType::NAVI) {
      EXPECT_CALL(mock_message_helper_, MobileToHMIResult(result_code))
          .WillOnce(Return(navi_result));
    } else if (command_type == CommandType::TTS) {
      EXPECT_CALL(mock_message_helper_, MobileToHMIResult(result_code))
          .WillOnce(Return(tts_result));
    } else {
      EXPECT_CALL(
          mock_app_manager_,
          ManageMobileCommand(MobileResultCodeIs(mobile_result_code), _));
    }

    Event event(function_id);
    event.set_smart_object(*msg);

    command->on_event(event);
  }

  MockMessageHelper& mock_message_helper_;
};

TEST_F(AlertManeuverRequestTest, Run_RequiredFieldsDoesNotExist_UNSUCCESS) {
  CommandPtr command(CreateCommand<AlertManeuverRequest>());
  EXPECT_CALL(mock_app_manager_, application(_)).Times(0);
  MessageSharedPtr result_msg(CatchMobileCommandResult(CallRun(*command)));
  EXPECT_EQ(mobile_apis::Result::INVALID_DATA,
            static_cast<mobile_apis::Result::eType>(
                (*result_msg)[am::strings::msg_params][am::strings::result_code]
                    .asInt()));
}

TEST_F(AlertManeuverRequestTest, Run_ApplicationIsNotRegistered_UNSUCCESS) {
  MessageSharedPtr msg = CreateMessage(smart_objects::SmartType_Map);
  (*msg)[am::strings::msg_params][am::strings::soft_buttons] = 0;
  (*msg)[am::strings::msg_params][am::strings::tts_chunks] = 0;

  CommandPtr command(CreateCommand<AlertManeuverRequest>(msg));

  ON_CALL(mock_app_manager_, application(_))
      .WillByDefault(Return(ApplicationSharedPtr()));

  MessageSharedPtr result_msg(CatchMobileCommandResult(CallRun(*command)));
  EXPECT_EQ(mobile_apis::Result::APPLICATION_NOT_REGISTERED,
            static_cast<mobile_apis::Result::eType>(
                (*result_msg)[am::strings::msg_params][am::strings::result_code]
                    .asInt()));
}

TEST_F(AlertManeuverRequestTest, Run_ProcessingResult_UNSUCCESS) {
  MessageSharedPtr msg = CreateMessage(smart_objects::SmartType_Map);
  (*msg)[am::strings::msg_params][am::strings::soft_buttons] = 0;

  CommandPtr command(CreateCommand<AlertManeuverRequest>(msg));

  MockAppPtr app(CreateMockApp());
  ON_CALL(mock_app_manager_, application(_)).WillByDefault(Return(app));

  ON_CALL(mock_app_manager_, GetPolicyHandler())
      .WillByDefault(
          ReturnRef(*static_cast<policy::PolicyHandlerInterface*>(NULL)));

  const mobile_apis::Result::eType kProcessingResult =
      mobile_apis::Result::ABORTED;

  EXPECT_CALL(*(am::MockMessageHelper::message_helper_mock()),
              ProcessSoftButtons(_, _, _, _))
      .WillOnce(Return(kProcessingResult));

  MessageSharedPtr result_msg(CatchMobileCommandResult(CallRun(*command)));
  EXPECT_EQ(kProcessingResult,
            static_cast<mobile_apis::Result::eType>(
                (*result_msg)[am::strings::msg_params][am::strings::result_code]
                    .asInt()));
}

TEST_F(AlertManeuverRequestTest, Run_IsWhiteSpaceExist_UNSUCCESS) {
  MessageSharedPtr msg = CreateMessage(smart_objects::SmartType_Map);
  (*msg)[am::strings::msg_params][am::strings::soft_buttons] = 0;
  (*msg)[am::strings::msg_params][am::strings::tts_chunks] =
      SmartObject(smart_objects::SmartType_Array);

  SmartObject tts_chunk(smart_objects::SmartType_Map);
  tts_chunk[am::strings::text] = "wrong chunk syntax\t\n";

  (*msg)[am::strings::msg_params][am::strings::tts_chunks].asArray()->push_back(
      tts_chunk);

  CommandPtr command(CreateCommand<AlertManeuverRequest>(msg));

  MockAppPtr app(CreateMockApp());
  ON_CALL(mock_app_manager_, application(_)).WillByDefault(Return(app));

  MessageSharedPtr result_msg(CatchMobileCommandResult(CallRun(*command)));
  EXPECT_EQ(mobile_apis::Result::INVALID_DATA,
            static_cast<mobile_apis::Result::eType>(
                (*result_msg)[am::strings::msg_params][am::strings::result_code]
                    .asInt()));
}

TEST_F(AlertManeuverRequestTest, Run_ProcessingResult_SUCCESS) {
  MessageSharedPtr msg = CreateMessage(smart_objects::SmartType_Map);
  (*msg)[am::strings::msg_params][am::strings::soft_buttons] = 0;

  CommandPtr command(CreateCommand<AlertManeuverRequest>(msg));

  MockAppPtr app(CreateMockApp());
  ON_CALL(mock_app_manager_, application(_)).WillByDefault(Return(app));

  ON_CALL(mock_app_manager_, GetPolicyHandler())
      .WillByDefault(
          ReturnRef(*static_cast<policy::PolicyHandlerInterface*>(NULL)));

  EXPECT_CALL(*(am::MockMessageHelper::message_helper_mock()),
              ProcessSoftButtons(_, _, _, _))
      .WillOnce(Return(mobile_apis::Result::SUCCESS));

  EXPECT_CALL(*(am::MockMessageHelper::message_helper_mock()),
              SubscribeApplicationToSoftButton(_, _, _));

  MessageSharedPtr result_msg(CatchHMICommandResult(CallRun(*command)));
  EXPECT_EQ(hmi_apis::FunctionID::Navigation_AlertManeuver,
            static_cast<hmi_apis::FunctionID::eType>(
                (*result_msg)[am::strings::params][am::strings::function_id]
                    .asInt()));
}

TEST_F(AlertManeuverRequestTest, Run_TTSIsOk_SUCCESS) {
  MessageSharedPtr msg = CreateMessage(smart_objects::SmartType_Map);
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
  (*msg)[am::strings::msg_params][am::strings::soft_buttons] = 0;
  (*msg)[am::strings::msg_params][am::strings::tts_chunks] = 0;
  (*msg)[am::strings::msg_params][am::strings::tts_chunks][0]
        [am::strings::text] = "text";

  CommandPtr command(CreateCommand<AlertManeuverRequest>(msg));

  MockAppPtr mock_app(CreateMockApp());
  ON_CALL(mock_app_manager_, application(kConnectionKey))
      .WillByDefault(Return(mock_app));

  ON_CALL(mock_app_manager_, GetPolicyHandler())
      .WillByDefault(
          ReturnRef(*static_cast<policy::PolicyHandlerInterface*>(NULL)));

  EXPECT_CALL(*(am::MockMessageHelper::message_helper_mock()),
              ProcessSoftButtons(_, _, _, _))
      .WillOnce(Return(mobile_apis::Result::SUCCESS));

  EXPECT_CALL(*(am::MockMessageHelper::message_helper_mock()),
              SubscribeApplicationToSoftButton(_, _, _));

  command->Run();
}

TEST_F(AlertManeuverRequestTest, OnEvent_ReceivedUnknownEvent_UNSUCCESS) {
  CommandPtr command(CreateCommand<AlertManeuverRequest>());
  Event event(hmi_apis::FunctionID::INVALID_ENUM);

  MessageSharedPtr result_msg(
      CatchMobileCommandResult(CallOnEvent(*command, event)));
  EXPECT_EQ(mobile_apis::Result::INVALID_ENUM,
            static_cast<mobile_apis::Result::eType>(
                (*result_msg)[am::strings::msg_params][am::strings::result_code]
                    .asInt()));
}

TEST_F(AlertManeuverRequestTest, OnEvent_AlertManeuverTTSInvalid_SUCCESS) {
  OnEventTestHelper(mobile_apis::Result::SUCCESS,
                    CommandType::NAVI,
                    hmi_apis::Common_Result::INVALID_ENUM,
                    hmi_apis::Common_Result::SUCCESS,
                    mobile_apis::Result::SUCCESS,
                    hmi_apis::FunctionID::Navigation_AlertManeuver);
}

TEST_F(AlertManeuverRequestTest, OnEvent_AlertManeuverTTSInvalid_UNSUCCESS) {
  OnEventTestHelper(mobile_apis::Result::SUCCESS,
                    CommandType::NAVI,
                    hmi_apis::Common_Result::INVALID_ENUM,
                    hmi_apis::Common_Result::WRONG_LANGUAGE,
                    mobile_apis::Result::SUCCESS,
                    hmi_apis::FunctionID::Navigation_AlertManeuver);
}

TEST_F(AlertManeuverRequestTest,
       OnEvent_AlertManeuverTTSUnsupResource_SUCCESS) {
  OnEventTestHelper(mobile_apis::Result::SUCCESS,
                    CommandType::NAVI,
                    hmi_apis::Common_Result::UNSUPPORTED_RESOURCE,
                    hmi_apis::Common_Result::SUCCESS,
                    mobile_apis::Result::WARNINGS,
                    hmi_apis::FunctionID::Navigation_AlertManeuver);
}

TEST_F(AlertManeuverRequestTest, OnEvent_AlertManeuverTTSWarnings_SUCCESS) {
  OnEventTestHelper(mobile_apis::Result::SUCCESS,
                    CommandType::NAVI,
                    hmi_apis::Common_Result::WARNINGS,
                    hmi_apis::Common_Result::SUCCESS,
                    mobile_apis::Result::WARNINGS,
                    hmi_apis::FunctionID::Navigation_AlertManeuver);
}

TEST_F(AlertManeuverRequestTest, OnEvent_TTSSpeakAlertInvalidEnum_SUCCESS) {
  OnEventTestHelper(mobile_apis::Result::SUCCESS,
                    CommandType::TTS,
                    hmi_apis::Common_Result::SUCCESS,
                    hmi_apis::Common_Result::INVALID_ENUM,
                    mobile_apis::Result::SUCCESS,
                    hmi_apis::FunctionID::TTS_Speak);
}

TEST_F(AlertManeuverRequestTest, OnEvent_TTSSpeakAlertWarnings_SUCCESS) {
  OnEventTestHelper(mobile_apis::Result::SUCCESS,
                    CommandType::TTS,
                    hmi_apis::Common_Result::SUCCESS,
                    hmi_apis::Common_Result::WARNINGS,
                    mobile_apis::Result::WARNINGS,
                    hmi_apis::FunctionID::TTS_Speak);
}

TEST_F(AlertManeuverRequestTest, OnEvent_TTSSpeakAlertWarnings_UNSUCCESS) {
  OnEventTestHelper(mobile_apis::Result::SUCCESS,
                    CommandType::TTS,
                    hmi_apis::Common_Result::UNSUPPORTED_RESOURCE,
                    hmi_apis::Common_Result::UNSUPPORTED_RESOURCE,
                    mobile_apis::Result::WARNINGS,
                    hmi_apis::FunctionID::TTS_Speak);
}

TEST_F(AlertManeuverRequestTest, OnEvent_TfTSSpeakAlertWarnings_Success) {
  MessageSharedPtr msg = CreateMessage();
  const uint32_t connection_key = 1u;
  const uint32_t correlation_id = 2u;
  (*msg)[strings::params][strings::connection_key] = connection_key;
  (*msg)[strings::params][strings::correlation_id] = correlation_id;

  CommandPtr command(CreateCommand<AlertManeuverRequest>(msg));

  Event event(hmi_apis::FunctionID::TTS_OnResetTimeout);
  event.set_smart_object(*msg);

  EXPECT_CALL(mock_app_manager_,
              updateRequestTimeout(connection_key, correlation_id, _));

  command->on_event(event);
}

}  // namespace alert_maneuver_request
}  // namespace mobile_commands_test
}  // namespace commands_test
}  // namespace components
}  // namespace test
