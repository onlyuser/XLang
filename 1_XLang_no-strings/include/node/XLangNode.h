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

#ifndef XLANG_NODE_H_
#define XLANG_NODE_H_

#include "node/XLangNodeBase.h" // Node
#include "node/XLangNodeVisitorPrinter.h"
#include "XLangType.h" // uint32
#include "XLang.tab.h" // YYLTYPE
#include <string> // std::string
#include <vector> // std::vector
#include <stdarg.h> // va_list

namespace node {

class Node : public NodeBase, public NodeVisitable
{
protected:
    NodeBase::type_e m_type;
    uint32 m_sym_id;
    YYLTYPE m_loc;

public:
    Node(NodeBase::type_e _type, uint32 _sym_id, YYLTYPE &_loc)
        : m_type(_type), m_sym_id(_sym_id), m_loc(_loc)
    {
    }
    NodeBase::type_e type() const
    {
        return m_type;
    }
    uint32 sym_id() const
    {
        return m_sym_id;
    }
    YYLTYPE loc() const
    {
        return m_loc;
    }
};

template<NodeBase::type_e _type>
class LeafNode : virtual public Node, public LeafNodeBase<_type>
{
    typename LeafValueType<_type>::type m_value;

public:
    LeafNode(uint32 sym_id, YYLTYPE &loc, typename LeafValueType<_type>::type _value)
        : Node(_type, sym_id, loc), m_value(_value)
    {
    }
    typename LeafValueType<_type>::type value() const
    {
        return m_value;
    }
    void accept(NodeVisitorBase* visitor) const
    {
        visitor->visit(this);
    }
};

class InnerNode : virtual public Node, public InnerNodeBase
{
    std::vector<Node*> m_child_vec;

public:
    InnerNode(uint32 sym_id, YYLTYPE &loc, size_t _child_count, va_list ap)
        : Node(NodeBase::INNER, sym_id, loc)
    {
        m_child_vec.resize(_child_count);
        for(size_t i = 0; i<_child_count; i++)
            m_child_vec[i] = va_arg(ap, Node*);
    }
    std::string name() const;
    Node* child(uint32 index) const
    {
        return m_child_vec[index];
    }
    size_t child_count() const
    {
        return m_child_vec.size();
    }
    void accept(NodeVisitorBase* visitor) const;
};

}

#endif
