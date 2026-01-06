/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __ninja_Writer_h
#define __ninja_Writer_h

#include <ninja/Value.h>

#include <string>
#include <sstream>
#include <vector>

namespace ninja {

/*
 * Writes a Ninja file. Attempts to be reasonably type-safe to avoid the
 * most common escaping and syntax errors, but remains quite low-level.
 */
class Writer {
private:
    std::ostringstream _buffer;

public:
    Writer();
    ~Writer();

public:
    /*
     * Write a new line.
     */
    void newline();

    /*
     * Write a top-level binding.
     */
    void binding(Binding const &binding, int indent = 0);

    /*
     * Write a generic command with an optional binding scope.
     */
    void command(std::string const &command, std::string const &remaining, std::vector<Binding> const &bindings = { });

    /*
     * Write out a comment.
     */
    void comment(std::string const &text);

public:
    /*
     * Write a subninja directive to load a scoped Ninja file.
     */
    void subninja(Value const &path);

    /*
     * Write an include directive to load an unscoped Ninja file.
     */
    void include(Value const &path);

    /*
     * Define the concatenative list of default Ninja targets.
     */
    void default_(std::vector<Value> const &paths);

public:
    /*
     * Write a rule to generalize the way files are built.
     */
    void rule(std::string const &name, Value const &command, std::vector<Binding> const &bindings = { });

    /*
     * Build a file with the specified rule, using the inputs and dependencies.
     */
    void build(std::vector<Value> const &outputs, std::string const &rule, std::vector<Value> const &inputs, std::vector<Binding> const &bindings = { }, std::vector<Value> const &dependencies = { }, std::vector<Value> const &orders = { });

    /*
     * Define a pool to limit the number of parallel invocations.
     */
    void pool(std::string const &name, int depth);

public:
    /*
     * Serialize what's been written so far.
     */
    std::string serialize() const;
};

}

#endif // !__ninja_Writer_h
