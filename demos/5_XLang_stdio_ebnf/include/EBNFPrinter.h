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
#include <vector>

class EBNFPrinter : public xl::visitor::DefaultTour
{
public:
    typedef void (*delayed_ast_mutate_cb_t)(void*);
    EBNFPrinter()
    {}
    void visit(const xl::node::SymbolNodeIFace* _node);

private:
    std::vector<delayed_ast_mutate_cb_t> m_delayed_ast_mutate_cb_vec; // FIX-ME!
};

#endif
