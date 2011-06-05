// Variations of a Flex-Bison parser -- based on
// "A COMPACT GUIDE TO LEX & YACC" by Tom Niemann
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

#ifndef XLANG_NODE_VISITOR_PRINTER_H_
#define XLANG_NODE_VISITOR_PRINTER_H_

#include "node/XLangNodeBase.h" // Node
#include "node/XLangNodeVisitorBase.h"

namespace node {

class NodeVisitorPrinter : public NodeVisitorBase
{
public:
    void visit(const node::LeafNodeBase<node::NodeBase::INT>* node);
    void visit(const node::LeafNodeBase<node::NodeBase::FLOAT>* node);
    void visit(const node::LeafNodeBase<node::NodeBase::STRING>* node);
    void visit(const node::LeafNodeBase<node::NodeBase::CHAR>* node);
    void visit(const node::LeafNodeBase<node::NodeBase::IDENT>* node);
    void visit(const node::InnerNodeBase* node);
};

}

#endif
