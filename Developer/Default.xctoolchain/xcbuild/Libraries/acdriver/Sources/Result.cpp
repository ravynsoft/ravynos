/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <acdriver/Result.h>
#include <acdriver/Output.h>
#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/Object.h>
#include <plist/String.h>

#include <cstdlib>

using acdriver::Result;
using acdriver::Output;

Result::
Result()
{
}

Result::
~Result()
{
}

static std::string
DocumentSeverityKey(Result::Severity severity)
{
    switch (severity) {
        case Result::Severity::Error:
            return "com.apple.actool.document.errors";
        case Result::Severity::Warning:
            return "com.apple.actool.document.warnings";
        case Result::Severity::Notice:
            return "com.apple.actool.document.notices";
    }

    abort();
}

static std::string
NormalSeverityKey(Result::Severity severity)
{
    switch (severity) {
        case Result::Severity::Error:
            return "com.apple.actool.errors";
        case Result::Severity::Warning:
            return "com.apple.actool.warnings";
        case Result::Severity::Notice:
            return "com.apple.actool.notices";
    }

    abort();
}

void Result::
normal(
    Severity severity,
    std::string const &message,
    ext::optional<std::string> const &reason,
    ext::optional<std::string> const &file)
{
    NormalEntry entry;
    entry.severity = severity;
    entry.message = message;
    entry.reason = reason;
    entry.file = file;
    _normalEntries[NormalSeverityKey(severity)].push_back(entry);
}

void Result::
document(
    Severity severity,
    std::string const &catalog,
    std::vector<std::string> const &items,
    std::string const &type,
    std::string const &message)
{
    DocumentEntry entry;
    entry.severity = severity;
    entry.catalog = catalog;
    entry.items = items;
    entry.type = type;
    entry.message = message;
    _documentEntries[DocumentSeverityKey(severity)].push_back(entry);
}

bool Result::
success() const
{
    auto nit = _normalEntries.find(NormalSeverityKey(Severity::Error));
    auto dit = _documentEntries.find(DocumentSeverityKey(Severity::Error));

    bool nvalid = (nit != _normalEntries.end());
    bool dvalid = (dit != _documentEntries.end());

    return (!nvalid || nit->second.empty()) && (!dvalid || dit->second.empty());
}

static std::string
SeverityText(Result::Severity severity)
{
    switch (severity) {
        case Result::Severity::Error:
            return "error";
        case Result::Severity::Warning:
            return "warning";
        case Result::Severity::Notice:
            return "notice";
    }

    abort();
}

std::unique_ptr<plist::Object> Result::
NormalEntryValue(NormalEntry const &entry)
{
    std::unique_ptr<plist::Dictionary> dict = plist::Dictionary::New();
    dict->set("description", plist::String::New(entry.message));
    if (entry.reason) {
        dict->set("failure-reason", plist::String::New(*entry.reason));
    }
    return plist::static_unique_pointer_cast<plist::Object>(std::move(dict));
}

std::string Result::
NormalEntryText(NormalEntry const &entry)
{
    std::string result;

    if (entry.file) {
        result += *entry.file;
    }
    result += ": ";

    result += SeverityText(entry.severity);
    result += ": ";

    result += entry.message;

    if (entry.reason) {
        result += "\n";
        result += "    ";
        result += "Failure Reason: ";
        result += *entry.reason;
    }

    return result;
}

std::unique_ptr<plist::Object> Result::
DocumentEntryValue(DocumentEntry const &entry)
{
    std::unique_ptr<plist::Dictionary> dict = plist::Dictionary::New();
    dict->set("catalog", plist::String::New(entry.catalog));

    std::unique_ptr<plist::Array> items = plist::Array::New();
    for (std::string const &item : entry.items) {
        items->append(plist::String::New(item));
    }
    dict->set("affected-items", std::move(items));

    dict->set("type", plist::String::New(entry.type));
    dict->set("message", plist::String::New(entry.message));
    return plist::static_unique_pointer_cast<plist::Object>(std::move(dict));
}

std::string Result::
DocumentEntryText(DocumentEntry const &entry)
{
    std::string result;

    result += entry.catalog;
    result += ":";
    for (std::string const &item : entry.items) {
        result += item;
        result += ":";
    }
    result += " ";

    result += SeverityText(entry.severity);
    result += ": ";

    result += entry.message;
    return result;
}

std::unique_ptr<plist::Array> Result::
normalArray(Severity severity) const
{
    std::unique_ptr<plist::Array> array;

    auto it = _normalEntries.find(NormalSeverityKey(severity));
    if (it != _normalEntries.end()) {
        for (NormalEntry const &entry : it->second) {
            if (array == nullptr) {
                array = plist::Array::New();
            }

            std::unique_ptr<plist::Object> value = NormalEntryValue(entry);
            array->append(std::move(value));
        }
    }

    return array;
}

ext::optional<std::string> Result::
normalText(Severity severity) const
{
    ext::optional<std::string> text;

    auto it = _normalEntries.find(NormalSeverityKey(severity));
    if (it != _normalEntries.end()) {
        for (NormalEntry const &entry : it->second) {
            if (!text) {
                text = std::string();
            }

            *text += NormalEntryText(entry);
            *text += "\n";
        }
    }

    return text;
}

std::unique_ptr<plist::Array> Result::
documentArray(Severity severity) const
{
    std::unique_ptr<plist::Array> array;

    auto it = _documentEntries.find(DocumentSeverityKey(severity));
    if (it != _documentEntries.end()) {
        for (DocumentEntry const &entry : it->second) {
            if (array == nullptr) {
                array = plist::Array::New();
            }

            std::unique_ptr<plist::Object> value = DocumentEntryValue(entry);
            array->append(std::move(value));
        }
    }

    return array;
}

ext::optional<std::string> Result::
documentText(Severity severity) const
{
    ext::optional<std::string> text;

    auto it = _documentEntries.find(DocumentSeverityKey(severity));
    if (it != _documentEntries.end()) {
        for (DocumentEntry const &entry : it->second) {
            if (!text) {
                text = std::string();
            }

            *text += DocumentEntryText(entry);
            *text += "\n";
        }
    }

    return text;
}

void Result::
write(Output *output) const
{
    Severity severities[] = {
        Severity::Error,
        Severity::Warning,
        Severity::Notice,
    };

    /*
     * Add normal entries/.
     */
    for (Severity severity : severities) {
        std::unique_ptr<plist::Array> array = normalArray(severity);
        ext::optional<std::string> text = normalText(severity);
        if (array != nullptr && text) {
            output->add(NormalSeverityKey(severity), std::move(array), *text);
        }
    }

    /*
     * Add document entries.
     */
    for (Severity severity : severities) {
        std::unique_ptr<plist::Array> array = documentArray(severity);
        ext::optional<std::string> text = documentText(severity);
        if (array != nullptr && text) {
            output->add(DocumentSeverityKey(severity), std::move(array), *text);
        }
    }
}

