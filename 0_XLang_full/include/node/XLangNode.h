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

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "node/XLangNodeVisitorIFace.h" // node::VisitableNodeIFace
#include "XLangType.h" // uint32_t
#include "XLang.tab.h" // YYLTYPE
#include <string> // std::string
#include <vector> // std::vector
#include <stdarg.h> // va_list

namespace node {

class Node : public NodeIdentIFace, public VisitableNodeIFace
{
protected:
    NodeIdentIFace::type_e m_type;
    uint32_t m_sym_id;
    YYLTYPE m_loc;

public:
    Node(NodeIdentIFace::type_e _type, uint32_t _sym_id, YYLTYPE &_loc)
        : m_type(_type), m_sym_id(_sym_id), m_loc(_loc)
    {
    }
    NodeIdentIFace::type_e type() const
    {
        return m_type;
    }
    uint32_t sym_id() const
    {
        return m_sym_id;
    }
    YYLTYPE loc() const
    {
        return m_loc;
    }
};

template<NodeIdentIFace::type_e _type>
class LeafNode : virtual public Node, public LeafNodeIFace<_type>
{
    typename LeafTypeTraits<_type>::type m_value;

public:
    LeafNode(uint32_t _sym_id, YYLTYPE &loc, typename LeafTypeTraits<_type>::type _value)
        : Node(_type, _sym_id, loc), m_value(_value)
    {
    }
    typename LeafTypeTraits<_type>::type value() const
    {
        return m_value;
    }
    void accept(NodeVisitorIFace* visitor) const
    {
        visitor->visit(this);
    }
};

class InnerNode : virtual public Node, public InnerNodeIFace
{
    typedef std::vector<NodeIdentIFace*> child_vec_t;
    child_vec_t m_child_vec;

    const child_vec_t &child_vec() const
    {
        return m_child_vec;
    }
public:
    InnerNode(uint32_t _sym_id, YYLTYPE &loc, size_t _child_count, va_list ap)
        : Node(NodeIdentIFace::INNER, _sym_id, loc)
    {
        for(size_t i = 0; i<_child_count; i++)
        {
            NodeIdentIFace* _node = va_arg(ap, NodeIdentIFace*);
            if(is_same_type(_node))
            {
                InnerNode* inner_node = dynamic_cast<InnerNode*>(_node);
                m_child_vec.insert(m_child_vec.end(),
                        inner_node->child_vec().begin(),
                        inner_node->child_vec().end());
                continue;
            }
            m_child_vec.push_back(_node);
        }
    }
    std::string name() const;
    NodeIdentIFace* child(uint32_t index) const
    {
        return m_child_vec[index];
    }
    size_t child_count() const
    {
        return m_child_vec.size();
    }
    void accept(NodeVisitorIFace* visitor) const;
};

}

#endif