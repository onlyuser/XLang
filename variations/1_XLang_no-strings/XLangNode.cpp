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
#include "XLangTreeContext.h" // TreeContext
#include <sstream> // std::stringstream

// prototype
extern std::string id_to_name(uint32_t sym_id);

static std::string ptr_to_string(const void* x)
{
    std::stringstream ss;
    ss << '_' << x;
    std::string s = ss.str();
    return s;
}

namespace xl { namespace node {

std::string Node::name() const
{
    return id_to_name(sym_id());
}

std::string Node::uid() const
{
    return ptr_to_string(this);
}

template<>
NodeIdentIFace* TermNode<
        static_cast<NodeIdentIFace::type_t>(TermType<std::string>::type)
        >::clone(TreeContext* tc)
{
    return new (tc->alloc(), __FILE__, __LINE__, [](void* x) {
            reinterpret_cast<NodeIdentIFace*>(x)->~NodeIdentIFace();
            }) TermNode<
                    static_cast<NodeIdentIFace::type_t>(TermType<std::string>::type)
                    >(m_sym_id, m_value);
}

SymbolNode::SymbolNode(uint32_t _sym_id, size_t _size, va_list ap)
    : Node(NodeIdentIFace::SYMBOL, _sym_id), visitor::Visitable<SymbolNode>(this),
      m_visit_state(NULL)
{
    for(size_t i = 0; i<_size; i++)
    {
        NodeIdentIFace* child = va_arg(ap, NodeIdentIFace*);
        if(!child)
            continue;
        if(is_same_type(child))
        {
            SymbolNode* symbol_node = dynamic_cast<SymbolNode*>(child);
            m_child_vec.insert(m_child_vec.end(),
                    symbol_node->m_child_vec.begin(),
                    symbol_node->m_child_vec.end());
            std::vector<NodeIdentIFace*>::iterator p;
            for(p = symbol_node->m_child_vec.begin(); p != symbol_node->m_child_vec.end(); ++p)
                (*p)->set_parent(this);
            continue;
        }
        m_child_vec.push_back(child);
        child->set_parent(this);
    }
}

NodeIdentIFace* SymbolNode::clone(TreeContext* tc)
{
    va_list ap;
    SymbolNode *_clone = new (tc->alloc(), __FILE__, __LINE__, [](void* x) {
            reinterpret_cast<NodeIdentIFace*>(x)->~NodeIdentIFace();
            }) SymbolNode(m_sym_id, 0, ap);
    std::copy(m_child_vec.begin(), m_child_vec.end(),
            std::back_inserter(_clone->m_child_vec));
    return _clone;
}

} }