/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Output.h>
#include <plist/Object.h>
#include <plist/Dictionary.h>
#include <plist/Format/Binary.h>
#include <plist/Format/Encoding.h>
#include <plist/Format/XML.h>

#include <cstdlib>

using acdriver::Output;

Output::
Output() :
    _values(plist::Dictionary::New())
{
}

Output::
~Output()
{
}

void Output::
add(std::string const &key, std::unique_ptr<plist::Object> value, std::string const &text)
{
    _values->set(key, std::move(value));
    _texts.insert({ key, text });
}

ext::optional<std::vector<uint8_t>> Output::
serialize(Format format) const
{
    switch (format) {
        case Format::XML: {
            if (_values->empty()) {
                return std::vector<uint8_t>();
            }

            auto serialize = plist::Format::XML::Serialize(_values.get(), plist::Format::XML::Create(plist::Format::Encoding::UTF8));
            if (serialize.first == nullptr) {
                return ext::nullopt;
            }
            return *serialize.first;
        }
        case Format::Binary: {
            if (_values->empty()) {
                return std::vector<uint8_t>();
            }

            auto serialize = plist::Format::Binary::Serialize(_values.get(), plist::Format::Binary::Create());
            if (serialize.first == nullptr) {
                return ext::nullopt;
            }
            return *serialize.first;
        }
        case Format::Text: {
            std::string output;
            for (size_t n = 0; n < _values->count(); ++n) {
                std::string key = _values->key(n);
                std::string text = _texts.at(key);

                output += "/* " + key + " */\n";
                output += text;
                output += "\n";
            }
            return std::vector<uint8_t>(output.begin(), output.end());
        }
    }

    abort();
}

