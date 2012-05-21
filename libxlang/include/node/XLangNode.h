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
#include "visitor/XLangVisitable.h" // visitor::Visitable
#include "visitor/XLangVisitStateIFace.h" // visitor::VisitStateIFace
#include "XLangType.h" // uint32_t
#include <string> // std::string
#include <vector> // std::vector
#include <stdarg.h> // va_list

namespace xl { namespace node {

class Node : virtual public NodeIdentIFace
{
public:
    Node(NodeIdentIFace::type_t _type, uint32_t _sym_id)
        : m_type(_type), m_sym_id(_sym_id), m_parent(NULL)
    {}
    NodeIdentIFace::type_t type() const
    {
        return m_type;
    }
    uint32_t sym_id() const
    {
        return m_sym_id;
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
    bool is_root() const
    {
        return m_parent == NULL;
    }
    std::string uid() const;

protected:
    NodeIdentIFace::type_t m_type;
    uint32_t m_sym_id;
    NodeIdentIFace* m_parent;
};

template<NodeIdentIFace::type_t _type>
class LeafNode
    : public Node, public LeafNodeIFace<_type>, public visitor::Visitable<LeafNode<_type> >
{
public:
    LeafNode(uint32_t _sym_id, typename LeafInternalType<_type>::type _value)
        : Node(_type, _sym_id), visitor::Visitable<LeafNode<_type> >(this), m_value(_value)
    {}
    typename LeafInternalType<_type>::type value() const
    {
        return m_value;
    }

private:
    typename LeafInternalType<_type>::type m_value;
};

class InnerNode
    : public Node, public InnerNodeIFace, public visitor::Visitable<InnerNode>,
      virtual public visitor::VisitStateIFace
{
public:
    InnerNode(uint32_t _sym_id, size_t _size, va_list ap)
        : Node(NodeIdentIFace::INNER, _sym_id), visitor::Visitable<InnerNode>(this),
          m_visit_state(NULL)
    {
        for(size_t i = 0; i<_size; i++)
        {
            NodeIdentIFace* child = va_arg(ap, NodeIdentIFace*);
            if(is_same_type(child))
            {
                InnerNode* inner_node = dynamic_cast<InnerNode*>(child);
                m_child_vec.insert(m_child_vec.end(),
                        inner_node->m_child_vec.begin(),
                        inner_node->m_child_vec.end());
                std::vector<NodeIdentIFace*>::iterator p;
                for(p = inner_node->m_child_vec.begin(); p != inner_node->m_child_vec.end(); ++p)
                    (*p)->set_parent(this);
                continue;
            }
            m_child_vec.push_back(child);
            child->set_parent(this);
        }
    }
    NodeIdentIFace* operator[](uint32_t index) const
    {
        return m_child_vec[index];
    }
    void add_child(NodeIdentIFace* node)
    {
        m_child_vec.push_back(node);
    }
    size_t size() const
    {
        return m_child_vec.size();
    }
    visitor::VisitStateIFace::state_t &visit_state()
    {
        return m_visit_state;
    }

private:
    std::vector<NodeIdentIFace*> m_child_vec;
    visitor::VisitStateIFace::state_t m_visit_state;
};

} }

#endif
