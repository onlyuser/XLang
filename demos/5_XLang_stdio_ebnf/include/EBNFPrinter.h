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

#ifndef EBNF_PRINTER_H_
#define EBNF_PRINTER_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "visitor/XLangDefaultTour.h" // visitor::DefaultTour
#include "XLangTreeContext.h" // TreeContext
#include <list> // std::list
#include <set> // std::set

struct EBNFChanges
{
    EBNFChanges(xl::TreeContext* tc)
        : m_tc(tc), m_symbols_node(NULL), m_rules_node(NULL)
    {}
    void reset();
    bool apply();

private:
    xl::TreeContext* m_tc;
public:
    const xl::node::NodeIdentIFace *m_symbols_node, *m_rules_node;
    std::list<std::string> m_new_symbol_list;
    std::list<xl::node::NodeIdentIFace*> m_new_rule_list;
    std::set<const xl::node::NodeIdentIFace*> m_remove_set;
};

class EBNFPrinter : public xl::visitor::DefaultTour
{
public:
    EBNFPrinter(xl::TreeContext* tc, EBNFChanges* changes = NULL)
        : m_tc(tc), m_changes(changes)
    {}
    void visit(const xl::node::SymbolNodeIFace* _node);

private:
    xl::TreeContext* m_tc;
    EBNFChanges* m_changes;
};

#endif
