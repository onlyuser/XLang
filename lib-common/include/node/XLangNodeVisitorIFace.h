// Variations of a Flex-Bison parser
// -- based on "A COMPACT GUIDE TO LEX & YACC" by Tom Niemann
// Copyright (C) 2011 Jerry Chen <mailto:onlyuser@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef XLANG_NODE_VISITOR_IFACE_H_
#define XLANG_NODE_VISITOR_IFACE_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace

namespace node {

struct NodeVisitorIFace
{
    virtual ~NodeVisitorIFace() { }
    virtual void visit(const node::LeafNodeIFace<node::NodeIdentIFace::INT>* _node) = 0;
    virtual void visit(const node::LeafNodeIFace<node::NodeIdentIFace::FLOAT>* _node) = 0;
    virtual void visit(const node::LeafNodeIFace<node::NodeIdentIFace::STRING>* _node) = 0;
    virtual void visit(const node::LeafNodeIFace<node::NodeIdentIFace::CHAR>* _node) = 0;
    virtual void visit(const node::LeafNodeIFace<node::NodeIdentIFace::IDENT>* _node) = 0;
    virtual void visit(const node::InnerNodeIFace* _node) = 0;
};

struct VisitableNodeIFace
{
    virtual ~VisitableNodeIFace() { }
    virtual void accept(NodeVisitorIFace* visitor) const = 0;
};

}

#endif
