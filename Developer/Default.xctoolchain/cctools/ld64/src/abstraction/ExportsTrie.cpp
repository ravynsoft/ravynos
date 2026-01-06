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

#include <sys/types.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach-o/loader.h>

#include "ExportsTrie.h"

namespace mach_o {

static uint32_t uleb128_size(uint64_t value)
{
    uint32_t result = 0;
    do {
        value = value >> 7;
        ++result;
    } while ( value != 0 );
    return result;
}

//
// MARK: --- ExportsTrie methods ---
//

ExportsTrie::ExportsTrie(const uint8_t* start, size_t size)
    : _trieStart(start)
    , _trieEnd(start + size)
{
}

// generic constructor
ExportsTrie::ExportsTrie(size_t exportsCount, Getter get)
{
    buildTrieBytes(exportsCount, get);
}

void ExportsTrie::dumpNodes(const std::vector<Node*>& allNodes)
{
    for ( const Node* node : allNodes ) {
        fprintf(stderr, "Node: %p '%.*s'\n", node, (int)node->cummulativeString.size(), node->cummulativeString.data());
        for ( const Edge& edge : node->children ) {
            fprintf(stderr, "  edge: '%.*s' -> %p\n", (int)(edge.partialString.size()), edge.partialString.data(), edge.child);
        }
    }
}

void ExportsTrie::buildTrieBytes(size_t exportsCount, Getter get)
{
    // build exports trie by splicing in each new symbol
    std::vector<Node*>  allNodes;
    Node* start = new Node("", allNodes);
    for ( size_t i = 0; i < exportsCount; ++i ) {
        start->addSymbol(get(i), allNodes);
    }

    // assign each node in the vector an offset in the trie stream, iterating until all uleb128 sizes have stabilized
    bool more;
    do {
        uint32_t curOffset = 0;
        more               = false;
        for ( Node* node : allNodes ) {
            if ( node->updateOffset(curOffset) )
                more = true;
        }
    } while ( more );

    // create trie stream
    for ( Node* node : allNodes ) {
        node->appendToStream(*this);
    }
    // pad to be 8-btye aligned
    while ( (_trieBytes.size() % 8) != 0 )
        _trieBytes.push_back(0);

    // delete nodes used during building
    for ( Node* node : allNodes ) {
        delete node;
    }

    // set up trie buffer
    _trieStart = &_trieBytes[0];
    _trieEnd   = &_trieBytes[_trieBytes.size()];
}

void ExportsTrie::Node::addSymbol(const Export& exp, std::vector<Node*>& allNodes)
{
    std::string_view tail = exp.name.substr(cummulativeString.size());
    for ( Edge& edge : this->children ) {
        // quick check if first char of edge matches
        if ( *edge.partialString.data() != *tail.data() )
            continue;
        if ( tail.starts_with(edge.partialString) ) {
            // already have matching edge, go down that path
            edge.child->addSymbol(exp, allNodes);
            return;
        }
        else {
            for ( size_t len = edge.partialString.size() - 1; len > 0; --len ) {
                std::string_view edgePrefix = edge.partialString.substr(0, len);
                if ( edgePrefix == tail.substr(0, len) ) {
                    // found a common substring, splice in new node
                    // for instance had "foo", and add in "frob", common prefix is "f"
                    //  was: A--foo-->B, now: A--f-->C--oo-->B and later we add A--f-->C--rob-->D
                    Node* bNode = edge.child;
                    Node* cNode = new Node(exp.name.substr(0, cummulativeString.size() + len), allNodes);
                    Edge  cbEdge(edge.partialString.substr(len), bNode);
                    Edge& acEdge         = edge;
                    acEdge.partialString = edgePrefix;
                    acEdge.child         = cNode;
                    cNode->children.push_back(cbEdge);
                    cNode->addSymbol(exp, allNodes);
                    return;
                }
            }
        }
    }
    if ( exp.flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
        assert(!exp.importName.empty());
        assert(exp.other != 0);
    }
    if ( exp.flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) {
        assert(exp.other != 0);
    }
    // no commonality with any existing child, make a new edge that is this whole string
    Node* newNode   = new Node(exp.name, allNodes);
    newNode->symbol = exp;
    Edge newEdge(tail, newNode);
    this->children.push_back(newEdge);
}

// byte for terminal node size in bytes, or 0x00 if not terminal node
// teminal node (uleb128 flags, uleb128 addr [uleb128 other])
// byte for child node count
//  each child: zero terminated substring, uleb128 node offset
bool ExportsTrie::Node::updateOffset(uint32_t& curOffset)
{
    uint32_t nodeSize = 1; // length of export info when no export info
    if ( !symbol.name.empty() ) {
        if ( symbol.flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
            nodeSize = uleb128_size(symbol.flags) + uleb128_size(symbol.other); // ordinal
            if ( !symbol.importName.empty() )
                nodeSize += symbol.importName.size();
            ++nodeSize; // trailing zero in imported name
        }
        else {
            nodeSize = uleb128_size(symbol.flags) + uleb128_size(symbol.offset);
            if ( symbol.flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
                nodeSize += uleb128_size(symbol.other);
        }
        // do have export info, overall node size so far is uleb128 of export info + export info
        nodeSize += uleb128_size(nodeSize);
    }
    // add children
    ++nodeSize; // byte for count of chidren
    for ( Edge& edge : this->children ) {
        nodeSize += edge.partialString.size() + 1 + uleb128_size(edge.child->trieOffset);
    }
    bool result = (trieOffset != curOffset);
    trieOffset  = curOffset;
    // fprintf(stderr, "updateOffset %p %05d %s\n", this, trieOffset, fCummulativeString);
    curOffset += nodeSize;
    // return true if trieOffset was changed
    return result;
}

void ExportsTrie::Node::appendToStream(ExportsTrie& trie)
{
    if ( !symbol.name.empty() ) {
        if ( symbol.flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
            if ( !symbol.importName.empty() ) {
                // nodes with re-export info: size, flags, ordinal, string
                size_t nodeSize = uleb128_size(symbol.flags) + uleb128_size(symbol.other) + symbol.importName.size() + 1;
                trie.append_byte(nodeSize);
                trie.append_uleb128(symbol.flags);
                trie.append_uleb128(symbol.other);
                trie.append_string(symbol.importName);
            }
            else {
                // nodes with re-export info: size, flags, ordinal, empty-string
                uint32_t nodeSize = uleb128_size(symbol.flags) + uleb128_size(symbol.other) + 1;
                trie.append_byte(nodeSize);
                trie.append_uleb128(symbol.flags);
                trie.append_uleb128(symbol.other);
                trie.append_byte(0);
            }
        }
        else if ( symbol.flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) {
            // nodes with export info: size, flags, address, other
            uint32_t nodeSize = uleb128_size(symbol.flags) + uleb128_size(symbol.offset) + uleb128_size(symbol.other);
            trie.append_byte(nodeSize);
            trie.append_uleb128(symbol.flags);
            trie.append_uleb128(symbol.offset);
            trie.append_uleb128(symbol.other);
        }
        else {
            // nodes with export info: size, flags, address
            uint32_t nodeSize = uleb128_size(symbol.flags) + uleb128_size(symbol.offset);
            trie.append_byte(nodeSize);
            trie.append_uleb128(symbol.flags);
            trie.append_uleb128(symbol.offset);
        }
    }
    else {
        // no export info uleb128 of zero is one byte of zero
        trie.append_byte(0);
    }
    // write number of children
    trie.append_byte(children.size());
    // write each child
    for ( Edge& e : children ) {
        trie.append_string(e.partialString);
        trie.append_uleb128(e.child->trieOffset);
    }
}

void ExportsTrie::append_uleb128(uint64_t value)
{
    uint8_t byte;
    do {
        byte = value & 0x7F;
        value &= ~0x7F;
        if ( value != 0 )
            byte |= 0x80;
        _trieBytes.push_back(byte);
        value = value >> 7;
    } while ( byte >= 0x80 );
}

void ExportsTrie::append_string(const std::string_view& str)
{
    for ( char c : str )
        _trieBytes.push_back(c);
    _trieBytes.push_back('\0');
}

void ExportsTrie::append_string(const char* str)
{
    for ( const char* s = str; *s != '\0'; ++s )
        _trieBytes.push_back(*s);
    _trieBytes.push_back('\0');
}

void ExportsTrie::append_byte(uint8_t value)
{
    _trieBytes.push_back(value);
}


} // namespace mach_o
