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

#include "XLangView.h" // mvc (owner)
#include "XLangNodeBase.h" // node::NodeBase
#include "XLangType.h" // uint32
#include <iostream> // std::cout

// prototype
const char* sym_name(uint32 sym_id);

namespace mvc {

void View::print_lisp(const node::NodeBase* _node)
{
    if(NULL == _node)
        return;
    switch(_node->type())
    {
        case node::NodeBase::FLOAT:
            std::cout << dynamic_cast<const node::LeafNodeBase<float32>*>(_node)->value();
            break;
        case node::NodeBase::STRING:
            std::cout << '\"' << dynamic_cast<const node::LeafNodeBase<std::string>*>(_node)->value() << '\"';
            break;
        case node::NodeBase::CHAR:
            std::cout << '\'' << dynamic_cast<const node::LeafNodeBase<char>*>(_node)->value() << '\'';
            break;
        case node::NodeBase::IDENT:
            std::cout << *dynamic_cast<const node::LeafNodeBase<const std::string*>*>(_node)->value();
            break;
        case node::NodeBase::INNER:
            {
                std::cout << '(' << sym_name(_node->sym_id()) << ' ';
                size_t i;
                for(i = 0; i < dynamic_cast<const node::InnerNodeBase*>(_node)->child_count()-1; i++)
                {
                    print_lisp(dynamic_cast<const node::InnerNodeBase*>(_node)->child(i));
                    std::cout << ' ';
                }
                print_lisp(dynamic_cast<const node::InnerNodeBase*>(_node)->child(i));
                std::cout << ')';
            }
    }
}

}
