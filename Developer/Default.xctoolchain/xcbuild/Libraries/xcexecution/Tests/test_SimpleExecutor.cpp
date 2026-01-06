/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <xcexecution/SimpleExecutor.h>
#include <xcformatter/NullFormatter.h>
#include <pbxbuild/Tool/Invocation.h>
#include <builtin/Driver.h>
#include <builtin/Registry.h>
#include <process/MemoryContext.h>
#include <process/MemoryLauncher.h>
#include <libutil/MemoryFilesystem.h>

using xcexecution::SimpleExecutor;
using libutil::Filesystem;
using libutil::MemoryFilesystem;

class Driver : public builtin::Driver {
public:
    using Impl = std::function<int(process::Context const *processContext, Filesystem *filesystem)>;

private:
    std::string _name;
    Impl        _impl;

public:
    Driver(std::string const &name, Impl const &impl) :
        _name(name),
        _impl(impl)
    {
    }

public:
    virtual std::string name()
    { return _name; }

public:
    virtual int run(process::Context const *processContext, Filesystem *filesystem)
    { return _impl(processContext, filesystem); }
};

TEST(SimpleExecutor, PropagateToolResult)
{
    /* Create in-memory execution environment. */
    auto filesystem = MemoryFilesystem({
        MemoryFilesystem::Entry::File("fail-tool", std::vector<uint8_t>()),
        MemoryFilesystem::Entry::File("success-tool", std::vector<uint8_t>()),
    });

    auto launcher = process::MemoryLauncher({
        { filesystem.path("fail-tool"), [](Filesystem *filesystem, process::Context const *context) -> ext::optional<int> {
            return 1;
        } },
        { filesystem.path("success-tool"), [](Filesystem *filesystem, process::Context const *context) -> ext::optional<int> {
            return 0;
        } },
    });

    auto registry = builtin::Registry::Create({
        std::static_pointer_cast<builtin::Driver>(std::make_shared<Driver>("builtin-fail", [](process::Context const *context, Filesystem *filesystem) -> int {
            return 1;
        })),
        std::static_pointer_cast<builtin::Driver>(std::make_shared<Driver>("builtin-success", [](process::Context const *context, Filesystem *filesystem) -> int {
            return 0;
        })),
    });

    auto context = process::MemoryContext(
        "",
        filesystem.path(""),
        std::vector<std::string>(),
        std::unordered_map<std::string, std::string>());

    /* Create invocations to execute. */
    auto builtinSuccess = pbxbuild::Tool::Invocation();
    builtinSuccess.executable() = pbxbuild::Tool::Invocation::Executable::Builtin("builtin-success");
    auto builtinFail = pbxbuild::Tool::Invocation();
    builtinFail.executable() = pbxbuild::Tool::Invocation::Executable::Builtin("builtin-fail");

    auto externalSuccess = pbxbuild::Tool::Invocation();
    externalSuccess.executable() = pbxbuild::Tool::Invocation::Executable::External("success-tool");
    auto externalFail = pbxbuild::Tool::Invocation();
    externalFail.executable() = pbxbuild::Tool::Invocation::Executable::External("fail-tool");

    /* Create test executor. */
    auto formatter = xcformatter::NullFormatter::Create();
    std::vector<std::string> const executablePaths = { filesystem.path("") };
    SimpleExecutor executor = SimpleExecutor(formatter, false, registry);

    /* Succeed if all tools succeed. */
    auto success = executor.performInvocations(
        &context,
        &launcher,
        &filesystem,
        executablePaths,
        {
            builtinSuccess,
            externalSuccess,
        },
        false);
    ASSERT_TRUE(success.first);
    EXPECT_EQ(success.second.size(), 0);

    /* Fail if a tool fails. */
    auto fail1 = executor.performInvocations(
        &context,
        &launcher,
        &filesystem,
        executablePaths,
        {
            externalFail,
            builtinSuccess,
            externalSuccess,
        },
        false);
    ASSERT_FALSE(fail1.first);
    EXPECT_EQ(fail1.second.size(), 1);

    /* Fail if a tool fails, even if not first. */
    auto fail2 = executor.performInvocations(
        &context,
        &launcher,
        &filesystem,
        executablePaths,
        {
            builtinSuccess,
            builtinFail,
            externalSuccess,
            externalFail,
        },
        false);
    ASSERT_FALSE(fail2.first);
    EXPECT_EQ(fail2.second.size(), 1);
}

