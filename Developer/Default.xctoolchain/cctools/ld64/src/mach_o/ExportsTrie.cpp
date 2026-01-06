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
// MARK: --- GenericTrie methods ---
//


const uint8_t* GenericTrie::bytes(size_t& size)
{
    size = _trieEnd - _trieStart;
    return _trieStart;
}

// generic constructor
GenericTrie::GenericTrie(size_t count, const std::vector<uint8_t>& terminalBuffer, Getter get)
{
    buildTrieBytes(count, terminalBuffer, get);
}

void GenericTrie::buildTrieBytes(size_t entriesCount, const std::vector<uint8_t>& terminalBuffer, Getter get)
{
    // build exports trie by splicing in each new symbol
    std::vector<Node*>  allNodes;
    Node* start = new Node("", allNodes);
    for ( size_t i = 0; i < entriesCount; ++i ) {
        Error err = start->addEntry(get(i), terminalBuffer, allNodes);
        if ( err.hasError() ) {
            _buildError = Error("%s", err.message());
            return;
        }
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
        assert(node->trieOffset == _trieBytes.size() && "malformed trie node, computed node offset doesn't match buffer position");
        node->appendToStream(*this, terminalBuffer);
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

Error GenericTrie::Node::addEntry(const WriterEntry& newEntry, const std::vector<uint8_t>& terminalBuffer, std::vector<Node*>& allNodes)
{
    std::string_view tail = newEntry.name.substr(cummulativeString.size());
    for ( Edge& edge : this->children ) {
        // quick check if first char of edge matches
        if ( *edge.partialString.data() != *tail.data() )
            continue;
        if ( tail.starts_with(edge.partialString) ) {
            // already have matching edge, go down that path
            return edge.child->addEntry(newEntry, terminalBuffer, allNodes);
        }
        else {
            for ( size_t len = edge.partialString.size() - 1; len > 0; --len ) {
                std::string_view edgePrefix = edge.partialString.substr(0, len);
                if ( edgePrefix == tail.substr(0, len) ) {
                    // found a common substring, splice in new node
                    // for instance had "foo", and add in "frob", common prefix is "f"
                    //  was: A--foo-->B, now: A--f-->C--oo-->B and later we add A--f-->C--rob-->D
                    Node* bNode = edge.child;
                    Node* cNode = new Node(newEntry.name.substr(0, cummulativeString.size() + len), allNodes);
                    Edge  cbEdge(edge.partialString.substr(len), bNode);
                    Edge& acEdge         = edge;
                    acEdge.partialString = edgePrefix;
                    acEdge.child         = cNode;
                    cNode->children.push_back(cbEdge);
                    return cNode->addEntry(newEntry, terminalBuffer, allNodes);
                }
            }
        }
    }
    if ( tail.empty() && (this->terminalEntry.terminalStride.size != 0) ) {
        char cstr[newEntry.name.size()+2];
        memcpy(cstr, newEntry.name.data(), newEntry.name.size());
        cstr[newEntry.name.size()] = '\0';
        return Error("duplicate symbol '%s'", (const char*)cstr); // cast is to work around va_list aliasing issue
    }

    // no commonality with any existing child, make a new edge that is this whole string
    Node* newNode  = new Node(newEntry.name, allNodes);
    newNode->terminalEntry = newEntry;
    Edge newEdge(tail, newNode);
    this->children.push_back(newEdge);

    return Error::none();
}

// byte for terminal node size in bytes, or 0x00 if not terminal node
// teminal node (uleb128 flags, uleb128 addr [uleb128 other])
// byte for child node count
//  each child: zero terminated substring, uleb128 node offset
bool GenericTrie::Node::updateOffset(uint32_t& curOffset)
{
    uint32_t nodeSize = 1; // length of node payload info when there is no payload (non-terminal)
    if ( !terminalEntry.name.empty() ) {
        // in terminal nodes, size is uleb128 encoded, so we include that in calculation
        nodeSize = (uint32_t)terminalEntry.terminalStride.size;
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

void GenericTrie::Node::appendToStream(GenericTrie& trie, const std::vector<uint8_t>& terminalBuffer)
{
    if ( !terminalEntry.name.empty() ) {
        trie.append_uleb128(terminalEntry.terminalStride.size, trie._trieBytes);
        for (uint8_t byte : terminalEntry.payload(terminalBuffer))
            trie._trieBytes.push_back(byte);
    }
    else {
        // no terminal uleb128 of zero is one byte of zero
        trie._trieBytes.push_back(0);
    }
    // write number of children
    trie._trieBytes.push_back(children.size());
    // write each child
    for ( Edge& e : children ) {
        trie.append_string(e.partialString, trie._trieBytes);
        trie.append_uleb128(e.child->trieOffset, trie._trieBytes);
    }
}

void GenericTrie::append_uleb128(uint64_t value, std::vector<uint8_t>& out)
{
    uint8_t byte;
    do {
        byte = value & 0x7F;
        value &= ~0x7F;
        if ( value != 0 )
            byte |= 0x80;
        out.push_back(byte);
        value = value >> 7;
    } while ( byte >= 0x80 );
}

void GenericTrie::append_string(const std::string_view& str, std::vector<uint8_t>& out)
{
    for ( char c : str )
        out.push_back(c);
    out.push_back('\0');
}


//
// MARK: --- ExportsTrie methods ---
//

GenericTrie::WriterEntry ExportsTrie::exportToEntry(const Export& exportInfo, std::vector<uint8_t>& temp)
{
    // encode exportInfo as uleb128s
    size_t tempStartLen = temp.size();
    if ( !exportInfo.name.empty() ) {
        if ( exportInfo.flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
            std::string_view importName = exportInfo.importName;
            // optimize case where re-export does not change name to just have a trailing empty string
            if ( importName == exportInfo.name )
                importName = "";
            // nodes with re-export info: size, flags, ordinal, string
            append_uleb128(exportInfo.flags, temp);
            append_uleb128(exportInfo.other, temp);
            append_string(importName, temp);
        }
        else if ( exportInfo.flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) {
            // nodes with stub and resolver: size, flags, address, other
            append_uleb128(exportInfo.flags, temp);
            append_uleb128(exportInfo.offset, temp);
            append_uleb128(exportInfo.other, temp);
        }
        else {
            // nodes with export info: size, flags, address
            append_uleb128(exportInfo.flags, temp);
            append_uleb128(exportInfo.offset, temp);
        }
    }
    WriterEntry entry;
    entry.name           = exportInfo.name;
    entry.terminalStride = { (uint32_t)tempStartLen, (uint32_t)(temp.size()-tempStartLen) };
    return entry;
}


// generic constructor
ExportsTrie::ExportsTrie(size_t exportsCount, Getter getter)
 : GenericTrie(nullptr, 0)
{
    __block std::vector<uint8_t> temp;
    const size_t tempSize = exportsCount*16; // estimate size buffer to try to avoid reallocation
    temp.reserve(tempSize);
    buildTrieBytes(exportsCount, temp, ^(size_t index) {
        return exportToEntry(getter(index), temp);
    });
}

} // namespace mach_o
