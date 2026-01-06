/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxproj/PBX/ShellScriptBuildPhase.h>
#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Keys/Unpack.h>

using pbxproj::PBX::ShellScriptBuildPhase;
using pbxproj::Context;

ShellScriptBuildPhase::
ShellScriptBuildPhase() :
    BuildPhase       (Isa(), Type::ShellScript),
    _showEnvVarsInLog(true)
{
}

bool ShellScriptBuildPhase::
parse(Context &context, plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check)
{
    if (!BuildPhase::parse(context, dict, seen, false)) {
        return false;
    }

    auto unpack = plist::Keys::Unpack("ShellScriptBuildPhase", dict, seen);

    auto N  = unpack.cast <plist::String> ("name");
    auto SP = unpack.cast <plist::String> ("shellPath");
    auto SS = unpack.cast <plist::String> ("shellScript");
    auto IP = unpack.cast <plist::Array> ("inputPaths");
    auto OP = unpack.cast <plist::Array> ("outputPaths");
    auto SE = unpack.coerce <plist::Boolean> ("showEnvVarsInLog");

    if (!unpack.complete(check)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (N != nullptr) {
        _name = N->value();
    }

    if (SP != nullptr) {
        _shellPath = SP->value();
    }

    if (SS != nullptr) {
        _shellScript = SS->value();
    }

    if (IP != nullptr) {
        for (size_t n = 0; n < IP->count(); n++) {
            auto P = IP->value <plist::String> (n);
            if (P != nullptr) {
                pbxsetting::Value V = pbxsetting::Value::Parse(P->value());
                _inputPaths.push_back(V);
            }
        }
    }

    if (OP != nullptr) {
        for (size_t n = 0; n < OP->count(); n++) {
            auto P = OP->value <plist::String> (n);
            if (P != nullptr) {
                pbxsetting::Value V = pbxsetting::Value::Parse(P->value());
                _outputPaths.push_back(V);
            }
        }
    }

    if (SE != nullptr) {
        _showEnvVarsInLog = SE->value();
    }

    return true;
}
