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
#include <sstream>

class EBNFPrinter : public xl::visitor::DefaultTour
{
public:
    EBNFPrinter(xl::TreeContext* tc)
        : m_tc(tc), m_symbols_node(NULL), m_rules_node(NULL)
    {}
    void visit(const xl::node::SymbolNodeIFace* _node);
    void apply_changes(bool* changed);
    void redirect_stdout();
    std::string restore_stdout();

private:
    xl::TreeContext* m_tc;
    std::list<std::string> m_new_symbol_list;
    std::list<xl::node::NodeIdentIFace*> m_new_rule_list;
    const xl::node::NodeIdentIFace *m_symbols_node, *m_rules_node;

    // redirect stdout
    std::stringstream m_cout_buf;
    std::streambuf* m_prev_stream_buf;
};

#endif
