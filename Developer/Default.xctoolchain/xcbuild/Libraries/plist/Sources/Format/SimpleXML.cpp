/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/SimpleXML.h>
#include <plist/Format/SimpleXMLParser.h>

using plist::Format::Encoding;
using plist::Format::Format;
using plist::Format::SimpleXML;
using plist::Format::SimpleXMLParser;
using plist::Object;

SimpleXML::
SimpleXML(Encoding encoding) :
    _encoding(encoding)
{
}

namespace plist { namespace Format {

template<>
std::unique_ptr<SimpleXML> Format<SimpleXML>::
Identify(std::vector<uint8_t> const &contents)
{
    /*
     * To identify XML document, we look for a <? or <!, ignoring
     * any whitespace or UTF BOM characters.
     */

    uint8_t last = '\0';

    for (auto bp = contents.begin(); bp != contents.end();) {
        /* Conceal zeroes for UTF-16/32 encodings. */
        if (*bp == 0 || isspace(*bp)) {
            bp++;
        } else if (last == 0 && *bp == '<') {
            last = *bp++;
        } else if (last == '<') {
            if (*bp == '?' || *bp == '!') {
                /* Found <? or <! */
            }

            Encoding encoding = Encodings::Detect(contents);
            return std::unique_ptr<SimpleXML>(new SimpleXML(SimpleXML::Create(encoding)));
        } else if (bp - contents.begin() < 4) {
            /*
             * We conceal some BOM chars for UTF encodings in the first
             * four bytes.
             */
            switch (*bp) {
                case 0xfe: /* UTF-16/32 */
                case 0xff:
                case 0xef: /* UTF-8 */
                case 0xbb:
                case 0xbf:
                    bp++;
                    break;
                default:
                    return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    return nullptr;
}

template<>
std::pair<std::unique_ptr<Object>, std::string> Format<SimpleXML>::
Deserialize(std::vector<uint8_t> const &contents, SimpleXML const &format)
{
    std::vector<uint8_t> const data = Encodings::Convert(contents, format.encoding(), Encoding::UTF8);

    SimpleXMLParser parser;
    std::unique_ptr<Object> root = std::unique_ptr<Object>(parser.parse(data));
    if (root == nullptr) {
        return std::make_pair(nullptr, parser.error());
    }

    return std::make_pair(std::move(root), std::string());
}

template<>
std::pair<std::unique_ptr<std::vector<uint8_t>>, std::string> Format<SimpleXML>::
Serialize(Object const *object, SimpleXML const &format)
{
    return std::make_pair(nullptr, "not yet implemented");
}

} }

SimpleXML SimpleXML::
Create(Encoding encoding)
{
    return SimpleXML(encoding);
}
