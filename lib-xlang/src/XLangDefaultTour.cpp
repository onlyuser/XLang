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

#include "visitor/XLangDefaultTour.h" // visitor::DefaultTour
#include <iostream> // std::cout

#ifdef EXTERN_INCLUDE_PATH
    #define USE_COROUTINE
#endif
#ifdef USE_COROUTINE
    #include "coroutine/coroutine_cpp.h"
#endif

namespace xl { namespace visitor {

void DefaultTour::visit(const node::LeafNodeIFace<node::NodeIdentIFace::INT>* _node)
{
    std::cout << _node->value();
}

void DefaultTour::visit(const node::LeafNodeIFace<node::NodeIdentIFace::FLOAT>* _node)
{
    std::cout << _node->value();
}

void DefaultTour::visit(const node::LeafNodeIFace<node::NodeIdentIFace::STRING>* _node)
{
    std::cout << '\"' << _node->value() << '\"';
}

void DefaultTour::visit(const node::LeafNodeIFace<node::NodeIdentIFace::CHAR>* _node)
{
    std::cout << '\'' << _node->value() << '\'';
}

void DefaultTour::visit(const node::LeafNodeIFace<node::NodeIdentIFace::IDENT>* _node)
{
    std::cout << *_node->value();
}

#ifdef USE_COROUTINE
static int get_next_asc(ccrContParam, const node::InnerNodeIFace* _node)
{
    ccrBeginContext;
    int i;
    ccrEndContext(foo);
    ccrBegin(foo);
    for(foo->i = 0; foo->i < static_cast<int>(_node->size()); foo->i++)
        ccrReturn(foo->i);
    ccrFinish(-1);
}

static void stop_asc(ccrContParam)
{
    ccrStopV;
}
#endif

bool DefaultTour::visit_next_child(const node::InnerNodeIFace* _node)
{
#ifdef USE_COROUTINE
    int index = get_next_asc(&const_cast<node::InnerNodeIFace*>(_node)->visit_state(), _node);
    if(index == -1)
        return false;
    visit_any(_node->operator[](index));
    if(index == static_cast<int>(_node->size())-1)
    {
        abort_visitation(_node);
        return false;
    }
    return true;
#else
    return false;
#endif
}

void DefaultTour::abort_visitation(const node::InnerNodeIFace* _node)
{
#ifdef USE_COROUTINE
    stop_asc(&const_cast<node::InnerNodeIFace*>(_node)->visit_state());
#endif
}

void DefaultTour::visit(const node::InnerNodeIFace* _node)
{
    while(visit_next_child(_node));
}

void DefaultTour::visit_any(const node::NodeIdentIFace* unknown)
{
    switch(unknown->type())
    {
        case node::NodeIdentIFace::INT:
            visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::INT>*>(unknown));
            break;
        case node::NodeIdentIFace::FLOAT:
            visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::FLOAT>*>(unknown));
            break;
        case node::NodeIdentIFace::STRING:
            visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::STRING>*>(unknown));
            break;
        case node::NodeIdentIFace::CHAR:
            visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::CHAR>*>(unknown));
            break;
        case node::NodeIdentIFace::IDENT:
            visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::IDENT>*>(unknown));
            break;
        case node::NodeIdentIFace::INNER:
            visit(dynamic_cast<const node::InnerNodeIFace*>(unknown));
            break;
        default:
            std::cout << "unknown node type" << std::endl;
            break;
    }
}

} }
