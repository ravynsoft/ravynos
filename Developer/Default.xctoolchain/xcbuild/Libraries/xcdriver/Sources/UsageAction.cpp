/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcdriver/UsageAction.h>
#include <xcdriver/Usage.h>
#include <libutil/FSUtil.h>
#include <process/Context.h>

#include <cstdio>

using xcdriver::UsageAction;
using xcdriver::Usage;
using libutil::FSUtil;

UsageAction::
UsageAction()
{
}

UsageAction::
~UsageAction()
{
}

int UsageAction::
Run(process::Context const *processContext)
{
    std::string path = processContext->executablePath();
    std::string text = Usage::Text(FSUtil::GetBaseName(path));
    fprintf(stdout, "%s", text.c_str());

    return 0;
}
