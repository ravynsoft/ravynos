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


namespace mach_o {

class Symbol;

/*!
 * @class ExportsTrie
 *
 * @abstract
 *      Class to encapsulate accessing and building tries as used by mach-o
 */
class ExportsTrie
{
public:
                    // encapsulates exports trie in a final linked image
                    ExportsTrie(const uint8_t* start, size_t size);

                    // generic trie builder
                    struct Export { std::string_view name; uint64_t offset=0; uint64_t flags=0; uint64_t other=0; std::string_view importName; };
                    typedef Export (^Getter)(size_t index);
                    ExportsTrie(size_t exportsCount, Getter);

    bool            hasExportedSymbol(const char* symbolName, Symbol& symbol) const;
    void            forEachExportedSymbol(void (^callback)(const Symbol& symbol, bool& stop)) const;
    size_t          trieSize() const { return _trieEnd - _trieStart; }
    const void*     trieBytes() const { return (void*)_trieStart; }

private:
    void            buildTrieBytes(size_t exportsCount, Getter);

    struct Node;

    void            append_uleb128(uint64_t value);
    void            append_string(const char* str);
    void            append_string(const std::string_view& str);
    void            append_byte(uint8_t);
    void            dumpNodes(const std::vector<Node*>& allNodes);

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

        std::string_view    cummulativeString;
        std::vector<Edge>   children;
        Export              symbol;
        uint32_t            trieOffset = 0;

        void            addSymbol(const Export& sym, std::vector<Node*>& allNodes);
        bool            updateOffset(uint32_t& curOffset);
        void            appendToStream(ExportsTrie& trie);
   };

    const uint8_t*       _trieStart;
    const uint8_t*       _trieEnd;
    std::vector<uint8_t> _trieBytes;
};



} // namespace mach_o

#endif // mach_o_ExportsTrie_h


