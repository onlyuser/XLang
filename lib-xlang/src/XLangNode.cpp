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
#include <algorithm> // std::remove_if

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
NodeIdentIFace* TermNode<NodeIdentIFace::STRING>::clone(TreeContext* tc) const
{
    TermNodeIFace<NodeIdentIFace::STRING> *_clone = new (PNEW(tc->alloc(), , NodeIdentIFace))
            TermNode<NodeIdentIFace::STRING>(m_sym_id, m_value);
    _clone->set_original(original() ? original() : this);
    return _clone;
}

SymbolNode::SymbolNode(uint32_t _sym_id, size_t _size, va_list ap)
    : Node(NodeIdentIFace::SYMBOL, _sym_id), visitor::Visitable<SymbolNode>(this),
      m_visit_state(NULL)
{
    for(size_t i = 0; i<_size; i++)
    {
        NodeIdentIFace* child = va_arg(ap, NodeIdentIFace*);
        if(child == SymbolNode::eol())
            continue;
        if(child && is_same_type(child))
        {
            SymbolNode* symbol_node = dynamic_cast<SymbolNode*>(child);
            m_child_vec.insert(m_child_vec.end(),
                    symbol_node->m_child_vec.begin(),
                    symbol_node->m_child_vec.end());
            for(auto p = symbol_node->m_child_vec.begin(); p != symbol_node->m_child_vec.end(); ++p)
            {
                if(*p)
                    (*p)->set_parent(this);
            }
            continue;
        }
        m_child_vec.push_back(child);
        if(child)
            child->set_parent(this);
    }
}

void SymbolNode::remove(NodeIdentIFace* node)
{
    m_child_vec.erase(std::remove(m_child_vec.begin(), m_child_vec.end(), node), m_child_vec.end());
}

void SymbolNode::replace(NodeIdentIFace* find_node, NodeIdentIFace* replace_node)
{
    std::replace(m_child_vec.begin(), m_child_vec.end(), find_node, replace_node);
}

NodeIdentIFace* SymbolNode::find_clone_of_original(const NodeIdentIFace* original) const
{
    auto p = std::find_if(m_child_vec.begin(), m_child_vec.end(), [&original](const NodeIdentIFace* _node) {
            return _node->original() == original;
            });
    if(p == m_child_vec.end())
        return NULL;
    int index = std::distance(m_child_vec.begin(), p);
    return m_child_vec[index];
}

NodeIdentIFace* SymbolNode::find_if(bool (*pred)(const NodeIdentIFace* _node)) const
{
    auto p = std::find_if(m_child_vec.begin(), m_child_vec.end(), pred);
    if(p == m_child_vec.end())
        return NULL;
    int index = std::distance(m_child_vec.begin(), p);
    return m_child_vec[index];
}

NodeIdentIFace* SymbolNode::clone(TreeContext* tc) const
{
    va_list ap;
    SymbolNodeIFace *_clone = new (PNEW(tc->alloc(), , NodeIdentIFace))
            SymbolNode(m_sym_id, 0, ap);
    _clone->set_original(original() ? original() : this);
    for(auto p = m_child_vec.begin(); p != m_child_vec.end(); ++p)
    {
        NodeIdentIFace *child_clone = (*p) ? (*p)->clone(tc) : NULL;
        _clone->push_back(child_clone);
        if(child_clone)
            child_clone->set_parent(_clone);
    }
    return _clone;
}

} }
