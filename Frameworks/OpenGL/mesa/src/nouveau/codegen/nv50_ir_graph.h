/*
 * Copyright 2011 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __NV50_IR_GRAPH_H__
#define __NV50_IR_GRAPH_H__

#include "nv50_ir_util.h"
#include <vector>

namespace nv50_ir {

#define ITER_NODE(x) reinterpret_cast<Graph::Node *>((x).get())
#define ITER_EDGE(x) reinterpret_cast<Graph::Edge *>((x).get())

// A connected graph.
class Graph
{
public:
   class Node;

   class Edge
   {
   public:
      enum Type
      {
         UNKNOWN,
         TREE,
         FORWARD,
         BACK,
         CROSS, // e.g. loop break
      };

      Edge(Node *dst, Node *src, Type kind);
      ~Edge() { unlink(); }

      inline Node *getOrigin() const { return origin; }
      inline Node *getTarget() const { return target; }

      inline Type getType() const { return type; }
      const char *typeStr() const;

   private:
      Node *origin;
      Node *target;

      Type type;
      Edge *next[2]; // next edge outgoing/incident from/to origin/target
      Edge *prev[2];

      void unlink();

      friend class Graph;
   };

   class EdgeIterator : public Iterator
   {
   public:
      EdgeIterator() : e(0), t(0), d(0), rev(false) { }
      EdgeIterator(Graph::Edge *first, int dir, bool reverse)
         : d(dir), rev(reverse)
      {
         t = e = ((rev && first) ? first->prev[d] : first);
      }

      virtual void next()
      {
         Graph::Edge *n = (rev ? e->prev[d] : e->next[d]);
         e = (n == t ? NULL : n);
      }
      virtual bool end() const { return !e; }
      virtual void *get() const { return e; }

      inline Node *getNode() const { assert(e); return d ?
                                                   e->origin : e->target; }
      inline Edge *getEdge() const { return e; }
      inline Edge::Type getType() { return e ? e->getType() : Edge::UNKNOWN; }

   private:
      Graph::Edge *e;
      Graph::Edge *t;
      int d;
      bool rev;
   };

   class Node
   {
   public:
      Node(void *);
      ~Node() { cut(); }

      void attach(Node *, Edge::Type);
      bool detach(Node *);
      void cut();

      inline EdgeIterator outgoing(bool reverse = false) const;
      inline EdgeIterator incident(bool reverse = false) const;

      inline Node *parent() const; // returns NULL if count(incident edges) != 1

      bool reachableBy(const Node *node, const Node *term) const;

      inline bool visit(int);
      inline int  getSequence() const;

      inline int incidentCountFwd() const; // count of incident non-back edges
      inline int incidentCount() const { return inCount; }
      inline int outgoingCount() const { return outCount; }

      Graph *getGraph() const { return graph; }

      void *data;

   private:
      Edge *in;
      Edge *out;
      Graph *graph;

      int visited;

      int16_t inCount;
      int16_t outCount;
   public:
      int tag; // for temporary use

      friend class Graph;
   };

public:
   Graph();
   virtual ~Graph(); // does *not* free the nodes (make it an option ?)

   inline Node *getRoot() const { return root; }

   inline unsigned int getSize() const { return size; }

   inline int nextSequence();

   void insert(Node *node); // attach to or set as root

   IteratorRef iteratorDFS(bool preorder = true);
   IteratorRef iteratorCFG();

   // safe iterators are unaffected by changes to the *edges* of the graph
   IteratorRef safeIteratorDFS(bool preorder = true);
   IteratorRef safeIteratorCFG();

   void classifyEdges();

   // @weights: indexed by Node::tag
   int findLightestPathWeight(Node *, Node *, const std::vector<int>& weights);

private:
   void classifyDFS(Node *, int&);

private:
   Node *root;
   unsigned int size;
   int sequence;
};

int Graph::nextSequence()
{
   return ++sequence;
}

Graph::Node *Graph::Node::parent() const
{
   if (inCount != 1)
      return NULL;
   assert(in);
   return in->origin;
}

bool Graph::Node::visit(int v)
{
   if (visited == v)
      return false;
   visited = v;
   return true;
}

int Graph::Node::getSequence() const
{
   return visited;
}

Graph::EdgeIterator Graph::Node::outgoing(bool reverse) const
{
   return EdgeIterator(out, 0, reverse);
}

Graph::EdgeIterator Graph::Node::incident(bool reverse) const
{
   return EdgeIterator(in, 1, reverse);
}

int Graph::Node::incidentCountFwd() const
{
   int n = 0;
   for (EdgeIterator ei = incident(); !ei.end(); ei.next())
      if (ei.getType() != Edge::BACK)
         ++n;
   return n;
}

} // namespace nv50_ir

#endif // __NV50_IR_GRAPH_H__
