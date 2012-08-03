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

#include "EBNFPrinter.h" // EBNFPrinter
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "node/XLangNode.h" // node::Node
#include "XLangTreeContext.h" // TreeContext
#include <iostream> // std::cout
#include <algorithm> // std::replace

#define MAKE_TERM(sym_id, ...) xl::mvc::MVCModel::make_term(tc, sym_id, ##__VA_ARGS__)
#define MAKE_SYMBOL(...)       xl::mvc::MVCModel::make_symbol(tc, ##__VA_ARGS__)

static std::string gen_name(std::string stem)
{
    static int index;
    std::stringstream ss;
    ss << stem << '_' << index++;
    return ss.str();
}

static xl::node::NodeIdentIFace* kleene_node_from_rule_node(
        const xl::node::NodeIdentIFace* rule_node)
{
    return NULL;
}

static void replace_node(
        const xl::node::NodeIdentIFace* replaced_node,
        const xl::node::NodeIdentIFace* replacement_node)
{
    xl::node::SymbolNodeIFace* parent = dynamic_cast<xl::node::SymbolNodeIFace*>(replaced_node->parent());
    parent->replace_child(
            const_cast<xl::node::NodeIdentIFace*>(replaced_node),
            const_cast<xl::node::NodeIdentIFace*>(replacement_node));
}

static xl::node::NodeIdentIFace* make_stem_rule(
        std::string name,
        const xl::node::NodeIdentIFace* rule_node,
        xl::TreeContext* tc)
{
    return NULL;
    xl::node::NodeIdentIFace* rule_node_copy = rule_node->clone(tc);
    xl::node::NodeIdentIFace* kleene_node = kleene_node_from_rule_node(rule_node_copy); // <-- NULL
    xl::node::NodeIdentIFace* kleene_node_replacement =
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name));
    replace_node(kleene_node, kleene_node_replacement); // <-- FIX-ME!: segfault here
    return rule_node_copy;
}

static xl::node::NodeIdentIFace* make_recursive_rule_plus(std::string name1, std::string name2,
        xl::TreeContext* tc)
{
    return NULL;
}

static xl::node::NodeIdentIFace* make_recursive_rule_star(std::string name1, std::string name2,
        xl::TreeContext* tc)
{
    return NULL;
}

static xl::node::NodeIdentIFace* make_recursive_rule_optional(std::string name1, std::string name2,
        xl::TreeContext* tc)
{
    return NULL;
}

static xl::node::NodeIdentIFace* make_term_rule(
        std::string name,
        const xl::node::NodeIdentIFace* alt_node,
        xl::TreeContext* tc)
{
    return MAKE_SYMBOL(ID_RULE, 2,
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name)),
            alt_node->clone(tc));
}

static const xl::node::NodeIdentIFace* ancestor_node(
        uint32_t sym_id,
        const xl::node::NodeIdentIFace* node)
{
    for(const xl::node::NodeIdentIFace* p = node; p; p = p->parent())
    {
        if(p->sym_id() == sym_id)
            return p;
    }
    return NULL;
}

static std::string lhs_value_from_rule_node(const xl::node::NodeIdentIFace* rule_node)
{
    xl::node::NodeIdentIFace* lhs_node =
            (*dynamic_cast<const xl::node::SymbolNodeIFace*>(rule_node))[0];
    const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::IDENT>* lhs_term =
            dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::IDENT>*>(lhs_node);
    return *lhs_term->value();
}

static void expand_kleene_closure(
        std::list<std::string> *new_symbol_list,
        std::list<xl::node::NodeIdentIFace*> *new_rule_list,
        std::set<const xl::node::NodeIdentIFace*> *remove_set,
        char closure_type,
        const xl::node::NodeIdentIFace* rule_node,
        const xl::node::NodeIdentIFace* alt_node,
        xl::TreeContext* tc)
{
    if(!new_symbol_list || !new_rule_list || !remove_set)
        return;
    std::string lhs_value = lhs_value_from_rule_node(rule_node);
    std::string name1 = gen_name(lhs_value);
    std::string name2 = gen_name(lhs_value);
    xl::node::NodeIdentIFace* stem_rule = make_stem_rule(name1, rule_node, tc);
    if(stem_rule)
    {
        std::cout << "(stem_rule) <<<" << std::endl;
        EBNFPrinter v(tc); v.visit_any(stem_rule); std::cout << std::endl;
        std::cout << ">>> (stem_rule)" << std::endl;
        new_symbol_list->push_back(lhs_value);
        new_rule_list->push_back(stem_rule);
    }
    xl::node::NodeIdentIFace* recursive_rule = NULL;
    switch(closure_type)
    {
        case '+': recursive_rule = make_recursive_rule_plus(name1, name2, tc); break;
        case '*': recursive_rule = make_recursive_rule_star(name1, name2, tc); break;
        case '?': recursive_rule = make_recursive_rule_optional(name1, name2, tc); break;
    }
    if(recursive_rule)
    {
        std::cout << "(recursive_rule) <<<" << std::endl;
        EBNFPrinter v(tc); v.visit_any(recursive_rule); std::cout << std::endl;
        std::cout << ">>> (recursive_rule)" << std::endl;
        new_symbol_list->push_back(name1);
        new_rule_list->push_back(recursive_rule);
    }
    xl::node::NodeIdentIFace* term_rule = make_term_rule(name2, alt_node, tc);
    if(term_rule)
    {
        std::cout << "(term_rule) <<<" << std::endl;
        EBNFPrinter v(tc); v.visit_any(term_rule); std::cout << std::endl;
        std::cout << ">>> (term_rule)" << std::endl;
        new_symbol_list->push_back(name2);
        new_rule_list->push_back(term_rule);
    }
    remove_set->insert(rule_node);
}

static xl::node::NodeIdentIFace* alt_node_from_kleene_node(
        const xl::node::SymbolNodeIFace* kleene_node)
{
    xl::node::SymbolNodeIFace* paren_node =
            dynamic_cast<xl::node::SymbolNodeIFace*>((*kleene_node)[0]);
    return (*paren_node)[0];
}

void EBNFChanges::reset()
{
    m_symbols_node = NULL;
    m_rules_node = NULL;
    m_new_symbol_list.clear();
    m_new_rule_list.clear();
    m_remove_set.clear();
}

bool EBNFChanges::apply()
{
    bool changed = false;
    if(m_symbols_node)
    {
        xl::node::SymbolNodeIFace* attach_point =
                const_cast<xl::node::SymbolNodeIFace*>(
                        dynamic_cast<const xl::node::SymbolNodeIFace*>(m_symbols_node)
                        );
        if(m_new_symbol_list.size() > 0)
        {
            std::list<std::string>::iterator p;
            for(p = m_new_symbol_list.begin(); p != m_new_symbol_list.end(); p++)
            {
                // insert front to avoid eol-symbol
                xl::TreeContext* tc = m_tc;
                attach_point->insert_child(MAKE_TERM(ID_IDENT, tc->alloc_unique_string(*p)));
            }
            changed = true;
        }
    }
    if(m_rules_node)
    {
        xl::node::SymbolNodeIFace* attach_point =
                const_cast<xl::node::SymbolNodeIFace*>(
                        dynamic_cast<const xl::node::SymbolNodeIFace*>(m_rules_node)
                        );
        if(m_new_rule_list.size() > 0)
        {
            std::list<xl::node::NodeIdentIFace*>::iterator q;
            for(q = m_new_rule_list.begin(); q != m_new_rule_list.end(); q++)
                attach_point->insert_child(*q); // insert front to avoid eol-symbol
            changed = true;
        }
    }
    if(m_remove_set.size() > 0)
    {
        std::set<const xl::node::NodeIdentIFace*>::iterator r;
        for(r = m_remove_set.begin(); r != m_remove_set.end(); r++)
        {
            xl::node::SymbolNodeIFace* parent =
                    dynamic_cast<xl::node::SymbolNodeIFace*>((*r)->parent());
            parent->remove_child(const_cast<xl::node::NodeIdentIFace*>(*r));
        }
        changed = true;
    }
    return changed;
}

void EBNFPrinter::visit(const xl::node::SymbolNodeIFace* _node)
{
    bool more;
    switch(_node->sym_id())
    {
        case ID_GRAMMAR:
            if(m_changes)
                m_changes->reset(); // clear existing changes before starting over
            visit_next_child(_node);
            std::cout << std::endl << std::endl << "%%" << std::endl << std::endl;
            visit_next_child(_node);
            std::cout << std::endl << std::endl << "%%";
            visit_next_child(_node);
            std::cout << std::endl;
            break;
        case ID_DEFINITIONS:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl;
            } while(more);
            break;
        case ID_DECL:
            std::cout << '%';
            more = visit_next_child(_node);
            if(more)
            {
                std::cout << ' ';
                visit_next_child(_node);
            }
            break;
        case ID_DECL_EQ:
            std::cout << '%';
            visit_next_child(_node);
            std::cout << '=';
            visit_next_child(_node);
            break;
        case ID_DECL_BRACE:
            std::cout << '%';
            visit_next_child(_node);
            std::cout << '<';
            visit_next_child(_node);
            std::cout << "> ";
            visit_next_child(_node);
            break;
        case ID_PROTO_BLOCK:
            std::cout << "%{";
            std::cout << dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
                    (*_node)[0])->value();
            std::cout << "%}";
            break;
        case ID_UNION_BLOCK:
            std::cout << '{';
            std::cout << dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
                    (*_node)[0])->value();
            std::cout << '}';
            break;
        case ID_SYMBOLS:
            if(m_changes)
                m_changes->m_symbols_node = _node; // record location so we can insert here later!
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << ' ';
            } while(more);
            break;
        case ID_RULES:
            if(m_changes)
                m_changes->m_rules_node = _node; // record location so we can insert here later!
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl << std::endl;
            } while(more);
            break;
        case ID_RULE:
            visit_next_child(_node);
            std::cout << ':' << std::endl << "\t  ";
            visit_next_child(_node);
            std::cout << ';';
            break;
        case ID_ALTS:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl << "\t| ";
            } while(more);
            break;
        case ID_ALT:
            if(visit_next_child(_node))
                visit_next_child(_node);
            break;
        case ID_ACTION_BLOCK:
            std::cout << " {";
            std::cout << dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
                    (*_node)[0])->value();
            std::cout << '}';
            break;
        case ID_TERMS:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << ' ';
            } while(more);
            break;
        case '+':
        case '*':
        case '?':
            if(m_changes)
                expand_kleene_closure(
                        &m_changes->m_new_symbol_list,
                        &m_changes->m_new_rule_list,
                        &m_changes->m_remove_set,
                        _node->sym_id(),
                        ancestor_node(ID_RULE, _node),
                        alt_node_from_kleene_node(_node),
                        m_tc);
            xl::visitor::DefaultTour::visit(_node);
            std::cout << static_cast<char>(_node->sym_id());
            break;
        case '(':
            std::cout << '(';
            xl::visitor::DefaultTour::visit(_node);
            std::cout << ')';
            break;
        case ID_CODE:
            std::cout << dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
                    (*_node)[0])->value();
            break;
    }
}
