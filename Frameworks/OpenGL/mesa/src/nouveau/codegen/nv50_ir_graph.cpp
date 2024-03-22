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

#include "nv50_ir_graph.h"
#include <limits>
#include <list>
#include <stack>
#include "nv50_ir.h"

namespace nv50_ir {

Graph::Graph()
{
   root = NULL;
   size = 0;
   sequence = 0;
}

Graph::~Graph()
{
   for (IteratorRef it = safeIteratorDFS(); !it->end(); it->next())
      reinterpret_cast<Node *>(it->get())->cut();
}

void Graph::insert(Node *node)
{
   if (!root)
      root = node;

   node->graph = this;
   size++;
}

void Graph::Edge::unlink()
{
   if (origin) {
      prev[0]->next[0] = next[0];
      next[0]->prev[0] = prev[0];
      if (origin->out == this)
         origin->out = (next[0] == this) ? NULL : next[0];

      --origin->outCount;
   }
   if (target) {
      prev[1]->next[1] = next[1];
      next[1]->prev[1] = prev[1];
      if (target->in == this)
         target->in = (next[1] == this) ? NULL : next[1];

      --target->inCount;
   }
}

const char *Graph::Edge::typeStr() const
{
   switch (type) {
   case TREE:    return "tree";
   case FORWARD: return "forward";
   case BACK:    return "back";
   case CROSS:   return "cross";
   case UNKNOWN:
   default:
      return "unk";
   }
}

Graph::Node::Node(void *priv) : data(priv),
                                in(0), out(0), graph(0),
                                visited(0),
                                inCount(0), outCount(0),
                                tag(0)
{
   // nothing to do
}

void Graph::Node::attach(Node *node, Edge::Type kind)
{
   Edge *edge = new Edge(this, node, kind);

   // insert head
   if (this->out) {
      edge->next[0] = this->out;
      edge->prev[0] = this->out->prev[0];
      edge->prev[0]->next[0] = edge;
      this->out->prev[0] = edge;
   }
   this->out = edge;

   if (node->in) {
      edge->next[1] = node->in;
      edge->prev[1] = node->in->prev[1];
      edge->prev[1]->next[1] = edge;
      node->in->prev[1] = edge;
   }
   node->in = edge;

   ++this->outCount;
   ++node->inCount;

   assert(graph || node->graph);
   if (!node->graph)
      graph->insert(node);
   if (!graph)
      node->graph->insert(this);

   if (kind == Edge::UNKNOWN)
      graph->classifyEdges();
}

bool Graph::Node::detach(Graph::Node *node)
{
   EdgeIterator ei = this->outgoing();
   for (; !ei.end(); ei.next())
      if (ei.getNode() == node)
         break;
   if (ei.end()) {
      ERROR("no such node attached\n");
      return false;
   }
   delete ei.getEdge();
   return true;
}

// Cut a node from the graph, deleting all attached edges.
void Graph::Node::cut()
{
   while (out)
      delete out;
   while (in)
      delete in;

   if (graph) {
      if (graph->root == this)
         graph->root = NULL;
      graph = NULL;
   }
}

Graph::Edge::Edge(Node *org, Node *tgt, Type kind)
{
   target = tgt;
   origin = org;
   type = kind;

   next[0] = next[1] = this;
   prev[0] = prev[1] = this;
}

bool
Graph::Node::reachableBy(const Node *node, const Node *term) const
{
   std::stack<const Node *> stack;
   const Node *pos = NULL;
   const int seq = graph->nextSequence();

   stack.push(node);

   while (!stack.empty()) {
      pos = stack.top();
      stack.pop();

      if (pos == this)
         return true;
      if (pos == term)
         continue;

      for (EdgeIterator ei = pos->outgoing(); !ei.end(); ei.next()) {
         if (ei.getType() == Edge::BACK)
            continue;
         if (ei.getNode()->visit(seq))
            stack.push(ei.getNode());
      }
   }
   return pos == this;
}

class DFSIterator : public Iterator
{
public:
   DFSIterator(Graph *graph, const bool preorder)
   {
      unsigned int seq = graph->nextSequence();

      nodes = new Graph::Node * [graph->getSize() + 1];
      count = 0;
      pos = 0;
      nodes[graph->getSize()] = 0;

      if (graph->getRoot()) {
         graph->getRoot()->visit(seq);
         search(graph->getRoot(), preorder, seq);
      }
   }

   ~DFSIterator()
   {
      if (nodes)
         delete[] nodes;
   }

   void search(Graph::Node *node, const bool preorder, const int sequence)
   {
      if (preorder)
         nodes[count++] = node;

      for (Graph::EdgeIterator ei = node->outgoing(); !ei.end(); ei.next())
         if (ei.getNode()->visit(sequence))
            search(ei.getNode(), preorder, sequence);

      if (!preorder)
         nodes[count++] = node;
   }

   virtual bool end() const { return pos >= count; }
   virtual void next() { if (pos < count) ++pos; }
   virtual void *get() const { return nodes[pos]; }
   virtual void reset() { pos = 0; }

protected:
   Graph::Node **nodes;
   int count;
   int pos;
};

IteratorRef Graph::iteratorDFS(bool preorder)
{
   return IteratorRef(new DFSIterator(this, preorder));
}

IteratorRef Graph::safeIteratorDFS(bool preorder)
{
   return this->iteratorDFS(preorder);
}

class CFGIterator : public Iterator
{
public:
   CFGIterator(Graph *graph)
   {
      nodes = new Graph::Node * [graph->getSize() + 1];
      count = 0;
      pos = 0;
      nodes[graph->getSize()] = 0;

      // TODO: argh, use graph->sequence instead of tag and just raise it by > 1
      for (IteratorRef it = graph->iteratorDFS(); !it->end(); it->next())
         reinterpret_cast<Graph::Node *>(it->get())->tag = 0;

      if (graph->getRoot())
         search(graph->getRoot(), graph->nextSequence());
   }

   ~CFGIterator()
   {
      if (nodes)
         delete[] nodes;
   }

   virtual void *get() const { return nodes[pos]; }
   virtual bool end() const { return pos >= count; }
   virtual void next() { if (pos < count) ++pos; }
   virtual void reset() { pos = 0; }

private:
   void search(Graph::Node *node, const int sequence)
   {
      Stack bb, cross;

      bb.push(node);

      while (bb.getSize() || cross.getSize()) {
         if (bb.getSize() == 0)
            cross.moveTo(bb);

         node = reinterpret_cast<Graph::Node *>(bb.pop().u.p);
         assert(node);
         if (!node->visit(sequence))
            continue;
         node->tag = 0;

         for (Graph::EdgeIterator ei = node->outgoing(); !ei.end(); ei.next()) {
            switch (ei.getType()) {
            case Graph::Edge::TREE:
            case Graph::Edge::FORWARD:
               if (++(ei.getNode()->tag) == ei.getNode()->incidentCountFwd())
                  bb.push(ei.getNode());
               break;
            case Graph::Edge::BACK:
               continue;
            case Graph::Edge::CROSS:
               if (++(ei.getNode()->tag) == 1)
                  cross.push(ei.getNode());
               break;
            default:
               assert(!"unknown edge kind in CFG");
               break;
            }
         }
         nodes[count++] = node;
      }
   }

private:
   Graph::Node **nodes;
   int count;
   int pos;
};

IteratorRef Graph::iteratorCFG()
{
   return IteratorRef(new CFGIterator(this));
}

IteratorRef Graph::safeIteratorCFG()
{
   return this->iteratorCFG();
}

/**
 * Edge classification:
 *
 * We have a graph and want to classify the edges into one of four types:
 * - TREE:    edges that belong to a spanning tree of the graph
 * - FORWARD: edges from a node to a descendent in the spanning tree
 * - BACK:    edges from a node to a parent (or itself) in the spanning tree
 * - CROSS:   all other edges (because they cross between branches in the
 *            spanning tree)
 */
void Graph::classifyEdges()
{
   int seq;

   for (IteratorRef it = iteratorDFS(true); !it->end(); it->next()) {
      Node *node = reinterpret_cast<Node *>(it->get());
      node->visit(0);
      node->tag = 0;
   }

   classifyDFS(root, (seq = 0));

   sequence = seq;
}

void Graph::classifyDFS(Node *curr, int& seq)
{
   Graph::Edge *edge;
   Graph::Node *node;

   curr->visit(++seq);
   curr->tag = 1;

   for (edge = curr->out; edge; edge = edge->next[0]) {
      node = edge->target;

      if (node->getSequence() == 0) {
         edge->type = Edge::TREE;
         classifyDFS(node, seq);
      } else
      if (node->getSequence() > curr->getSequence()) {
         edge->type = Edge::FORWARD;
      } else {
         edge->type = node->tag ? Edge::BACK : Edge::CROSS;
      }
   }

   for (edge = curr->in; edge; edge = edge->next[1]) {
      node = edge->origin;

      if (node->getSequence() == 0) {
         edge->type = Edge::TREE;
         classifyDFS(node, seq);
      } else
      if (node->getSequence() > curr->getSequence()) {
         edge->type = Edge::FORWARD;
      } else {
         edge->type = node->tag ? Edge::BACK : Edge::CROSS;
      }
   }

   curr->tag = 0;
}

// @dist is indexed by Node::tag, returns -1 if no path found
int
Graph::findLightestPathWeight(Node *a, Node *b, const std::vector<int> &weight)
{
   std::vector<int> path(weight.size(), std::numeric_limits<int>::max());
   std::list<Node *> nodeList;
   const int seq = nextSequence();

   path[a->tag] = 0;
   for (Node *c = a; c && c != b;) {
      const int p = path[c->tag] + weight[c->tag];
      for (EdgeIterator ei = c->outgoing(); !ei.end(); ei.next()) {
         Node *t = ei.getNode();
         if (t->getSequence() < seq) {
            if (path[t->tag] == std::numeric_limits<int>::max())
               nodeList.push_front(t);
            if (p < path[t->tag])
               path[t->tag] = p;
         }
      }
      c->visit(seq);
      Node *next = NULL;
      for (std::list<Node *>::iterator n = nodeList.begin();
           n != nodeList.end(); ++n) {
         if (!next || path[(*n)->tag] < path[next->tag])
            next = *n;
         if ((*n) == c) {
            // erase visited
            n = nodeList.erase(n);
            --n;
         }
      }
      c = next;
   }
   if (path[b->tag] == std::numeric_limits<int>::max())
      return -1;
   return path[b->tag];
}

} // namespace nv50_ir
