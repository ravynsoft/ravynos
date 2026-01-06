/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <ninja/Writer.h>

using ninja::Writer;
using ninja::Binding;
using ninja::Value;

Writer::
Writer()
{
}

Writer::
~Writer()
{
}

void Writer::
newline()
{
    _buffer << std::endl;
}

void Writer::
binding(Binding const &binding, int indent)
{
    for (int i = 0; i < indent; i++) {
        _buffer << "  ";
    }

    _buffer << binding.first << " = " << binding.second.resolve(Value::EscapeMode::Value) << std::endl;
}

void Writer::
command(std::string const &command, std::string const &remaining, std::vector<Binding> const &bindings)
{
    _buffer << command;
    if (!remaining.empty()) {
        _buffer << " " << remaining;
    }
    _buffer << std::endl;

    for (Binding const &binding : bindings) {
        this->binding(binding, 1);
    }

    _buffer << std::endl;
}

void Writer::
comment(std::string const &text)
{
    _buffer << "#" << " " << text << std::endl;
}

void Writer::
subninja(Value const &path)
{
    command("subninja", path.resolve(Value::EscapeMode::PathList));
}

void Writer::
include(Value const &path)
{
    command("include", path.resolve(Value::EscapeMode::PathList));
}

void Writer::
default_(std::vector<Value> const &paths)
{
    std::string remaining;
    for (Value const &path : paths) {
        if (&path != &paths[0]) {
            remaining += " ";
        }
        remaining += path.resolve(Value::EscapeMode::PathList);
    }

    command("default", remaining);
}

void Writer::
pool(std::string const &name, int depth)
{
    command("pool", name, {
        { "depth", Value::String(std::to_string(depth)) },
    });
}

void Writer::
rule(std::string const &name, Value const &command, std::vector<Binding> const &bindings)
{
    std::vector<Binding> bindingList = bindings;
    bindingList.insert(bindingList.begin(), { "command", command });

    this->command("rule", name, bindingList);
}

void Writer::
build(std::vector<Value> const &outputs, std::string const &rule, std::vector<Value> const &inputs, std::vector<Binding> const &bindings, std::vector<Value> const &dependencies, std::vector<Value> const &orders)
{
    std::ostringstream remaining;

    for (Value const &output : outputs) {
        if (&output != &outputs[0]) {
            remaining << " ";
        }
        remaining << output.resolve(Value::EscapeMode::BuildPathList);
    }

    remaining << ":" << " " << rule;

    for (Value const &input : inputs) {
        remaining << " " << input.resolve(Value::EscapeMode::BuildPathList);
    }

    if (!dependencies.empty()) {
        remaining << " " << "|";
        for (Value const &dependency : dependencies) {
            remaining << " " << dependency.resolve(Value::EscapeMode::BuildPathList);
        }
    }

    if (!orders.empty()) {
        remaining << " " << "||";
        for (Value const &order : orders) {
            remaining << " " << order.resolve(Value::EscapeMode::BuildPathList);
        }
    }

    command("build", remaining.str(), bindings);
}

std::string Writer::
serialize() const
{
    return _buffer.str();
}

