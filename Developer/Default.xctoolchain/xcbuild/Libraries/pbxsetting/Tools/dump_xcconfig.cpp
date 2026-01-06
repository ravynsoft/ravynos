/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxsetting/XC/Config.h>
#include <pbxsetting/Environment.h>
#include <pbxsetting/Setting.h>
#include <pbxsetting/Value.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/FSUtil.h>

#include <cstdio>

using libutil::Filesystem;
using libutil::DefaultFilesystem;
using libutil::FSUtil;

static void
DumpConfig(pbxsetting::Environment const &environment, pbxsetting::XC::Config const &config)
{
    fprintf(stdout, "%s:\n", config.path().c_str());
    for (pbxsetting::XC::Config::Entry const &entry : config.contents()) {
        switch (entry.type()) {
            case pbxsetting::XC::Config::Entry::Type::Setting: {
                pbxsetting::Setting const &setting = *entry.setting();
                fprintf(stdout, "  %s = %s\n", setting.name().c_str(), setting.value().raw().c_str());
                break;
            }
            case pbxsetting::XC::Config::Entry::Type::Include: {
                pbxsetting::Value const &path = *entry.path();
                fprintf(stdout, "  #include \"%s\"\n", path.raw().c_str());
                break;
            }
        }
    }
    fprintf(stdout, "\n");

    for (pbxsetting::XC::Config::Entry const &entry : config.contents()) {
        if (entry.type() == pbxsetting::XC::Config::Entry::Type::Include) {
            DumpConfig(environment, *entry.config());
        }
    }
}

int
main(int argc, char **argv)
{
    DefaultFilesystem filesystem = DefaultFilesystem();

    if (argc < 2) {
        fprintf(stderr, "usage: %s filename.xcconfig\n", argv[0]);
        return -1;
    }

    pbxsetting::Environment environment = pbxsetting::Environment();
    auto config = pbxsetting::XC::Config::Load(&filesystem, environment, argv[1]);
    if (!config) {
        fprintf(stderr, "The file \"%s\" couldn’t be opened. It may be missing.\n", argv[1]);
        return -1;
    }

    DumpConfig(environment, *config);

    fprintf(stdout, "Resolved settings:\n");
    std::vector<pbxsetting::Setting> settings = config->level().settings();
    for (pbxsetting::Setting const &setting : settings) {
        fprintf(stdout, "  %s = %s\n", setting.name().c_str(), setting.value().raw().c_str());
    }

    return 0;
}
