/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __pbxbuild_DirectedGraph_h
#define __pbxbuild_DirectedGraph_h

#include <pbxbuild/Base.h>

#include <list>
#include <ext/optional>

namespace pbxbuild {

/*
 * A generic directed graph as an external adjacency list. Generally
 * intended for topological sorting (see `ordered()`) but can also be
 * used to just pass graphs of objects around.
 *
 * Note: Specializations are realized in the implementation file.
 */
template<typename T>
class DirectedGraph {
private:
    std::unordered_set<T>                        _nodes;
    std::unordered_map<T, std::unordered_set<T>> _adjacency;

public:
    /*
     * Inserts a node into the graph along with the nodes its adjacent to.
     */
    void insert(T const &node, std::unordered_set<T> const &adjacent);

public:
    /*
     * Returns all of the nodes in the graph.
     */
    std::unordered_set<T> const &nodes() const;

    /*
     * Returns the nodes adjacent to a node. Empty if node is not
     * present in the graph or has no adjacent nodes.
     */
    std::unordered_set<T> adjacent(T const &node) const;

public:
    /*
     * Performs a toplogical sort of the graph. Fails if the graph
     * has a cycle.
     */
    ext::optional<std::vector<T>> ordered() const;
};

}

#endif // !__pbxbuild_DirectedGraph_h
