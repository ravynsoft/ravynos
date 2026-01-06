/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __acdriver_CompileAction_h
#define __acdriver_CompileAction_h

namespace libutil { class Filesystem; }

namespace acdriver {

class Options;
class Output;
class Result;

/*
 * Compiles an asset catalog.
 */
class CompileAction {
public:
    CompileAction();
    ~CompileAction();

public:
    void run(libutil::Filesystem *filesystem, Options const &options, Output *output, Result *result);
};

}

#endif // !__acdriver_CompileAction_h
