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

#ifndef EVAL_VISITOR_H_
#define EVAL_VISITOR_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "visitor/XLangDefaultTour.h" // visitor::DefaultTour

struct EvalVisitor : public xl::visitor::DefaultTour
{
public:
	EvalVisitor()
    {}
    void visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::INT>* _node);
    void visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::FLOAT>* _node);
    void visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>* _node);
    void visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::CHAR>* _node);
    void visit(const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::IDENT>* _node);
    void visit(const xl::node::SymbolNodeIFace* _node);

private:
    float32_t value;
};

#endif
