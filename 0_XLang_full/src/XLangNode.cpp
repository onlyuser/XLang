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

#include "node/XLangNode.h"
#include <iostream>

// prototype
extern std::string sym_name(uint32 sym_id);

namespace node {

std::string InnerNode::name() const
{
    return sym_name(sym_id());
}
void InnerNode::accept(NodeVisitorBase* visitor) const
{
    std::cout << '(';
    visitor->visit(this);
    std::cout << ' ';
    size_t i;
    for(i = 0; i < child_count(); i++)
    {
        switch(child(i)->type())
        {
        case NodeBase::FLOAT:
            visitor->visit(dynamic_cast<const node::LeafNodeBase<node::NodeBase::FLOAT>*>(child(i)));
            break;
        case NodeBase::INT:
            visitor->visit(dynamic_cast<const node::LeafNodeBase<node::NodeBase::INT>*>(child(i)));
            break;
        case NodeBase::STRING:
            visitor->visit(dynamic_cast<const node::LeafNodeBase<node::NodeBase::STRING>*>(child(i)));
            break;
        case NodeBase::CHAR:
            visitor->visit(dynamic_cast<const node::LeafNodeBase<node::NodeBase::CHAR>*>(child(i)));
            break;
        case NodeBase::IDENT:
            visitor->visit(dynamic_cast<const node::LeafNodeBase<node::NodeBase::IDENT>*>(child(i)));
            break;
        case NodeBase::INNER:
            dynamic_cast<const node::NodeVisitable*>(child(i))->accept(visitor);
            break;
        }
        if(i < child_count()-1)
            std::cout << ' ';
    }
    std::cout << ')';
}

}
