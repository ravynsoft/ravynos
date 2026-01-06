/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __acdriver_Result_h
#define __acdriver_Result_h

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <ext/optional>

namespace plist { class Array; }
namespace plist { class Object; }

namespace acdriver {

class Output;

/*
 * Represents the errors in a driver invocation.
 */
class Result {
public:
    /*
     * The severity of the message.
     */
    enum class Severity {
        /*
         * An unrecoverable error.
         */
        Error,
        /*
         * A dangerous warning.
         */
        Warning,
        /*
         * A noteworthy notice.
         */
        Notice,
    };

private:
    struct NormalEntry {
        Severity                   severity;
        std::string                message;
        ext::optional<std::string> reason;
        ext::optional<std::string> file;
    };

    struct DocumentEntry {
        Severity                 severity;
        std::string              catalog;
        std::vector<std::string> items;
        std::string              type;
        std::string              message;
    };

private:
    std::unordered_map<std::string, std::vector<NormalEntry>>   _normalEntries;
    std::unordered_map<std::string, std::vector<DocumentEntry>> _documentEntries;

public:
    Result();
    ~Result();

public:
    /*
     * If the result has not hit any serious errors.
     */
    bool success() const;

public:
    /*
     * Log a normal message.
     */
    void normal(
        Severity severity,
        std::string const &message,
        ext::optional<std::string> const &reason = ext::nullopt,
        ext::optional<std::string> const &file = ext::nullopt);

    /*
     * Log a message associated with a document.
     */
    void document(
        Severity severity,
        std::string const &catalog,
        std::vector<std::string> const &items,
        std::string const &type,
        std::string const &message);

public:
    /*
     * The structured serialization of the normal messages.
     */
    std::unique_ptr<plist::Array> normalArray(Severity severity) const;

    /*
     * The text serialization of normal messages logged.
     */
    ext::optional<std::string> normalText(Severity severity) const;

    /*
     * The structured serialization of the document messages.
     */
    std::unique_ptr<plist::Array> documentArray(Severity severity) const;

    /*
     * The text serialization of the document messages logged.
     */
    ext::optional<std::string> documentText(Severity severity) const;

    /*
     * Adds the contents of the result to an output.
     */
    void write(Output *output) const;

private:
    static std::unique_ptr<plist::Object> NormalEntryValue(NormalEntry const &entry);
    static std::string NormalEntryText(NormalEntry const &entry);
    static std::unique_ptr<plist::Object> DocumentEntryValue(DocumentEntry const &entry);
    static std::string DocumentEntryText(DocumentEntry const &entry);
};

}

#endif // !__acdriver_Result_h

