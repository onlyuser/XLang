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
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLangTreeContext.h" // TreeContext

#define MAKE_TERM(sym_id, ...) xl::mvc::MVCModel::make_term(tc, sym_id, ##__VA_ARGS__)

void EBNFChanges::reset()
{
    m_symbols_node = NULL;
    m_rules_node = NULL;
    m_new_symbol_list.clear();
    m_existing_symbol_map.clear();
    m_new_rule_list.clear();
    m_remove_set.clear();
}

bool EBNFChanges::apply()
{
    bool changed = false;
    if(m_symbols_node && !m_new_symbol_list.empty())
    {
        xl::node::SymbolNodeIFace* attach_point =
                const_cast<xl::node::SymbolNodeIFace*>(
                        dynamic_cast<const xl::node::SymbolNodeIFace*>(m_symbols_node)
                        );
        if(attach_point)
        {
            for(auto p = m_new_symbol_list.begin(); p != m_new_symbol_list.end(); ++p)
            {
                // insert front to avoid eol-symbol
                xl::TreeContext* tc = m_tc;
                attach_point->push_front(MAKE_TERM(ID_IDENT, tc->alloc_unique_string(*p)));
            }
            changed = true;
        }
        if(!m_existing_symbol_map.empty())
        {
            for(auto t = m_new_symbol_list.begin(); t != m_new_symbol_list.end(); ++t)
            {
                auto u = m_existing_symbol_map.find(*t);
                if(u != m_existing_symbol_map.end())
                    (*u).second->detach();
            }
        }
    }
    if(m_rules_node && !m_new_rule_list.empty())
    {
        xl::node::SymbolNodeIFace* attach_point =
                const_cast<xl::node::SymbolNodeIFace*>(
                        dynamic_cast<const xl::node::SymbolNodeIFace*>(m_rules_node)
                        );
        if(attach_point)
        {
            for(auto q = m_new_rule_list.begin(); q != m_new_rule_list.end(); ++q)
                attach_point->push_front(*q); // insert front to avoid eol-symbol
            changed = true;
        }
    }
    if(!m_remove_set.empty())
    {
        for(auto r = m_remove_set.begin(); r != m_remove_set.end(); ++r)
            const_cast<xl::node::NodeIdentIFace*>(*r)->detach(); // TODO: fix-me!
        changed = true;
    }
    return changed;
}
