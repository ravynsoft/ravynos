/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/XML.h>
#include <plist/Format/XMLParser.h>
#include <plist/Format/XMLWriter.h>

using plist::Format::Type;
using plist::Format::Encoding;
using plist::Format::Format;
using plist::Format::XML;
using plist::Format::XMLParser;
using plist::Format::XMLWriter;
using plist::Object;

XML::
XML(Encoding encoding) :
    _encoding(encoding)
{
}

Type XML::
FormatType()
{
    return Type::XML;
}

namespace plist { namespace Format {

template<>
std::unique_ptr<XML> Format<XML>::
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
            return std::unique_ptr<XML>(new XML(XML::Create(encoding)));
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
std::pair<std::unique_ptr<Object>, std::string> Format<XML>::
Deserialize(std::vector<uint8_t> const &contents, XML const &format)
{
    std::vector<uint8_t> const data = Encodings::Convert(contents, format.encoding(), Encoding::UTF8);

    XMLParser parser;
    std::unique_ptr<Object> root = std::unique_ptr<Object>(parser.parse(data));
    if (root == nullptr) {
        return std::make_pair(nullptr, parser.error());
    }

    return std::make_pair(std::move(root), std::string());
}

template<>
std::pair<std::unique_ptr<std::vector<uint8_t>>, std::string> Format<XML>::
Serialize(Object const *object, XML const &format)
{
    if (object == nullptr) {
        return std::make_pair(nullptr, "object was null");
    }

    XMLWriter writer = XMLWriter(object);
    if (!writer.write()) {
        return std::make_pair(nullptr, "serialization failed");
    }

    std::vector<uint8_t> const data = Encodings::Convert(writer.contents(), Encoding::UTF8, format.encoding());

    return std::make_pair(std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(data.begin(), data.end())), std::string());
}

} }

XML XML::
Create(Encoding encoding)
{
    return XML(encoding);
}
