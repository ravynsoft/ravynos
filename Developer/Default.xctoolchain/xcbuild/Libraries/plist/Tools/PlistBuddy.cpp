/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Data.h>
#include <plist/Date.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/Real.h>
#include <plist/String.h>
#include <plist/Object.h>
#include <plist/Format/Any.h>
#include <plist/Format/ASCII.h>
#include <plist/Format/Binary.h>
#include <plist/Format/Encoding.h>
#include <plist/Format/XML.h>
#include <libutil/Options.h>
#include <libutil/Filesystem.h>
#include <libutil/DefaultFilesystem.h>

#include <iostream>
#include <iterator>
#include <queue>
#include <sstream>
#include <string>

#if defined(HAVE_LINENOISE)
#include <linenoise.h>
#endif

using libutil::Filesystem;
using libutil::DefaultFilesystem;

class Options {
private:
    ext::optional<bool>         _help;
    ext::optional<bool>         _xml;

private:
    ext::optional<std::string>  _command;

private:
    std::string                 _input;

public:
    Options();
    ~Options();

public:
    bool help() const
    { return _help.value_or(false); }
    bool xml() const
    { return _xml.value_or(false); }

public:
    ext::optional<std::string> command() const
    { return _command; }

public:
    std::string input() const
    { return _input; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);

};

Options::
Options()
{
}

Options::
~Options()
{
}

std::pair<bool, std::string> Options::
parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
    std::string const &arg = **it;

    if (arg == "-h") {
        return libutil::Options::Current(&_help, arg);
    } else if (arg == "-x") {
        return libutil::Options::Current(&_xml, arg);
    } else if (arg == "-c") {
        return libutil::Options::Next<std::string>(&_command, args, it);
    } else if (!arg.empty() && arg[0] != '-') {
        _input = arg;
        return std::make_pair(true, std::string());
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

static plist::ObjectType
ParseType(std::string const &typeString) {
    if (typeString == "string") {
        return plist::ObjectType::String;
    } else if (typeString == "dictionary") {
        return plist::ObjectType::Dictionary;
    } else if (typeString == "array") {
        return plist::ObjectType::Array;
    } else if (typeString == "bool") {
        return plist::ObjectType::Boolean;
    } else if (typeString == "real") {
        return plist::ObjectType::Real;
    } else if (typeString == "integer") {
        return plist::ObjectType::Integer;
    } else if (typeString == "date") {
        return plist::ObjectType::Date;
    } else if (typeString == "data") {
        return plist::ObjectType::Data;
    } else {
        return plist::ObjectType::None;
    }
}

static int
Help(std::string const &error = std::string())
{
    if (!error.empty()) {
        fprintf(stderr, "Error: %s\n", error.c_str());
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "Usage: PlistBuddy [options] <file.plist>\n");

#define INDENT "  "
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, INDENT "-c \"<command>\" command to execute, otherwise run in interactive mode\n");
    fprintf(stderr, INDENT "-x output will be in xml plist format\n");
    fprintf(stderr, INDENT "-h print help including commands\n");
#undef INDENT

    return (error.empty() ? 0 : -1);
}

static void
CommandHelp()
{
#define INDENT "  "
    fprintf(stderr, "\nCommands help:\n");
    fprintf(stderr, INDENT "Help - Print this information\n");
    fprintf(stderr, INDENT "Exit - Exits this program\n");
    fprintf(stderr, INDENT "Save - Save the changed plist file\n");
    fprintf(stderr, INDENT "Revert - Revert to the saved plist file\n");
    fprintf(stderr, INDENT "Clear <Type> - Clears all data, and sets root to an empty Type\n");
    fprintf(stderr, INDENT "Print [<KeyPath>] - Print value at KeyPath or root\n");
    fprintf(stderr, INDENT "Set <KeyPath> <Value> - Set value at KeyPath to Value\n");
    fprintf(stderr, INDENT "Add <KeyPath> <Type> <Value> - Set value at KeyPath to Value\n");
    fprintf(stderr, INDENT "Copy <SrcKeyPath> <DstKeyPath> - Copy SrcKeyPath to DstKeyPath \n");
    fprintf(stderr, INDENT "Delete <KeyPath> - Removes entry at KeyPath\n");
    fprintf(stderr, INDENT "Merge <File> [<KeyPath>] - Merges data from <File> to KeyPath or root\n");
    fprintf(stderr, INDENT "Import <KeyPath> <File> - Import <File> as data at <KeyPath>\n");

    fprintf(stderr, "\n<KeyPath>\n");
    fprintf(stderr, INDENT ":= \"\"                             => root object\n");
    fprintf(stderr, INDENT ":= <KeyPath>[:<Dictionary Key>]   => indexes into dictionary\n");
    fprintf(stderr, INDENT ":= <KeyPath>[:<Array Index>]      => indexes into Array\n");
    fprintf(stderr, "\n<Type> := (string|dictionary|array|bool|real|integer|date|data)\n\n");
#undef INDENT
}

static void
ParseCommandKeyPathString(std::string const &keyPathString, std::queue<std::string> *keyPath)
{
    std::string::size_type prev = 0;
    do {
        std::string::size_type pos = keyPathString.find(':', prev);
        std::string::size_type end = (pos == std::string::npos ? pos : pos - prev);

        /* Skip leading colon representing the root object. */
        bool emptyInitialKey = (prev == 0 && pos == 0);
        if (!emptyInitialKey) {
            std::string key = keyPathString.substr(prev, end);
            keyPath->push(key);
        }

        prev = (pos == std::string::npos ? pos : pos + 1);
    } while (prev != std::string::npos);
}

static std::string
ParseCommandValueString(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end)
{
    if (begin == end) {
        return "";
    }

    std::string value;
    for (auto it = begin; it != end; it++) {
        if (it != begin) {
            value += ' ';
        }
        value += it->c_str();
    }

    /* Strip double quotes if necessary. */
    if (value.front() == '\"' && value.back() == '\"') {
        value = value.substr(1, value.size() - 2);
    }

    return value;
}

static plist::Object *
GetObjectAtKeyPath(plist::Object *object, std::queue<std::string> *keyPath, bool leaveLastKey = true)
{
    while (keyPath->size() > (leaveLastKey ? 1 : 0)) {
        if (object == nullptr) {
            fprintf(stderr, "Invalid key path (indexing into null object)\n");
            return nullptr;
        }

        std::string const &key = keyPath->front();

        plist::Object *next = nullptr;
        if (plist::Dictionary *dictionary = plist::CastTo<plist::Dictionary>(object)) {
            next = dictionary->value(key);
        } else if (plist::Array *array = plist::CastTo<plist::Array>(object)) {
            char *end = NULL;
            long long index = std::strtoll(key.c_str(), &end, 0);
            if (end == key.c_str() || index < 0) {
                fprintf(stderr, "Invalid array index\n");
                return nullptr;
            }

            if (static_cast<size_t>(index) >= array->count()) {
                fprintf(stderr, "Invalid array index\n");
                return nullptr;
            }

            next = array->value(static_cast<size_t>(index));
        } else {
            /* Reached a non-collection object with remaining key path, error. */
            fprintf(stderr, "Invalid key path (indexing into non-collection object)\n");
            return nullptr;
        }

        object = next;
        keyPath->pop();
    }

    return object;
}

static bool
SetObjectAtKeyPath(plist::Object *object, std::unique_ptr<plist::Object> value, std::queue<std::string> *keyPath, bool overwrite)
{
    if (keyPath->size() != 1) {
        fprintf(stderr, "Invalid key path (target object not found)\n");
        return false;
    }

    std::string const &key = keyPath->front();

    if (plist::Dictionary *dict = plist::CastTo<plist::Dictionary>(object)) {
        if (value == nullptr) {
            dict->remove(key);
        } else {
            if (!overwrite && dict->value(key) != nullptr) {
                fprintf(stderr, "Cannot overwrite key path\n");
                return false;
            }
            dict->set(key, std::move(value));
        }
    } else if (plist::Array *array = plist::CastTo<plist::Array>(object)) {
        if (key.empty()) {
            if (value != nullptr) {
                /* No index, append to array. */
                array->append(std::move(value));
            } else {
                fprintf(stderr, "Invalid key path (can't remove end of array)\n");
                return false;
            }
        } else {
            char *end = NULL;
            long long index = std::strtoll(key.c_str(), &end, 0);
            if (end == key.c_str() || index < 0) {
                fprintf(stderr, "Invalid array index\n");
                return false;
            }

            if (static_cast<size_t>(index) >= array->count()) {
                fprintf(stderr, "Invalid array index\n");
                return false;
            }

            if (value == nullptr) {
                array->remove(static_cast<size_t>(index));
            } else if (overwrite) {
                array->set(static_cast<size_t>(index), std::move(value));
            } else {
                array->insert(static_cast<size_t>(index), std::move(value));
            }
        }
    } else {
        fprintf(stderr, "Invalid key path (setting value on non-collection object)\n");
        return false;
    }

    return true;
}

static bool
Save(plist::Object *root, plist::Format::Any const &format, Filesystem *filesystem, std::string const &path)
{
    auto serialize = plist::Format::Any::Serialize(root, format);
    if (serialize.first == nullptr) {
        fprintf(stderr, "Error: %s\n", serialize.second.c_str());
        return false;
    }

    if (!filesystem->write(*serialize.first, path)) {
        fprintf(stderr, "Could not write to output\n");
        return false;
    }

    return true;
}

static bool
Print(plist::Object *object, std::queue<std::string> *keyPath, plist::Format::Any const &format)
{
    plist::Object *target = GetObjectAtKeyPath(object, keyPath, false);
    if (!keyPath->empty() || target == nullptr) {
        fprintf(stderr, "Invalid key path (no object at key path)\n");
        return false;
    }

    auto serialize = plist::Format::Any::Serialize(target, format);
    if (serialize.first == nullptr) {
        fprintf(stderr, "Error: %s\n", serialize.second.c_str());
        return false;
    }

    /* Print. */
    std::copy(serialize.first->begin(), serialize.first->end(), std::ostream_iterator<char>(std::cout));
    return true;
}

static bool
Revert(std::unique_ptr<plist::Object> *root, Filesystem const *filesystem, std::string const &path, ext::optional<plist::Format::Any> *format = nullptr)
{
    if (path.empty()) {
        fprintf(stderr, "Error: no input specified\n");
        return false;
    }

    std::vector<uint8_t> contents;
    if (!filesystem->read(&contents, path)) {
        fprintf(stderr, "Error: unable to read %s\n", path.c_str());
        return false;
    }

    auto deserialize = plist::Format::Any::Deserialize(contents);
    if (!deserialize.first) {
        fprintf(stderr, "Error: %s\n", deserialize.second.c_str());
        return false;
    }

    if (format) {
        std::unique_ptr<plist::Format::Any> identifiedFormat = plist::Format::Any::Identify(contents);
        if (identifiedFormat != nullptr) {
            *format = *identifiedFormat;
        }
    }

    *root = std::move(deserialize.first);
    return true;
}

static bool
Set(plist::Object *object, std::queue<std::string> *keyPath, plist::ObjectType type, std::string const &valueString, bool overwrite = true)
{
    plist::Object *target = GetObjectAtKeyPath(object, keyPath, true);
    if (target == nullptr) {
        fprintf(stderr, "Invalid key path (target object not found)\n");
        return false;
    }

    std::unique_ptr<plist::Object> value;
    switch (type) {
        case plist::ObjectType::String:
            value = plist::String::New(valueString);
            break;
        case plist::ObjectType::Dictionary:
            value = plist::Dictionary::New();
            break;
        case plist::ObjectType::Array:
            value = plist::Array::New();
            break;
        case plist::ObjectType::Boolean:
            value = plist::Boolean::Coerce(plist::String::New(valueString).get());
            break;
        case plist::ObjectType::Real:
            value = plist::Real::Coerce(plist::String::New(valueString).get());
            break;
        case plist::ObjectType::Integer:
            value = plist::Integer::Coerce(plist::String::New(valueString).get());
            break;
        case plist::ObjectType::Date:
            value = plist::Date::New(valueString);
            break;
        case plist::ObjectType::Data:
            value = plist::Data::New(valueString);
            break;
        default:
            fprintf(stderr, "Not supported\n");
            return false;
    }

    return SetObjectAtKeyPath(target, std::move(value), keyPath, overwrite);
}

static bool
Clear(std::unique_ptr<plist::Object> *root, plist::ObjectType clearType)
{
    switch (clearType) {
        case plist::ObjectType::String:
            *root = plist::String::New();
            break;
        case plist::ObjectType::Dictionary:
            *root = plist::Dictionary::New();
            break;
        case plist::ObjectType::Array:
            *root = plist::Array::New();
            break;
        case plist::ObjectType::Boolean:
            *root = plist::Boolean::New(false);
            break;
        case plist::ObjectType::Real:
            *root = plist::Real::New();
            break;
        case plist::ObjectType::Integer:
            *root = plist::Integer::New();
            break;
        case plist::ObjectType::Date:
            *root = plist::Date::New();
            break;
        case plist::ObjectType::Data:
            *root = plist::Data::New();
            break;
        default:
            fprintf(stderr, "Unsupported type\n");
            return false;
    }
    return true;
}

static bool
Delete(plist::Object *object, std::queue<std::string> *keyPath)
{
    plist::Object *target = GetObjectAtKeyPath(object, keyPath, true);
    if (target == nullptr) {
        fprintf(stderr, "Invalid key path (target object not found)\n");
        return false;
    }

    return SetObjectAtKeyPath(target, nullptr, keyPath, true);
}

static bool
Copy(plist::Object *object, std::queue<std::string> *srcKeyPath, std::queue<std::string> *dstKeyPath)
{
    plist::Object *source = GetObjectAtKeyPath(object, srcKeyPath, false);
    if (source == nullptr) {
        fprintf(stderr, "Invalid key path (source object not found)\n");
        return false;
    }

    plist::Object *target = GetObjectAtKeyPath(object, dstKeyPath, true);
    if (target == nullptr) {
        fprintf(stderr, "Invalid key path (target object not found)\n");
        return false;
    }

    return SetObjectAtKeyPath(target, source->copy(), dstKeyPath, true);
}

static bool
Merge(plist::Object *object, std::string const &mergeSource, std::queue<std::string> *keyPath, Filesystem const *filesystem)
{
    std::unique_ptr<plist::Object> mergeObject;
    if (!Revert(&mergeObject, filesystem, mergeSource)) {
        fprintf(stderr, "Unable to read merge source file\n");
        return false;
    }

    plist::Object *target = GetObjectAtKeyPath(object, keyPath, false);
    if (target == nullptr) {
        fprintf(stderr, "Invalid key path (no object at key path)\n");
        return false;
    }

    if (auto targetDictionary = plist::CastTo<plist::Dictionary>(target)) {
        if (auto mergeObjectDictionary = plist::CastTo<plist::Dictionary>(mergeObject.get())) {
            // Check to see if duplicate keys exists between target and merge dictionary,
            // and print warning that those keys will be skipped.
            for (auto const &mergeKey : *mergeObjectDictionary) {
                if (targetDictionary->value(mergeKey)) {
                    fprintf(stderr, "Skipping duplicate key: %s\n", mergeKey.c_str());
                }
            }
            targetDictionary->merge(mergeObjectDictionary, false);
            return true;
        } else if (auto mergeObjectArray = plist::CastTo<plist::Array>(mergeObject.get())) {
            (void)mergeObjectArray;
            fprintf(stderr, "Cannot merge array into dictionary\n");
            return false;
        } else {
            /* Add object to empty key. */
            targetDictionary->set("", std::move(mergeObject));
            return true;
        }
    } else if (auto targetArray = plist::CastTo<plist::Array>(target)) {
        if (auto mergeObjectDictionary = plist::CastTo<plist::Dictionary>(mergeObject.get())) {
            (void)mergeObjectDictionary;
            fprintf(stderr, "Cannot merge dictionary into array\n");
            return false;
        } else if (auto mergeObjectArray = plist::CastTo<plist::Array>(mergeObject.get())) {
            targetArray->merge(mergeObjectArray);
            return true;
        } else {
            targetArray->append(std::move(mergeObject));
            return true;
        }
    } else {
        fprintf(stderr, "Object at KeyPath is not a container\n");
        return false;
    }
}

static bool
Import(plist::Object *object, std::queue<std::string> *keyPath, std::string const &source, Filesystem const *filesystem)
{
    plist::Object *target = GetObjectAtKeyPath(object, keyPath, true);
    if (target == nullptr) {
        fprintf(stderr, "Invalid key path (no object at key path)\n");
        return false;
    }

    std::vector<uint8_t> contents;
    if (filesystem->read(&contents, source)) {
        fprintf(stderr, "Could not read source file\n");
        return false;
    }
    std::unique_ptr<plist::Data> value = plist::Data::New(contents);

    return SetObjectAtKeyPath(target, std::move(value), keyPath, true);
}

static bool
ProcessCommand(
    Filesystem *filesystem,
    std::string const &path,
    plist::Format::Any const &printFormat,
    plist::Format::Any const &saveFormat,
    std::unique_ptr<plist::Object> *root,
    std::string const &input,
    bool *mutated,
    bool *keepReading)
{
    std::vector<std::string> tokens;
    std::stringstream sstream(input);
    std::copy(std::istream_iterator<std::string>(sstream), std::istream_iterator<std::string>(), std::back_inserter(tokens));

    if (tokens.size() < 1) {
        return true;
    }

    std::string command = tokens[0];
    if (command == "Print") {
        std::queue<std::string> keyPath;
        if (tokens.size() > 1) {
            ParseCommandKeyPathString(tokens[1], &keyPath);
        }
        return Print(root->get(), &keyPath, printFormat);
    } else if (command == "Save") {
        return Save(root->get(), saveFormat, filesystem, path);
    } else if (command == "Revert") {
        if (!Revert(root, filesystem, path)) {
            return false;
        }
        *mutated = true;
    } else if (command == "Exit") {
        *keepReading = false;
        return true;
    } else if (command == "Set") {
        if (tokens.size() < 2) {
            fprintf(stderr, "Set command requires KeyPath\n");
            return false;
        }

        std::queue<std::string> keyPath;
        ParseCommandKeyPathString(tokens[1], &keyPath);
        std::string value = ParseCommandValueString(tokens.begin() + 2, tokens.end());
        if (!Set(root->get(), &keyPath, plist::ObjectType::String, value)) {
            return false;
        }
        *mutated = true;
    } else if (command == "Add") {
        if (tokens.size() < 3) {
            fprintf(stderr, "Add command requires KeyPath and Type\n");
            return false;
        }

        std::queue<std::string> keyPath;
        ParseCommandKeyPathString(tokens[1], &keyPath);
        plist::ObjectType type = ParseType(tokens[2]);
        std::string value = ParseCommandValueString(tokens.begin() + 3, tokens.end());
        if (!Set(root->get(), &keyPath, type, value, false)) {
            return false;
        }
        *mutated = true;
    } else if (command == "Clear") {
        plist::ObjectType clearType;
        if (tokens.size() < 2) {
            clearType = plist::ObjectType::Dictionary;
        } else {
            clearType = ParseType(tokens[1]);
        }
        if (!Clear(root, clearType)) {
            return false;
        }
        *mutated = true;
    } else if (command == "Delete") {
        if (tokens.size() < 2) {
            fprintf(stderr, "Delete command requires KeyPath\n");
            return false;
        }

        std::queue<std::string> keyPath;
        ParseCommandKeyPathString(tokens[1], &keyPath);
        if (!Delete(root->get(), &keyPath)) {
            return false;
        }
        *mutated = true;
    } else if (command == "Copy") {
        if (tokens.size() < 3) {
            fprintf(stderr, "Copy command requires SrcKeyPath and DstKeyPath\n");
            return false;
        }

        std::queue<std::string> srcKeyPath;
        ParseCommandKeyPathString(tokens[1], &srcKeyPath);
        std::queue<std::string> dstKeyPath;
        ParseCommandKeyPathString(tokens[2], &dstKeyPath);

        if (!Copy(root->get(), &srcKeyPath, &dstKeyPath)) {
            return false;
        }
        *mutated = true;
    } else if (command == "Merge") {
        if (tokens.size() < 2) {
            fprintf(stderr, "Merge command requires KeyPath\n");
            return false;
        }

        std::queue<std::string> keyPath;
        if (tokens.size() > 2) {
            ParseCommandKeyPathString(tokens[2], &keyPath);
        }
        if (!Merge(root->get(), tokens[1], &keyPath, filesystem)) {
            return false;
        }
        *mutated = true;
    } else if (command == "Import") {
        if (tokens.size() < 3) {
            fprintf(stderr, "Import command requires KeyPath and File\n");
            return false;
        }

        std::queue<std::string> keyPath;
        ParseCommandKeyPathString(tokens[1], &keyPath);
        if (!Import(root->get(), &keyPath, tokens[2], filesystem)) {
            return false;
        }
        *mutated = true;
    } else if (command == "Help") {
        CommandHelp();
    } else {
        fprintf(stderr, "Unrecognized command: %s\n", command.c_str());
        return false;
    }
    return true;
}

int
main(int argc, char **argv)
{
    std::vector<std::string> args = std::vector<std::string>(argv + 1, argv + argc);
    DefaultFilesystem filesystem = DefaultFilesystem();

    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, args);
    if (!result.first) {
        return Help(result.second);
    }

    if (options.help()) {
        Help();
        CommandHelp();
        return 0;
    }

    /*
     * Read input, and determine format for saving.
     */
    std::unique_ptr<plist::Object> root;
    ext::optional<plist::Format::Any> saveFormat;
    if (!options.input().empty() && !filesystem.exists(options.input())) {
        fprintf(stderr, "File does not exist, will create %s\n", options.input().c_str());
        root = plist::Dictionary::New();
    } else {
        if (!Revert(&root, &filesystem, options.input(), &saveFormat)) {
            return 1;
        }
    }

    /*
     * If format of input is not found, default to XML for save format.
     */
    if (!saveFormat) {
        plist::Format::XML xml = plist::Format::XML::Create(plist::Format::Encoding::UTF8);
        saveFormat = plist::Format::Any::Create<plist::Format::XML>(xml);
    }

    /*
     * Determine format for printing.
     */
    ext::optional<plist::Format::Any> printFormat;
    if (options.xml()) {
        plist::Format::XML xml = plist::Format::XML::Create(plist::Format::Encoding::UTF8);
        printFormat = plist::Format::Any::Create<plist::Format::XML>(xml);
    } else {
        plist::Format::ASCII ascii = plist::Format::ASCII::Create(false, plist::Format::Encoding::UTF8);
        printFormat = plist::Format::Any::Create<plist::Format::ASCII>(ascii);
    }

    bool success = true;
    if (options.command()) {
        /*
         * Process explicit command.
         */
        bool keepReading = true; /* Unused. */
        bool mutated = false;
        success &= ProcessCommand(&filesystem, options.input(), *printFormat, *saveFormat, &root, *options.command(), &mutated, &keepReading);
        if (success && mutated) {
            /* Save result only if changed. */
            std::queue<std::string> keyPath;
            Save(root.get(), *saveFormat, &filesystem, options.input());
        }
    } else {
        /*
         * Run interactive prompt.
         */
        bool keepReading = true;
        while (keepReading) {
#if defined(HAVE_LINENOISE)
            char *line = linenoise("Command: ");
            if (line != nullptr) {
                linenoiseHistoryAdd(line);
#else
            fprintf(stdout, "Command: ");
            char line[1024];
            if (fgets(line, sizeof(line), stdin) == line) {
#endif
                bool mutated = false; /* Unused. */
                success &= ProcessCommand(&filesystem, options.input(), *printFormat, *saveFormat, &root, line, &mutated, &keepReading);
            } else {
                keepReading = false;
            }
#if defined(HAVE_LINENOISE)
            linenoiseFree(static_cast<void *>(line));
#endif
        }
    }

    return (success ? 0 : 1);
}
