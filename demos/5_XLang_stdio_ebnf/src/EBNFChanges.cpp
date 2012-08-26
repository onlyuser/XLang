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
    m_symbols_attach_loc = NULL;
    m_rules_attach_loc = NULL;
    m_new_symbols.clear();
    m_existing_symbols.clear();
    m_new_rules.clear();
    m_removals.clear();
    m_replacements.clear();
}

bool EBNFChanges::apply()
{
    bool changed = false;
    if(m_symbols_attach_loc && !m_new_symbols.empty())
    {
        xl::node::SymbolNodeIFace* symbols_attach_loc =
                const_cast<xl::node::SymbolNodeIFace*>(
                        dynamic_cast<const xl::node::SymbolNodeIFace*>(m_symbols_attach_loc)
                        );
        if(symbols_attach_loc)
        {
            for(auto p = m_new_symbols.begin(); p != m_new_symbols.end(); ++p)
            {
                if(m_existing_symbols.find(*p) == m_existing_symbols.end())
                {
                    // insert front to avoid eol-symbol
                    xl::TreeContext* tc = m_tc;
                    symbols_attach_loc->push_front(MAKE_TERM(ID_IDENT, tc->alloc_unique_string(*p)));
                }
            }
            changed = true;
        }
    }
    if(m_rules_attach_loc && !m_new_rules.empty())
    {
        xl::node::SymbolNodeIFace* rules_attach_loc =
                const_cast<xl::node::SymbolNodeIFace*>(
                        dynamic_cast<const xl::node::SymbolNodeIFace*>(m_rules_attach_loc)
                        );
        if(rules_attach_loc)
        {
            for(auto q = m_new_rules.begin(); q != m_new_rules.end(); ++q)
                rules_attach_loc->push_front(*q); // insert front to avoid eol-symbol
            changed = true;
        }
    }
    if(!m_removals.empty())
    {
        for(auto r = m_removals.begin(); r != m_removals.end(); ++r)
            const_cast<xl::node::NodeIdentIFace*>(*r)->detach(); // TODO: fix-me!
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
            xl::node::SymbolNodeIFace* parent_symbol = dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
            if(parent_symbol)
                parent_symbol->replace(
                        const_cast<xl::node::NodeIdentIFace*>(find_node),
                        replace_node); // TODO: fix-me!
        }
        changed = true;
    }
    return changed;
}
