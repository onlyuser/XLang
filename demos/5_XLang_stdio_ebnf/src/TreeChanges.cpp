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

void TreeChangeImpl<TreeChange::NODE_INSERTIONS_AFTER>::apply()
{
    const xl::node::NodeIdentIFace* insert_after_node = m_reference_node;
    if(!insert_after_node)
        return;
    xl::node::NodeIdentIFace* parent_node = insert_after_node->parent();
    if(!parent_node)
        return;
    xl::node::SymbolNodeIFace* parent_symbol =
            dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
    if(!parent_symbol)
        return;
    xl::node::NodeIdentIFace* new_node = m_new_node;
#ifdef DEBUG_EBNF
    std::cout << "NODE_INSERT_AFTER " << ptr_to_string(insert_after_node) << " ==> "
            << ptr_to_string(new_node) << std::endl;
    //xl::mvc::MVCView::print_xml(new_node);
#endif
    parent_symbol->insert_after(
            const_cast<xl::node::NodeIdentIFace*>(insert_after_node), // TODO: fix-me!
            new_node);
}

void TreeChangeImpl<TreeChange::NODE_APPENDS_TO_BACK>::apply()
{
    const xl::node::NodeIdentIFace* append_to_node = m_reference_node;
    if(!append_to_node)
        return;
    const xl::node::SymbolNodeIFace* append_to_symbol =
            dynamic_cast<const xl::node::SymbolNodeIFace*>(append_to_node);
    if(!append_to_symbol)
        return;
    xl::node::NodeIdentIFace* new_node = m_new_node;
#ifdef DEBUG_EBNF
    std::cout << "NODE_APPEND_TO_BACK " << ptr_to_string(append_to_node) << " ==> "
            << ptr_to_string(new_node) << std::endl;
    //xl::mvc::MVCView::print_xml(append_to_node);
    //xl::mvc::MVCView::print_xml(new_node);
#endif
    const_cast<xl::node::SymbolNodeIFace*>( // TODO: fix-me!
            append_to_symbol
            )->push_back(new_node);
}

void TreeChangeImpl<TreeChange::NODE_REPLACEMENTS>::apply()
{
    const xl::node::NodeIdentIFace* find_node = m_reference_node;
    if(!find_node)
        return;
    xl::node::NodeIdentIFace* parent_node = find_node->parent();
    if(!parent_node)
        return;
    xl::node::SymbolNodeIFace* parent_symbol =
            dynamic_cast<xl::node::SymbolNodeIFace*>(parent_node);
    if(!parent_symbol)
        return;
    xl::node::NodeIdentIFace* replacement_node = m_new_node;
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

void TreeChangeImpl<TreeChange::STRING_APPENDS_TO_BACK>::apply()
{
    const xl::node::NodeIdentIFace* append_to_node = m_reference_node;
    if(!append_to_node)
        return;
    const xl::node::TermNode<xl::node::NodeIdentIFace::STRING>* append_to_term =
            dynamic_cast<const xl::node::TermNode<xl::node::NodeIdentIFace::STRING>*>(append_to_node);
    if(!append_to_term)
        return;
    std::string s = m_new_string;
#ifdef DEBUG_EBNF
    std::cout << "STRING_APPEND_TO_BACK " << ptr_to_string(append_to_node) << " ==> "
            << '\"' << s << '\"' << std::endl;
    //xl::mvc::MVCView::print_xml(append_to_node);
#endif
    std::string &s_lvalue =
            *const_cast<xl::node::TermNode<xl::node::NodeIdentIFace::STRING>*>( // TODO: fix-me!
                    append_to_term
                    )->value();
    s_lvalue.append(s);
}

void TreeChangeImpl<TreeChange::STRING_INSERTIONS_TO_FRONT>::apply()
{
    const xl::node::NodeIdentIFace* insertion_to_node = m_reference_node;
    if(!insertion_to_node)
        return;
    const xl::node::TermNode<xl::node::NodeIdentIFace::STRING>* insertion_to_term =
            dynamic_cast<const xl::node::TermNode<xl::node::NodeIdentIFace::STRING>*>(insertion_to_node);
    if(!insertion_to_term)
        return;
    std::string s = m_new_string;
#ifdef DEBUG_EBNF
    std::cout << "STRING_INSERT_TO_FRONT " << ptr_to_string(insertion_to_node) << " ==> "
            << '\"' << s << '\"' << std::endl;
    //xl::mvc::MVCView::print_xml(insertion_to_node);
#endif
    std::string &s_lvalue =
            *const_cast<xl::node::TermNode<xl::node::NodeIdentIFace::STRING>*>( // TODO: fix-me!
                    insertion_to_term
                    )->value();
    s_lvalue.insert(0, s);
}

TreeChanges::~TreeChanges()
{
    reset();
}

void TreeChanges::reset()
{
    for(auto p = m_tree_changes.begin(); p != m_tree_changes.end(); p++)
        delete *p;
}

void TreeChanges::add_change(
        TreeChange::type_t _type,
        const xl::node::NodeIdentIFace* reference_node,
        xl::node::NodeIdentIFace* new_node)
{
    switch(_type)
    {
        case TreeChange::NODE_INSERTIONS_AFTER:
            m_tree_changes.push_back(
                    new TreeChangeImpl<TreeChange::NODE_INSERTIONS_AFTER>(_type, reference_node, new_node));
            break;
        case TreeChange::NODE_APPENDS_TO_BACK:
            m_tree_changes.push_back(
                    new TreeChangeImpl<TreeChange::NODE_APPENDS_TO_BACK>(_type, reference_node, new_node));
            break;
        case TreeChange::NODE_REPLACEMENTS:
            m_tree_changes.push_back(
                    new TreeChangeImpl<TreeChange::NODE_REPLACEMENTS>(_type, reference_node, new_node));
            break;
        default:
            break;
    }
}

void TreeChanges::add_change(
        TreeChange::type_t _type,
        const xl::node::NodeIdentIFace* reference_node,
        std::string new_string)
{
    switch(_type)
    {
        case TreeChange::STRING_APPENDS_TO_BACK:
            m_tree_changes.push_back(
                    new TreeChangeImpl<TreeChange::STRING_APPENDS_TO_BACK>(_type, reference_node, new_string));
            break;
        case TreeChange::STRING_INSERTIONS_TO_FRONT:
            m_tree_changes.push_back(
                    new TreeChangeImpl<TreeChange::STRING_INSERTIONS_TO_FRONT>(_type, reference_node, new_string));
            break;
        default:
            break;
    }
}

bool TreeChanges::apply()
{
    if(m_tree_changes.empty())
        return false;
#ifdef DEBUG_EBNF
    std::cout << "BEGIN APPLYING CHANGES" << std::endl;
#endif
    for(auto p = m_tree_changes.begin(); p != m_tree_changes.end(); p++)
        (*p)->apply();
#ifdef DEBUG_EBNF
    std::cout << "END APPLYING CHANGES" << std::endl << std::endl;
#endif
    return true;
}
