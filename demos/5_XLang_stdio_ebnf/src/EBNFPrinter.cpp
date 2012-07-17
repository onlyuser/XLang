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
#include <vector>
#include <sstream>

#define MAKE_TERM(sym_id, ...) xl::mvc::MVCModel::make_term(tc, sym_id, ##__VA_ARGS__)
#define MAKE_SYMBOL(...)       xl::mvc::MVCModel::make_symbol(tc, ##__VA_ARGS__)

static std::string gen_name(std::string stem)
{
    static int index;
    std::stringstream ss;
    ss << stem << '_' << index++;
    return ss.str();
}

static xl::node::NodeIdentIFace* make_stem_rule(
        std::string name,
        const xl::node::NodeIdentIFace* rule_node,
        xl::TreeContext* tc)
{
    const xl::node::NodeIdentIFace* rule_node_copy = rule_node->clone(tc);
    xl::node::NodeIdentIFace* replaced_kleene_closure =
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name));
    return NULL;
}

static xl::node::NodeIdentIFace* make_recursive_rule(std::string name1, std::string name2,
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
            alt_node->clone(tc)
            );
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

static xl::node::NodeIdentIFace* expand_kleene_closure(char closure_type,
        const xl::node::NodeIdentIFace* rule_node,
        const xl::node::NodeIdentIFace* alt_node,
        xl::TreeContext* tc)
{
    xl::node::NodeIdentIFace* result = NULL;
    std::string lhs_value = lhs_value_from_rule_node(rule_node);
    std::string name1 = gen_name(lhs_value);
    std::string name2 = gen_name(lhs_value);
//    std::cout << "NAME1: " << name1 << std::endl;
//    std::cout << "NAME2: " << name2 << std::endl;
    switch(closure_type)
    {
        case '+':
            make_stem_rule(name1, rule_node, tc);
            make_recursive_rule(name1, name2, tc);
            result = make_term_rule(name2, alt_node, tc);
            break;
        case '*':
            make_stem_rule(name1, rule_node, tc);
            make_recursive_rule(name1, name2, tc);
            result = make_term_rule(name2, alt_node, tc);
            break;
        case '?':
            make_stem_rule(name1, rule_node, tc);
            make_recursive_rule(name1, name2, tc);
            result = make_term_rule(name2, alt_node, tc);
            break;
    }
    return result;
}

static xl::node::NodeIdentIFace* alt_node_from_kleene_node(
        const xl::node::SymbolNodeIFace* kleene_node)
{
    xl::node::SymbolNodeIFace* paren_node =
            dynamic_cast<xl::node::SymbolNodeIFace*>((*kleene_node)[0]);
    return (*paren_node)[0];
}

void EBNFPrinter::visit(const xl::node::SymbolNodeIFace* _node)
{
    bool more;
    switch(_node->sym_id())
    {
        case ID_GRAMMAR:
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
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << ' ';
            } while(more);
            break;
        case ID_RULES:
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
            {
                const xl::node::NodeIdentIFace* rule_node = ancestor_node(ID_RULE, _node);
                xl::node::NodeIdentIFace* alt_node = alt_node_from_kleene_node(_node);
                xl::node::NodeIdentIFace* temp =
                        expand_kleene_closure(_node->sym_id(), rule_node, alt_node, m_tc);
//                std::cout << "BEFORE" << std::endl;
//                xl::visitor::DefaultTour::visit_any(temp);
//                std::cout << "AFTER" << std::endl;
            }
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
