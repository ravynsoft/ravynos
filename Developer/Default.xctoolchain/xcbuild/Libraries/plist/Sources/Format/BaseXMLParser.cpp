/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/BaseXMLParser.h>

#include <algorithm>
#include <mutex>
#include <cerrno>
#include <cstdlib>
#include <cstdarg>

#if _WIN32
#include <windows.h>
#endif

using plist::Format::BaseXMLParser;

#if _WIN32
using WideString = std::basic_string<std::remove_const<std::remove_pointer<LPCWSTR>::type>::type>;

static std::string
WideStringToString(WideString const &str)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0, NULL, NULL);
    std::string multi = std::string();
    multi.resize(size);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), &multi[0], size, NULL, NULL);
    return multi;
}
#endif

BaseXMLParser::BaseXMLParser() :
#if _WIN32
    _reader (nullptr),
#else
    _parser (nullptr),
#endif
    _errored(false)
{
}

#if _WIN32
static IXmlReader *
CreateXmlReaderImpl(IMalloc *pMalloc)
{
    static decltype(CreateXmlReader) *create = nullptr;
    static decltype(IID_IXmlReader) *uuid = nullptr;

#if __MINGW32__
    /* MinGW is missing the library to link against XmlLite. */
    std::once_flag flag;
    std::call_once(flag, []{
        HMODULE module = LoadLibraryA("XmlLite.dll");
        if (module == nullptr) {
            return;
        }

        create = reinterpret_cast<decltype(CreateXmlReader) *>(GetProcAddress(module, "CreateXmlReader"));
        uuid = reinterpret_cast<decltype(IID_IXmlReader) *>(new GUID({ 0x7279fc81, 0x709d, 0x4095, { 0xb6, 0x3d, 0x69, 0xfe, 0x4b, 0x0d, 0x90, 0x30 } }));
    });
#else
    create = &CreateXmlReader;
    uuid = &IID_IXmlReader;
#endif

    if (create == nullptr || uuid == nullptr) {
        return nullptr;
    }

    IXmlReader *reader = nullptr;
    if (create(*uuid, reinterpret_cast<void **>(&reader), nullptr) != S_OK) {
        return nullptr;
    }

    return reader;
}
#endif

bool BaseXMLParser::
parse(std::vector<uint8_t> const &contents)
{
    _errored = false;

#if _WIN32
    std::vector<uint8_t> contents_ = contents;

    bool wine = (GetProcAddress(GetModuleHandle("ntdll.dll"), "wine_get_version") != nullptr);
    if (wine) {
        /*
         * Wine's XmlLite can't handle "DOCTYPE PUBLIC" with two parameters. Work around by
         * removing the doctype completely. Should continue to work even after Wine is fixed.
         */
        static char const *doctype_start = "<!DOCTYPE";
        auto doctype_start_it = std::search(contents_.begin(), contents_.end(), doctype_start, doctype_start + strlen(doctype_start));
        if (doctype_start_it != contents_.end()) {
            static char const *doctype_end = ">";
            auto doctype_end_it = std::search(doctype_start_it, contents_.end(), doctype_end, doctype_end + strlen(doctype_end));
            if (doctype_end_it != contents_.end()) {
                contents_.erase(doctype_start_it, std::next(doctype_end_it));
            }
        }
    }
    /*
     * Wine's XmlLite calculates depth incorrectly. Rather than adjust the result, use a
     * custom implementation of depth.
     */
    UINT wineDepth = 0;

    /*
     * Create memory handle.
     */
    auto memory = GlobalAlloc(GMEM_MOVEABLE, contents_.size());
    if (memory == nullptr) {
        return false;
    }
    void *memoryData = GlobalLock(memory);
    if (memoryData == nullptr) {
        GlobalFree(memory);
        return false;
    }
    memcpy(memoryData, contents_.data(), contents_.size());
    if (GlobalUnlock(memory) || GetLastError() != NO_ERROR) {
        GlobalFree(memory);
        return false;
    }

    /*
     * Create stream for data.
     */
    IStream *stream = nullptr;
    if (CreateStreamOnHGlobal(memory, FALSE, &stream) != S_OK) {
        GlobalFree(memory);
        return false;
    }

    /*
     * Create XML reader attached to stream.
     */
    _reader = CreateXmlReaderImpl(nullptr);
    if (_reader == nullptr) {
        IStream_Release(stream);
        GlobalFree(memory);
        return false;
    }
    if (IXmlReader_SetProperty(_reader, XmlReaderProperty_DtdProcessing, DtdProcessing_Parse) != S_OK) {
        IStream_Release(stream);
        GlobalFree(memory);
        return false;
    }
    if (IXmlReader_SetInput(_reader, reinterpret_cast<IUnknown *>(stream)) != S_OK) {
        IXmlReader_Release(_reader);
        _reader = nullptr;
        IStream_Release(stream);
        GlobalFree(memory);
        return false;
    }

    /*
     * Parse XML document.
     */
    onBeginParse();

    XmlNodeType type;
    HRESULT ret = IXmlReader_Read(_reader, &type);
    while (ret == S_OK) {
        /*
         * Get parser depth.
         */
        UINT depth = 0;
        ret = IXmlReader_GetDepth(_reader, &depth);
        if (ret != S_OK) {
            break;
        }

        /* See above for workaround. */
        if (wine) {
            depth = wineDepth;
        }

        if (type == XmlNodeType_Element) {
            /*
             * Read element name.
             */
            LPCWSTR elementString = nullptr;
            UINT elementLength = 0;
            ret = IXmlReader_GetQualifiedName(_reader, &elementString, &elementLength);
            if (ret != S_OK) {
                break;
            }
            auto element = WideString(elementString, elementString + elementLength);

            /*
             * Read all attributes.
             */
            std::unordered_map<std::string, std::string> attrs;
            ret = IXmlReader_MoveToFirstAttribute(_reader);
            while (ret == S_OK) {
                /* Store attribute. */
                LPCWSTR nameString = nullptr;
                UINT nameLength = 0;
                ret = IXmlReader_GetLocalName(_reader, &nameString, &nameLength);
                if (ret != S_OK) {
                    break;
                }

                LPCWSTR valueString = nullptr;
                UINT valueLength = 0;
                ret = IXmlReader_GetValue(_reader, &valueString, &valueLength);
                if (ret != S_OK) {
                    break;
                }

                auto name = WideString(nameString, nameString + nameLength);
                auto value = WideString(valueString, valueString + valueLength);
                attrs[WideStringToString(name)] = WideStringToString(value);

                ret = IXmlReader_MoveToNextAttribute(_reader);
            }

            /* Handle error. */
            if (ret != S_FALSE) {
                break;
            }

            BOOL empty = IXmlReader_IsEmptyElement(_reader);
            onStartElement(WideStringToString(element), attrs, static_cast<size_t>(depth));
            if (empty) {
                /* Empty element ends immediately. */
                onEndElement(WideStringToString(element), static_cast<size_t>(depth));
            } else {
                /* See above for workaround. */
                if (wine) {
                    wineDepth += 1;
                }
            }
        } else if (type == XmlNodeType_EndElement) {
            LPCWSTR elementString = nullptr;
            UINT elementLength = 0;
            ret = IXmlReader_GetQualifiedName(_reader, &elementString, &elementLength);
            if (ret != S_OK) {
                break;
            }
            auto element = WideString(elementString, elementString + elementLength);

            onEndElement(WideStringToString(element), static_cast<size_t>(depth));

            /* See above for workaround. */
            if (wine) {
                wineDepth -= 1;
            }
        } else if (type == XmlNodeType_Text) {
            LPCWSTR value = nullptr;
            UINT length = 0;
            ret = IXmlReader_GetValue(_reader, &value, &length);
            if (ret != S_OK) {
                break;
            }

            auto buffer = WideString(value, value + length);
            onCharacterData(WideStringToString(buffer), static_cast<size_t>(depth));
        }

        /* Handle error. */
        if (_errored) {
            ret = -1;
            break;
        }

        ret = IXmlReader_Read(_reader, &type);
    }

    onEndParse(ret == S_FALSE);

    /*
     * Free XML reader.
     */
    IXmlReader_Release(_reader);
    _reader = nullptr;
    IStream_Release(stream);
    GlobalFree(memory);

    return (ret == S_FALSE);
#else
    _parser = ::xmlReaderForMemory(reinterpret_cast<char const *>(contents.data()), contents.size(), nullptr, nullptr, XML_PARSE_NOENT | XML_PARSE_NONET);
    if (_parser == nullptr) {
        return false;
    }

    onBeginParse();

    int ret = xmlTextReaderRead(_parser);
    while (ret == 1) {
        size_t depth = xmlTextReaderDepth(_parser);

        int type = xmlTextReaderNodeType(_parser);
        if (type == 1 /* Start element. */) {
            std::unordered_map<std::string, std::string> attrs;

            ret = xmlTextReaderMoveToFirstAttribute(_parser);
            while (ret == 1) {
                /* Store attribute. */
                xmlChar const *name = xmlTextReaderConstName(_parser);
                xmlChar const *value = xmlTextReaderConstValue(_parser);
                attrs[std::string(reinterpret_cast<char const *>(name))] = std::string(reinterpret_cast<char const *>(value));

                ret = xmlTextReaderMoveToNextAttribute(_parser);
            }

            /* Handle error. */
            if (ret != 0) {
                break;
            }

            ret = xmlTextReaderMoveToElement(_parser);
            if (ret != 0 && ret != 1) {
                break;
            }

            /* Check for empty element. Before onStart in case of error. */
            ret = xmlTextReaderIsEmptyElement(_parser);
            if (ret != 0 && ret != 1) {
                break;
            }

            xmlChar const *name = xmlTextReaderConstName(_parser);
            onStartElement(std::string(reinterpret_cast<char const *>(name)), attrs, depth);

            if (ret == 1) {
                /* Empty element. */
                onEndElement(std::string(reinterpret_cast<char const *>(name)), depth);
            }
        } else if (type == 15 /* End element. */) {
            xmlChar const *name = xmlTextReaderConstName(_parser);
            onEndElement(std::string(reinterpret_cast<char const *>(name)), depth);
        } else if (type == 3 /* Text. */) {
            xmlChar const *value = xmlTextReaderConstValue(_parser);
            onCharacterData(std::string(reinterpret_cast<char const *>(value)), depth);
        }

        /* Handle error. */
        if (_errored) {
            ret = -1;
            break;
        }

        ret = xmlTextReaderRead(_parser);
    }

    ::xmlFreeTextReader(_parser);
    _parser = nullptr;

    onEndParse(ret == 0);

    return (ret == 0);
#endif
}

void BaseXMLParser::
onBeginParse()
{
}

void BaseXMLParser::
onEndParse(bool success)
{
}

void BaseXMLParser::
onStartElement(std::string const &name, std::unordered_map<std::string, std::string> const &attrs, size_t depth)
{
}

void BaseXMLParser::
onEndElement(std::string const &name, size_t depth)
{
}

void BaseXMLParser::
onCharacterData(std::string const &cdata, size_t depth)
{
}

void BaseXMLParser::
error(std::string format, ...)
{
    _errored = true;

#if _WIN32
    UINT line;
    if (IXmlReader_GetLineNumber(_reader, &line) != S_OK) {
        _line = 0;
    } else {
        _line = static_cast<size_t>(line);
    }

    UINT column;
    if (IXmlReader_GetLinePosition(_reader, &column) != S_OK) {
        _column = 0;
    } else {
        _column = static_cast<size_t>(column);
    }
#else
    _line = ::xmlTextReaderGetParserLineNumber(_parser);
    _column = ::xmlTextReaderGetParserColumnNumber(_parser);
#endif

    va_list ap1, ap2;
    va_start(ap1, format);
    va_copy(ap2, ap1);

    /* Get size of formatted string. */
#if _WIN32
    int ret = ::_vscprintf(format.c_str(), ap1);
#else
    int ret = ::vsnprintf(NULL, 0, format.c_str(), ap1);
#endif
    if (ret < 0) {
        _error = "error formatting error";
    } else {
        std::string buffer;
        buffer.resize(ret);

        /* Format error message. */
        ret = ::vsnprintf(&buffer[0], buffer.size(), format.c_str(), ap2);
        if (ret < 0) {
            _error = "error formatting error";
        } else {
            _error = buffer;
        }
    }

    va_end(ap1);
    va_end(ap2);
}
