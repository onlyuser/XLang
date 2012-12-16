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

#ifndef XLANG_DEFAULT_TOUR_H_
#define XLANG_DEFAULT_TOUR_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "visitor/XLangVisitor.h" // visitor::Visitor
#include <sstream> // std::stringstream

namespace xl { namespace visitor {

struct DefaultTour : public Visitor<const node::NodeIdentIFace>
{
    virtual ~DefaultTour()
    {}
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::INT>* _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::FLOAT>* _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::STRING>* _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::CHAR>* _node);
    virtual void visit(const node::TermNodeIFace<node::NodeIdentIFace::IDENT>* _node);
    virtual void visit_null();
    int get_next_child_index(const node::SymbolNodeIFace* _node);
    node::NodeIdentIFace* get_next_child(const node::SymbolNodeIFace* _node);
    virtual bool visit_next_child(const node::SymbolNodeIFace* _node,
            node::NodeIdentIFace** ref_node = NULL);
    virtual void abort_visitation(const node::SymbolNodeIFace* _node);
    virtual void visit(const node::SymbolNodeIFace* _node);
    virtual void visit_any(const node::NodeIdentIFace* unknown);
};

} }

#endif
