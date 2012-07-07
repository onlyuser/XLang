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
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "node/XLangNode.h" // node::Node
#include "XLangTreeContext.h" // TreeContext
#include <iostream> // std::cout
#include <vector>
#include <sstream>

static std::string gen_name(std::string stem)
{
    static int index;
    std::stringstream ss;
    ss << stem << '_' << index;
    return ss.str();
}

static xl::node::NodeIdentIFace* make_stem_rule(std::string name,
        const xl::node::NodeIdentIFace* rule_node, xl::TreeContext* tc)
{
    const xl::node::NodeIdentIFace* rule_node_copy = rule_node->clone(tc);
    return NULL;
}

static xl::node::NodeIdentIFace* make_recursive_rule(std::string name1, std::string name2)
{
    return NULL;
}

static xl::node::NodeIdentIFace* make_term_rule(std::string name)
{
    return NULL;
}

static const xl::node::NodeIdentIFace* find_ancestor_node(uint32_t sym_id,
        const xl::node::NodeIdentIFace* node)
{
    const xl::node::NodeIdentIFace* cur_node = node;
    do
    {
        if(cur_node->sym_id() == sym_id)
            return cur_node;
    } while((cur_node = cur_node->parent()));
    return NULL;
}

static void expand_kleene_closure(char closure_type,
        const xl::node::NodeIdentIFace* rule_node,
        const xl::node::NodeIdentIFace* kleene_node,
        const xl::node::NodeIdentIFace* child,
        xl::TreeContext* tc)
{
    std::string name1 = gen_name("");
    std::string name2 = gen_name("");
    switch(closure_type)
    {
        case '+':
            make_stem_rule(name1, rule_node, tc);
            make_recursive_rule(name1, name2);
            make_term_rule(name2);
            break;
        case '*':
            make_stem_rule(name1, rule_node, tc);
            make_recursive_rule(name1, name2);
            make_term_rule(name2);
            break;
        case '?':
            make_stem_rule(name1, rule_node, tc);
            make_recursive_rule(name1, name2);
            make_term_rule(name2);
            break;
    }
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
                const xl::node::NodeIdentIFace* rule_node = find_ancestor_node(ID_RULE, _node);
                xl::node::SymbolNodeIFace* paren_node =
                        dynamic_cast<xl::node::SymbolNodeIFace*>((*_node)[0]);
                xl::node::NodeIdentIFace* alt_node = (*paren_node)[0];
                expand_kleene_closure(_node->sym_id(), rule_node, _node, alt_node, m_tc);
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
