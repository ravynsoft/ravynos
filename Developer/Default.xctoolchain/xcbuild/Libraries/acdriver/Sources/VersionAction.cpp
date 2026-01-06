/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/VersionAction.h>
#include <acdriver/Version.h>
#include <acdriver/Options.h>
#include <acdriver/Output.h>
#include <acdriver/Result.h>
#include <plist/Dictionary.h>
#include <plist/String.h>

using acdriver::VersionAction;
using acdriver::Version;
using acdriver::Options;
using acdriver::Output;
using acdriver::Result;

VersionAction::
VersionAction()
{
}

VersionAction::
~VersionAction()
{
}

void VersionAction::
run(Options const &options, Output *output, Result *result)
{
    std::unique_ptr<plist::Dictionary> dict = plist::Dictionary::New();

    std::string bundleVersion = std::to_string(Version::BuildVersion());
    std::string shortBundleVersion = Version::UserVersion();

    dict->set("bundle-version", plist::String::New(bundleVersion));
    dict->set("short-bundle-version", plist::String::New(shortBundleVersion));

    std::string text;
    text += "bundle-version: " + bundleVersion + "\n";
    text += "short-bundle-version: " + shortBundleVersion + "\n";

    output->add("com.apple.actool.version", std::move(dict), text);
}

