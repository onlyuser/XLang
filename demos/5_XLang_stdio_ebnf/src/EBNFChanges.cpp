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

//#define DEBUG_EBNF
#ifdef DEBUG_EBNF
    #include "mvc/XLangMVCView.h" // mvc::MVCView
    #include <iostream> // std::cout
    #include <sstream> // std::stringstream

    static std::string ptr_to_string(const void* x)
    {
        std::stringstream ss;
        ss << '_' << x;
        std::string s = ss.str();
        return s;
    }
#endif

void EBNFChanges::reset()
{
    m_insertions_after.clear();
    m_append_to.clear();
    m_replacements.clear();
}

bool EBNFChanges::apply()
{
#ifdef DEBUG_EBNF
    std::cout << "BEGIN APPLYING CHANGES" << std::endl;
#endif
    bool changed = false;
    if(!m_insertions_after.empty())
    {
        // NOTE: traversal not "in order"
        for(auto p = m_insertions_after.begin(); p != m_insertions_after.end(); ++p)
        {
            const xl::node::NodeIdentIFace* after_node = (*p).first;
            if(!after_node)
                continue;
            xl::node::NodeIdentIFace* parent_node = after_node->parent();
            if(!parent_node)
                continue;
            xl::node::SymbolNodeIFace* parent_symbol =
                    dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
            if(parent_symbol)
            {
                std::list<xl::node::NodeIdentIFace*> &insert_after_list = (*p).second;
                for(auto q = insert_after_list.begin(); q != insert_after_list.end(); ++q)
                {
#ifdef DEBUG_EBNF
                    std::cout << "INSERT_AFTER " << ptr_to_string(after_node) << " ==> "
                            << ptr_to_string(*q) << std::endl;
                    //xl::mvc::MVCView::print_xml(*q);
#endif
                    parent_symbol->insert_after(
                            const_cast<xl::node::NodeIdentIFace*>(after_node), // TODO: fix-me!
                            *q);
                }
            }
        }
        changed = true;
    }
    if(!m_replacements.empty())
    {
        // NOTE: traversal not "in order"
        for(auto r = m_replacements.begin(); r != m_replacements.end(); ++r)
        {
            const xl::node::NodeIdentIFace* find_node = (*r).first;
            xl::node::NodeIdentIFace* replace_node = (*r).second;
            if(!find_node)
                continue;
            xl::node::NodeIdentIFace* parent_node = find_node->parent();
            if(!parent_node)
                continue;
            xl::node::SymbolNodeIFace* parent_symbol =
                    dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
            if(parent_symbol)
            {
#ifdef DEBUG_EBNF
                std::cout << "REPLACE " << ptr_to_string(find_node) << " ==> "
                        << ptr_to_string(replace_node) << std::endl;
                //xl::mvc::MVCView::print_xml(replace_node);
#endif
                parent_symbol->replace(
                        const_cast<xl::node::NodeIdentIFace*>(find_node), // TODO: fix-me!
                        replace_node);
            }
        }
        changed = true;
    }
#ifdef DEBUG_EBNF
    std::cout << "END APPLYING CHANGES" << std::endl << std::endl;
#endif
    return changed;
}
