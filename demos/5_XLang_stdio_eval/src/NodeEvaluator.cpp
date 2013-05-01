// XLang
// -- A parser framework for language modeling
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

#include "NodeEvaluator.h" // NodeEvaluator
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "XLangString.h" // xl::escape
#include <iostream> // std::cout

void NodeEvaluator::visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::INT>* _node)
{
    value = _node->value();
}

void NodeEvaluator::visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::FLOAT>* _node)
{
    value = _node->value();
}

void NodeEvaluator::visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>* _node)
{
    value = 0;
}

void NodeEvaluator::visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::CHAR>* _node)
{
    value = 0;
}

void NodeEvaluator::visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::IDENT>* _node)
{
    value = 0;
}

void NodeEvaluator::visit(const xl::node::SymbolNodeIFace* _node)
{
    if(_node->lexer_id() == ID_UMINUS)
    {
        visit_next_child(_node);
        value = -value;
        return;
    }
    float32_t _value = 0;
    bool more = visit_next_child(_node);
    _value = value;
    while(more)
    {
        more = visit_next_child(_node);
        switch(_node->lexer_id())
        {
            case '+': _value += value; break;
            case '-': _value -= value; break;
            case '*': _value *= value; break;
            case '/': _value /= value; break;
        }
    }
    value = _value;
    if(_node->is_root())
        std::cout << _value << std::endl;
}
