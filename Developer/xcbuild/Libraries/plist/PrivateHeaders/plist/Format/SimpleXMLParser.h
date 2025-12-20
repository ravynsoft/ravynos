/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_SimpleXMLParser_h
#define __plist_Format_SimpleXMLParser_h

#include <plist/Format/BaseXMLParser.h>
#include <plist/Dictionary.h>

#ifdef __linux__
#include <cstdint>
#endif

namespace plist {
namespace Format {

class SimpleXMLParser : public BaseXMLParser {
private:
    Dictionary                 *_root;
    Dictionary                 *_current;
    std::vector <Dictionary *>  _stack;

public:
    SimpleXMLParser();

public:
    Dictionary *parse(std::vector<uint8_t> const &contents);

private:
    virtual void onBeginParse();
    virtual void onEndParse(bool success);

private:
    void onStartElement(std::string const &name, std::unordered_map<std::string, std::string> const &attrs, size_t);
    void onEndElement(std::string const &name, size_t);
};

}
}

#endif  // !__plist_Format_SimpleXMLParser_h
