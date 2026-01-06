/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __dependency_DependencyInfo_h
#define __dependency_DependencyInfo_h

#include <string>
#include <vector>

namespace dependency {

/*
 * Generic dependency info holder. Note this intentionally does not associate
 * outputs with any particular input; many dependency info formats don't support
 * that and it's unnecessary for build dependencies.
 */
class DependencyInfo {
private:
    std::vector<std::string> _inputs;
    std::vector<std::string> _outputs;

public:
    DependencyInfo(std::vector<std::string> const &inputs, std::vector<std::string> const &outputs);
    DependencyInfo();

public:
    /*
     * The inputs to the tool.
     */
    std::vector<std::string> const &inputs() const
    { return _inputs; }
    std::vector<std::string> &inputs()
    { return _inputs; }

    /*
     * The files output by the tool.
     */
    std::vector<std::string> const &outputs() const
    { return _outputs; }
    std::vector<std::string> &outputs()
    { return _outputs; }
};

}

#endif /* __dependency_DependencyInfo_h */
