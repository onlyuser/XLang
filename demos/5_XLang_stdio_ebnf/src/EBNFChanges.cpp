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

#include "EBNFChanges.h" // EBNFChanges
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace

void EBNFChanges::reset()
{
    m_symbols_attach_loc_map.clear();
    m_insertions_after.clear();
    m_replacements.clear();
}

bool EBNFChanges::apply()
{
    bool changed = false;
    if(!m_insertions_after.empty())
    {
        for(auto u = m_insertions_after.begin(); u != m_insertions_after.end(); ++u)
        {
            const xl::node::NodeIdentIFace* after_node = (*u).first;
            if(!after_node)
                continue;
            xl::node::NodeIdentIFace* parent_node = after_node->parent();
            if(!parent_node)
                continue;
            xl::node::SymbolNodeIFace* parent_symbol =
                    dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
            if(parent_symbol)
            {
                std::list<xl::node::NodeIdentIFace*> &insert_after_list = (*u).second;
                for(auto v = insert_after_list.begin(); v != insert_after_list.end(); ++v)
                {
                    parent_symbol->insert_after(
                            const_cast<xl::node::NodeIdentIFace*>(after_node),
                            *v); // TODO: fix-me!
                }
            }
        }
        changed = true;
    }
    if(!m_replacements.empty())
    {
        for(auto t = m_replacements.begin(); t != m_replacements.end(); ++t)
        {
            const xl::node::NodeIdentIFace* find_node = (*t).first;
            xl::node::NodeIdentIFace* replace_node = (*t).second;
            if(!find_node)
                continue;
            xl::node::NodeIdentIFace* parent_node = find_node->parent();
            if(!parent_node)
                continue;
            xl::node::SymbolNodeIFace* parent_symbol =
                    dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
            if(parent_symbol)
            {
                parent_symbol->replace(
                        const_cast<xl::node::NodeIdentIFace*>(find_node),
                        replace_node); // TODO: fix-me!
            }
        }
        changed = true;
    }
    return changed;
}
