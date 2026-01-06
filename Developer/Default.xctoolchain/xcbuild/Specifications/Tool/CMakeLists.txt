#
# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.
#

install(FILES
        com.apple.build-tools.nmedit.xcspec
        com.apple.build-tools.strip.xcspec
        com.apple.build-tools.swift-stdlib-tool.xcspec
        com.apple.commands.built-in.compilation-database-generator.xcspec
        com.apple.commands.built-in.headermap-generator.xcspec
        com.apple.commands.built-in.validate.xcspec
        com.apple.commands.shell-script.xcspec
        com.apple.compilers.cpp.xcspec
        com.apple.compilers.lex.xcspec
        com.apple.compilers.pbxcp.xcspec
        com.apple.compilers.resource-copier.xcspec
        com.apple.compilers.yacc.xcspec
        com.apple.tools.ditto.xcspec
        com.apple.tools.dsymutil.xcspec
        com.apple.tools.info-plist-utility.xcspec
        com.apple.tools.mkdir.xcspec
        com.apple.tools.plutil.xcspec
        com.apple.tools.symlink.xcspec
        com.apple.tools.touch.xcspec
        DESTINATION Library/Xcode/Specifications)
