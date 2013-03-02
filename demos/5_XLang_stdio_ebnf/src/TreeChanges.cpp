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

#include "TreeChanges.h" // TreeChanges
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "node/XLangNode.h"

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

void TreeChanges::reset()
{
    m_node_insertions_after.clear();
    m_node_appends_to_back.clear();
    m_string_appends_to_back.clear();
    m_string_insertions_to_front.clear();
    m_node_replacements.clear();
}

void TreeChanges::add_node_change(
        TreeChange::node_type_change_t _type,
        const xl::node::NodeIdentIFace* _node,
        xl::node::NodeIdentIFace* new_node)
{
    switch(_type)
    {
        case TreeChange::NODE_INSERTIONS_AFTER:
            m_node_insertions_after[_node].push_back(new_node);
            break;
        case TreeChange::NODE_APPENDS_TO_BACK:
            m_node_appends_to_back[_node].push_back(new_node);
            break;
        case TreeChange::NODE_REPLACEMENTS:
            m_node_replacements[_node] = new_node;
            break;
        default:
            break;
    }
}

void TreeChanges::add_string_change(
        TreeChange::string_type_change_t _type,
        const xl::node::NodeIdentIFace* _node,
        std::string s)
{
    switch(_type)
    {
        case TreeChange::STRING_APPENDS_TO_BACK:
            m_string_appends_to_back[_node].push_back(s);
            break;
        case TreeChange::STRING_INSERTIONS_TO_FRONT:
            m_string_insertions_to_front[_node].push_back(s);
            break;
        default:
            break;
    }
}

bool TreeChanges::apply()
{
#ifdef DEBUG_EBNF
    std::cout << "BEGIN APPLYING CHANGES" << std::endl;
#endif
    bool changed = false;
    if(!m_node_insertions_after.empty())
    {
        // NOTE: unordered traversal
        for(auto p = m_node_insertions_after.begin(); p != m_node_insertions_after.end(); ++p)
        {
            const xl::node::NodeIdentIFace* insert_after_node = (*p).first;
            if(!insert_after_node)
                continue;
            xl::node::NodeIdentIFace* parent_node = insert_after_node->parent();
            if(!parent_node)
                continue;
            xl::node::SymbolNodeIFace* parent_symbol =
                    dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
            if(parent_symbol)
            {
                std::list<xl::node::NodeIdentIFace*> &insert_list = (*p).second;
                for(auto q = insert_list.begin(); q != insert_list.end(); ++q)
                {
                    xl::node::NodeIdentIFace* new_node = *q;
#ifdef DEBUG_EBNF
                    std::cout << "NODE_INSERT_AFTER " << ptr_to_string(insert_after_node) << " ==> "
                            << ptr_to_string(new_node) << std::endl;
                    //xl::mvc::MVCView::print_xml(new_node);
#endif
                    parent_symbol->insert_after(
                            const_cast<xl::node::NodeIdentIFace*>(insert_after_node), // TODO: fix-me!
                            new_node);
                }
            }
        }
        changed = true;
    }
    if(!m_node_appends_to_back.empty())
    {
        // NOTE: unordered traversal
        for(auto i = m_node_appends_to_back.begin(); i != m_node_appends_to_back.end(); ++i)
        {
            const xl::node::NodeIdentIFace* append_to_node = (*i).first;
            if(!append_to_node)
                continue;
            const xl::node::SymbolNodeIFace* append_to_symbol =
                    dynamic_cast<const xl::node::SymbolNodeIFace*>(append_to_node);
            if(append_to_symbol)
            {
                std::list<xl::node::NodeIdentIFace*> &append_list = (*i).second;
                for(auto j = append_list.begin(); j != append_list.end(); ++j)
                {
                    xl::node::NodeIdentIFace* new_node = *j;
#ifdef DEBUG_EBNF
                    std::cout << "NODE_APPEND_BACK " << ptr_to_string(append_to_node) << " ==> "
                            << ptr_to_string(new_node) << std::endl;
                    //xl::mvc::MVCView::print_xml(append_to_node);
                    //xl::mvc::MVCView::print_xml(new_node);
#endif
                    const_cast<xl::node::SymbolNodeIFace*>( // TODO: fix-me!
                            append_to_symbol
                            )->push_back(new_node);
                }
            }
        }
        changed = true;
    }
    if(!m_string_appends_to_back.empty())
    {
        // NOTE: unordered traversal
        for(auto u = m_string_appends_to_back.begin(); u != m_string_appends_to_back.end(); ++u)
        {
            const xl::node::NodeIdentIFace* append_to_node = (*u).first;
            if(!append_to_node)
                continue;
            const xl::node::TermNode<xl::node::NodeIdentIFace::STRING>* append_to_term =
                    dynamic_cast<const xl::node::TermNode<xl::node::NodeIdentIFace::STRING>*>(append_to_node);
            if(append_to_term)
            {
                std::list<std::string> &append_list = (*u).second;
                for(auto v = append_list.begin(); v != append_list.end(); ++v)
                {
                    std::string s = *v;
#ifdef DEBUG_EBNF
                    std::cout << "STRING_APPEND_BACK " << ptr_to_string(append_to_node) << " ==> "
                            << '\"' << s << '\"' << std::endl;
                    //xl::mvc::MVCView::print_xml(append_to_node);
#endif
                    std::string &s_lvalue =
                            *const_cast<xl::node::TermNode<xl::node::NodeIdentIFace::STRING>*>( // TODO: fix-me!
                                    append_to_term
                                    )->value();
                    s_lvalue.append(s);
                }
            }
        }
        changed = true;
    }
    if(!m_string_insertions_to_front.empty())
    {
        // NOTE: unordered traversal
        for(auto m = m_string_insertions_to_front.begin(); m != m_string_insertions_to_front.end(); ++m)
        {
            const xl::node::NodeIdentIFace* insertion_to_node = (*m).first;
            if(!insertion_to_node)
                continue;
            const xl::node::TermNode<xl::node::NodeIdentIFace::STRING>* insertion_to_term =
                    dynamic_cast<const xl::node::TermNode<xl::node::NodeIdentIFace::STRING>*>(insertion_to_node);
            if(insertion_to_term)
            {
                std::list<std::string> &insert_list = (*m).second;
                for(auto v = insert_list.begin(); v != insert_list.end(); ++v)
                {
                    std::string s = *v;
#ifdef DEBUG_EBNF
                    std::cout << "STRING_INSERT_FRONT " << ptr_to_string(insertion_to_node) << " ==> "
                            << '\"' << s << '\"' << std::endl;
                    //xl::mvc::MVCView::print_xml(insertion_to_node);
#endif
                    std::string &s_lvalue =
                            *const_cast<xl::node::TermNode<xl::node::NodeIdentIFace::STRING>*>( // TODO: fix-me!
                                    insertion_to_term
                                    )->value();
                    s_lvalue.insert(0, s);
                }
            }
        }
        changed = true;
    }
    if(!m_node_replacements.empty())
    {
        // NOTE: unordered traversal
        for(auto r = m_node_replacements.begin(); r != m_node_replacements.end(); ++r)
        {
            const xl::node::NodeIdentIFace* find_node = (*r).first;
            if(!find_node)
                continue;
            xl::node::NodeIdentIFace* parent_node = find_node->parent();
            if(!parent_node)
                continue;
            xl::node::SymbolNodeIFace* parent_symbol =
                    dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
            if(parent_symbol)
            {
                xl::node::NodeIdentIFace* replacement_node = (*r).second;
#ifdef DEBUG_EBNF
                std::cout << "NODE_REPLACE " << ptr_to_string(find_node) << " ==> "
                        << ptr_to_string(replacement_node) << std::endl;
                //xl::mvc::MVCView::print_xml(find_node);
                //xl::mvc::MVCView::print_xml(replacement_node);
#endif
                parent_symbol->replace_first(
                        const_cast<xl::node::NodeIdentIFace*>(find_node), // TODO: fix-me!
                        replacement_node);
            }
        }
        changed = true;
    }
#ifdef DEBUG_EBNF
    std::cout << "END APPLYING CHANGES" << std::endl << std::endl;
#endif
    return changed;
}
