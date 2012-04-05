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

#include "node/XLangNode.h" // node::NodeIdentIFace
#include <iostream> // std::cout

// prototype
extern std::string sym_name(uint32_t sym_id);

namespace node {

std::string InnerNode::name() const
{
    return sym_name(sym_id());
}
void InnerNode::accept(NodeVisitorIFace* visitor) const
{
    std::cout << '(';
    visitor->visit(this);
    std::cout << ' ';
    size_t i;
    for(i = 0; i < child_count(); i++)
    {
        switch(child(i)->type())
        {
        case NodeIdentIFace::INT:
            visitor->visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::INT>*>(child(i)));
            break;
        case NodeIdentIFace::FLOAT:
            visitor->visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::FLOAT>*>(child(i)));
            break;
        case NodeIdentIFace::STRING:
            visitor->visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::STRING>*>(child(i)));
            break;
        case NodeIdentIFace::CHAR:
            visitor->visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::CHAR>*>(child(i)));
            break;
        case NodeIdentIFace::IDENT:
            visitor->visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::IDENT>*>(child(i)));
            break;
        case NodeIdentIFace::INNER:
            dynamic_cast<const node::VisitableNodeIFace*>(child(i))->accept(visitor);
            break;
        }
        if(i < child_count()-1)
            std::cout << ' ';
    }
    std::cout << ')';
}

}
