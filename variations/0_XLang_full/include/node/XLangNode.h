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

#ifndef XLANG_NODE_H_
#define XLANG_NODE_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "visitor/XLangVisitable.h" // visitor::Visitable
#include "visitor/XLangVisitStateIFace.h" // visitor::VisitStateIFace
#include "XLangTreeContext.h" // TreeContext
#include "XLangType.h" // uint32_t
#include "XLang.tab.h" // YYLTYPE
#include <string> // std::string
#include <vector> // std::vector
#include <stdarg.h> // va_list

namespace xl { namespace node {

class Node : virtual public NodeIdentIFace
{
public:
    Node(NodeIdentIFace::type_t _type, uint32_t _lexer_id, YYLTYPE _loc)
        : m_type(_type), m_lexer_id(_lexer_id), m_parent(NULL), m_loc(_loc)
    {}

    // required
    NodeIdentIFace::type_t type() const
    {
        return m_type;
    }
    uint32_t lexer_id() const
    {
        return m_lexer_id;
    }
    std::string name() const;
    void set_parent(NodeIdentIFace* parent)
    {
        m_parent = parent;
    }
    NodeIdentIFace* parent() const
    {
        return m_parent;
    }
    std::string uid() const;

    // built-in
    YYLTYPE loc() const
    {
        return m_loc;
    }

protected:
    NodeIdentIFace::type_t m_type;
    uint32_t m_lexer_id;
    NodeIdentIFace* m_parent;
    YYLTYPE m_loc;
};

template<NodeIdentIFace::type_t _type>
class TermNode
    : public Node, public TermNodeIFace<_type>, public visitor::Visitable<TermNode<_type>>
{
public:
    TermNode(uint32_t _lexer_id, YYLTYPE loc, typename TermInternalType<_type>::type _value)
        : Node(_type, _lexer_id, loc), visitor::Visitable<TermNode<_type>>(this), m_value(_value)
    {
    }
    typename TermInternalType<_type>::type value() const
    {
        return m_value;
    }

private:
    typename TermInternalType<_type>::type m_value;
};

class SymbolNode
    : public Node, public SymbolNodeIFace, public visitor::Visitable<SymbolNode>,
      virtual public visitor::VisitStateIFace
{
public:
    SymbolNode(uint32_t _lexer_id, YYLTYPE loc, size_t _size, va_list ap);

    // required
    NodeIdentIFace* operator[](uint32_t index) const
    {
        return m_child_vec[index];
    }
    size_t size() const
    {
        return m_child_vec.size();
    }

    // optional
    void push_back(NodeIdentIFace* _node);

    // built-in
    visitor::VisitStateIFace::state_t &visit_state()
    {
        return m_visit_state;
    }
    static NodeIdentIFace* eol()
    {
        static int dummy;
        return reinterpret_cast<NodeIdentIFace*>(&dummy);
    }

private:
    std::vector<NodeIdentIFace*> m_child_vec;
    visitor::VisitStateIFace::state_t m_visit_state;
};

} }

#endif
