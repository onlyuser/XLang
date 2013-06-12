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

#include "node/XLangNode.h" // node::NodeIdentIFace
#include "XLangTreeContext.h" // TreeContext
#include <sstream> // std::stringstream

// prototype
extern std::string id_to_name(uint32_t lexer_id);

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
    return id_to_name(lexer_id());
}

std::string Node::uid() const
{
    return ptr_to_string(this);
}

SymbolNode::SymbolNode(uint32_t _lexer_id, YYLTYPE loc, size_t _size, va_list ap)
    : Node(NodeIdentIFace::SYMBOL, _lexer_id, loc), visitor::Visitable<SymbolNode>(this)
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

void SymbolNode::push_back(NodeIdentIFace* _node)
{
    m_child_vec.push_back(_node);
    if(_node)
        _node->set_parent(this);
}

} }
