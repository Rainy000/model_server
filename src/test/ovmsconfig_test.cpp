//*****************************************************************************
// Copyright 2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************
#include <fstream>
#include <filesystem>
#include <regex>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sysexits.h>

#include "../config.hpp"

using testing::_;
using testing::Return;
using testing::ReturnRef;
using testing::ContainerEq;


class OvmsConfigTest : public ::testing::Test {
public:
    void SetUp() override {
        sbuf = std::cout.rdbuf();
        std::cout.rdbuf(buffer.rdbuf());
    }
    void TearDown() override {
        std::cout.rdbuf(sbuf);
        sbuf = nullptr;
    }

    ::testing::AssertionResult AssertRegexMessageInOutput(std::string regexMessage) {
        // TODO: Currently cannot capture stdout should work once we switch to cmake
        // [ FATAL ] external/com_google_googletest/googletest/src/gtest-port.cc:1075:: Only one stdout capturer can exist at a time.
        // external/bazel_tools/tools/test/test-setup.sh: line 310: 11889 Aborted (core dumped) "${TEST_PATH}" "$@" 2>&1
        // when enabling with internal::CaptureStdout - and buffer is empty using Setup.
        std::string stdOut{buffer.str()};
        std::regex re(regexMessage.c_str());
        std::smatch m;
        bool found = std::regex_search(stdOut, m, re);

        return found ? ::testing::AssertionSuccess() : testing::AssertionFailure() << "message not found.";
    }

    std::stringstream buffer{};
    std::streambuf *sbuf;
};

TEST_F(OvmsConfigTest, bufferTest ) {
        std::string input{"Test buffer"};
        std::cout << input;
        std::string check{buffer.str()};
        EXPECT_EQ(input, check);
    }

TEST_F(OvmsConfigTest, emptyInput) {
    char* n_argv[] = { "ovms" };
    int arg_count = 1;
    EXPECT_EXIT(ovms::Config::instance().parse(arg_count, n_argv), ::testing::ExitedWithCode(EX_OK), "");

    // EXPECT_TRUE(AssertRegexMessageInOutput(std::string("config_path")));
}

TEST_F(OvmsConfigTest, helpInput) {
    char* n_argv[] = { "ovms", "help" };
    int arg_count = 2;
    EXPECT_EXIT(ovms::Config::instance().parse(arg_count, n_argv), ::testing::ExitedWithCode(EX_OK), "");

    // EXPECT_TRUE(AssertRegexMessageInOutput(std::string("config_path")));
}

TEST_F(OvmsConfigTest, badInput) {
    char* n_argv[] = { "ovms", "--bad_option" };
    int arg_count = 2;
    EXPECT_EXIT(ovms::Config::instance().parse(arg_count, n_argv), ::testing::ExitedWithCode(EX_USAGE), "error parsing options");
}

TEST_F(OvmsConfigTest, negativeTwoParams) {
    char* n_argv[] = { "ovms", "--config_path", "/path1", "--model_name", "some_name" };
    int arg_count = 5;
    EXPECT_EXIT(ovms::Config::instance().parse(arg_count, n_argv), ::testing::ExitedWithCode(EX_USAGE), "Use either config_path or model_path");
}


TEST_F(OvmsConfigTest, negativeMissingPaths) {
    char* n_argv[] = { "ovms", "--rest_port", "8080" };
    int arg_count = 5;
    EXPECT_EXIT(ovms::Config::instance().parse(arg_count, n_argv), ::testing::ExitedWithCode(EX_USAGE), "Use config_path or model_path");
}


TEST_F(OvmsConfigTest, negativeSamePorts) {
    char* n_argv[] = { "ovms", "--config_path", "/path1", "--rest_port", "8080", "--port", "8080" };
    int arg_count = 7;
    EXPECT_EXIT(ovms::Config::instance().parse(arg_count, n_argv), ::testing::ExitedWithCode(EX_USAGE), "port and rest_port cannot");
}


TEST_F(OvmsConfigTest, negativeMultiParams) {
    char* n_argv[] = { "ovms", "--config_path", "/path1", "--batch_size", "10" };
    int arg_count = 5;
    EXPECT_EXIT(ovms::Config::instance().parse(arg_count, n_argv), ::testing::ExitedWithCode(EX_USAGE), "Model parameters in CLI are exclusive");
}


TEST_F(OvmsConfigTest, missingParams) {
    char* n_argv[] = { "ovms", "--batch_size", "10" };
    int arg_count = 3;
    EXPECT_EXIT(ovms::Config::instance().parse(arg_count, n_argv), ::testing::ExitedWithCode(EX_USAGE), "Use config_path or model_path");
}
