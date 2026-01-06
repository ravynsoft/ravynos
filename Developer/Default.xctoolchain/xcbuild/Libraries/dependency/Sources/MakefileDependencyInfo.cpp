/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <dependency/MakefileDependencyInfo.h>
#include <dependency/DependencyInfo.h>
#include <libutil/Escape.h>

#include <unordered_set>
#include <cassert>

using dependency::MakefileDependencyInfo;
using dependency::DependencyInfo;
using libutil::Escape;

MakefileDependencyInfo::
MakefileDependencyInfo()
{
}

std::string MakefileDependencyInfo::
serialize() const
{
    std::string result;

    for (DependencyInfo const &dependencyInfo : _dependencyInfo) {
        /* Add outputs. */
        for (std::string const &output : dependencyInfo.outputs()) {
            if (&output != &dependencyInfo.outputs().front()) {
                /* Outputs after the first need a separator. */
                result += " ";
            }

            /* Add the output. */
            result += Escape::Makefile(output);
        }

        /* Add separator. */
        result += ":";

        /* Add inputs. */
        for (std::string const &input : dependencyInfo.inputs()) {
            result += " \\\n";
            result += "  ";
            result += Escape::Makefile(input);
        }

        if (&dependencyInfo != &_dependencyInfo.back()) {
            /* Separator for next info. */
            result += "\n\n";
        }
    }

    return result;
}

ext::optional<MakefileDependencyInfo> MakefileDependencyInfo::
Deserialize(std::string const &contents)
{
    std::vector<DependencyInfo> dependencyInfo;

    enum class State {
        Begin,
        Comment,
        Output,
        Inputs,
    };

    State state = State::Begin;

    std::string current;
    DependencyInfo currentDependencyInfo;

    for (auto it = contents.begin(), prev = contents.end(); it != contents.end(); prev = it, ++it) {
        bool escaped = (prev != contents.end() && *prev == '\\');

        if (!escaped && *it == '#') {
            /* Begin comment. */
            state = State::Comment;
        } else if (!escaped && *it == '\n') {
            switch (state) {
                case State::Begin:
                    break;
                case State::Output:
                    /* Output without inputs. */
                    return ext::nullopt;
                case State::Comment:
                case State::Inputs:
                    /* Current input. */
                    if (!current.empty()) {
                        currentDependencyInfo.inputs().push_back(current);
                        current = std::string();
                    }

                    /* Store this output and inputs. */
                    if (!currentDependencyInfo.outputs().empty()) {
                        dependencyInfo.push_back(currentDependencyInfo);

                        /* Reset for next entry. */
                        currentDependencyInfo = DependencyInfo();
                    }

                    state = State::Begin;
                    break;
            }
        } else if ((!escaped && isspace(*it)) || (escaped && *it == '\n')) {
            switch (state) {
                case State::Begin:
                case State::Comment:
                case State::Output:
                    break;
                case State::Inputs:
                    /* Remove escape character. */
                    if (!current.empty() && escaped) {
                        current.resize(current.size() - 1);
                    }

                    /* Next input. */
                    if (!current.empty()) {
                        currentDependencyInfo.inputs().push_back(current);
                        current = std::string();
                    }
                    break;
            }
        } else if (!escaped && *it == ':') {
            switch (state) {
                case State::Begin:
                    /* Invalid character. */
                    return ext::nullopt;
                case State::Comment:
                    break;
                case State::Output:
                    assert(!current.empty());

                    /* Wait for inputs. */
                    currentDependencyInfo.outputs().push_back(current);
                    current = std::string();
                    state = State::Inputs;
                    break;
                case State::Inputs:
                    /* Invalid character. */
                    return ext::nullopt;
            }
        } else if (*it == '#' || *it == '%' || (escaped && isspace(*it))) {
            switch (state) {
                case State::Begin:
                    /* Invalid character. */
                    return ext::nullopt;
                case State::Comment:
                    break;
                case State::Output:
                case State::Inputs:
                    if (escaped) {
                        /* Unescape; replace backslash. */
                        current[current.size() - 1] = *it;
                        break;
                    } else {
                        /* Invalid character. */
                        return ext::nullopt;
                    }
            }
        } else {
            switch (state) {
                case State::Begin:
                    /* Start of output. */
                    state = State::Output;

                    /* Add character. */
                    current += *it;
                    break;
                case State::Comment:
                    break;
                case State::Output:
                case State::Inputs:
                    /* Add character. */
                    current += *it;
                    break;
            }
        }
    }

    switch (state) {
        case State::Begin:
            break;
        case State::Output:
            /* Output without inputs. */
            return ext::nullopt;
        case State::Comment:
        case State::Inputs:
            /* Current input. */
            if (!current.empty()) {
                currentDependencyInfo.inputs().push_back(current);
            }

            /* Store this output and inputs. */
            if (!currentDependencyInfo.outputs().empty()) {
                dependencyInfo.push_back(currentDependencyInfo);
            }
            break;
    }

    /* Create dependency info. */
    MakefileDependencyInfo makefileInfo;
    makefileInfo.dependencyInfo() = dependencyInfo;
    return makefileInfo;
}
