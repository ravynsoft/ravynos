/*
 * Copyright (c) 2021 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef mach_o_ExportsTrie_h
#define mach_o_ExportsTrie_h

#include <stdint.h>

#include <span>
#include <string_view>
#include <vector>

#include "Error.h"


namespace mach_o {

class Symbol;


/*!
 * @class GenericTrie
 *
 * @abstract
 *      Abstract base class for searching and building tries
 */
class GenericTrie
{
protected:
					// construct from an already built trie
					GenericTrie(const uint8_t* start, size_t size) : _trieStart(start), _trieEnd(start+size) { }
					
                    struct WriterEntry {

                        std::string_view name;
                        /// Keep stride into the terminal payload buffer data instead of direct pointers,
                        /// so that the buffer can grown dynamically while generating entries on the fly.
                        struct { uint32_t offset = 0; uint32_t size = 0; } terminalStride;

                        std::span<const uint8_t> payload(const std::vector<uint8_t>& terminalBuffer) const
                        {
                            return std::span(terminalBuffer.data() + terminalStride.offset, terminalStride.size);
                        }
                    };
                    typedef WriterEntry (^Getter)(size_t index);

                    // construct a trie from an abstract list of <key,payload> entries
                    GenericTrie(size_t entriesCount, const std::vector<uint8_t>& terminalBuffer, Getter);

public:
    const uint8_t*  bytes(size_t& size);
    Error&          buildError() { return _buildError; }
protected:
    void            buildTrieBytes(size_t entryCount, const std::vector<uint8_t>& terminalBuffer, Getter);

    struct Node;

    static void     append_uleb128(uint64_t value, std::vector<uint8_t>& out);
    static void     append_string(const std::string_view& str, std::vector<uint8_t>& out);
    void            dumpNodes(const std::vector<Node*>& allNodes, const std::vector<uint8_t>& terminalBuffer);

    struct Edge
    {
                          Edge(const std::string_view& s, Node* n) : partialString(s), child(n) { }
                          ~Edge() { }

        std::string_view  partialString;
        Node*             child;
    };

    struct Node
    {
                            Node(const std::string_view& s, std::vector<Node*>& owner) : cummulativeString(s) { owner.push_back(this); }
                            ~Node() = default;

        std::string_view         cummulativeString;
        std::vector<Edge>        children;
        WriterEntry             terminalEntry;
        uint32_t                 trieOffset = 0;

        Error           addEntry(const WriterEntry& entry, const std::vector<uint8_t>& terminalBuffer, std::vector<Node*>& allNodes);
        bool            updateOffset(uint32_t& curOffset);
        void            appendToStream(GenericTrie& trie, const std::vector<uint8_t>& terminalBuffer);
   };

    const uint8_t*       _trieStart;
    const uint8_t*       _trieEnd;
    Error                _buildError;
    std::vector<uint8_t> _trieBytes;
};



/*!
 * @class ExportsTrie
 *
 * @abstract
 *      Class to encapsulate accessing and building export symbol tries
 */
class ExportsTrie : public GenericTrie
{
public:
                    // used by unit tests to build a trie
                    ExportsTrie(std::span<const Symbol> exports);

                    // generic trie builder
                    struct Export { std::string_view name; uint64_t offset=0; uint64_t flags=0; uint64_t other=0; std::string_view importName; };
                    typedef Export (^Getter)(size_t index);
                    ExportsTrie(size_t exportsCount, Getter);

private:
    WriterEntry    exportToEntry(const Export&, std::vector<uint8_t>& tempBuffer);
};

} // namespace mach_o

#endif // mach_o_ExportsTrie_h

