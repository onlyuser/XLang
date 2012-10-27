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
#include "XLangString.h" // xl::escape
#include <iostream> // std::cout

#ifdef EXTERN_INCLUDE_PATH
    #define USE_COROUTINE
#endif
#ifdef USE_COROUTINE
    #include "coroutine/coroutine_cpp.h"
#endif

namespace xl { namespace visitor {

void DefaultTour::visit(const node::TermNodeIFace<node::NodeIdentIFace::INT>* _node)
{
    std::cout << _node->value();
}

void DefaultTour::visit(const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>* _node)
{
    std::cout << _node->value();
}

void DefaultTour::visit(const node::TermNodeIFace<node::NodeIdentIFace::STRING>* _node)
{
    std::cout << '\"' << xl::escape(*_node->value()) << '\"';
}

void DefaultTour::visit(const node::TermNodeIFace<node::NodeIdentIFace::CHAR>* _node)
{
    std::cout << '\'' << xl::escape(_node->value()) << '\'';
}

void DefaultTour::visit(const node::TermNodeIFace<node::NodeIdentIFace::IDENT>* _node)
{
    std::cout << *_node->value();
}

void DefaultTour::visit_null()
{
    std::cout << "NULL";
}

#ifdef USE_COROUTINE
static int get_next_asc(ccrContParam, const node::SymbolNodeIFace* _node)
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

int DefaultTour::get_next_child_index(const node::SymbolNodeIFace* _node)
{
#ifdef USE_COROUTINE
    return get_next_asc(&const_cast<node::SymbolNodeIFace*>(_node)->visit_state(), _node);
#else
    return -1;
#endif
}

node::NodeIdentIFace* DefaultTour::get_next_child(const node::SymbolNodeIFace* _node)
{
    int index = get_next_child_index(_node);
    if(index == -1)
        return NULL;
    return (*_node)[index];
}

bool DefaultTour::visit_next_child(const node::SymbolNodeIFace* _node, node::NodeIdentIFace** ref_node)
{
#ifdef USE_COROUTINE
    int index = get_next_child_index(_node);
    if(index == -1)
        return false;
    if(ref_node)
        *ref_node = (*_node)[index];
    visit_any((*_node)[index]);
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

void DefaultTour::abort_visitation(const node::SymbolNodeIFace* _node)
{
#ifdef USE_COROUTINE
    stop_asc(&const_cast<node::SymbolNodeIFace*>(_node)->visit_state());
#endif
}

void DefaultTour::visit(const node::SymbolNodeIFace* _node)
{
    while(visit_next_child(_node));
}

void DefaultTour::visit_any(const node::NodeIdentIFace* unknown)
{
    if(!unknown)
    {
        visit_null();
        return;
    }
    switch(unknown->type())
    {
        case node::NodeIdentIFace::INT:
            visit(dynamic_cast<const node::TermNodeIFace<node::NodeIdentIFace::INT>*>(unknown));
            break;
        case node::NodeIdentIFace::FLOAT:
            visit(dynamic_cast<const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>*>(unknown));
            break;
        case node::NodeIdentIFace::STRING:
            visit(dynamic_cast<const node::TermNodeIFace<node::NodeIdentIFace::STRING>*>(unknown));
            break;
        case node::NodeIdentIFace::CHAR:
            visit(dynamic_cast<const node::TermNodeIFace<node::NodeIdentIFace::CHAR>*>(unknown));
            break;
        case node::NodeIdentIFace::IDENT:
            visit(dynamic_cast<const node::TermNodeIFace<node::NodeIdentIFace::IDENT>*>(unknown));
            break;
        case node::NodeIdentIFace::SYMBOL:
            visit(dynamic_cast<const node::SymbolNodeIFace*>(unknown));
            break;
        default:
            std::cout << "unknown node type" << std::endl;
            break;
    }
}

void DefaultTour::begin_redirect_stdout()
{
    m_prev_stream_buf = std::cout.rdbuf(m_cout_buf.rdbuf());
}

std::string DefaultTour::end_redirect_stdout()
{
    std::string s = m_cout_buf.str();
    std::cout.rdbuf(m_prev_stream_buf);
    m_cout_buf.str(std::string());
    m_cout_buf.clear();
    return s;
}

} }
