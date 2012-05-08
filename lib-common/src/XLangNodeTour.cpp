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

#include "node/XLangNodeTour.h" // node::NodeTour
#include <iostream> // std::cout

#ifdef EXTERN_INCLUDE_PATH
    #define HAVE_COROUTINE
#endif
#ifdef HAVE_COROUTINE
	#include "coroutine/coroutine_cpp.h"
#endif

namespace node {

void NodeTour::visit(const LeafNodeIFace<NodeIdentIFace::INT>* _node)
{
    std::cout << _node->value();
}

void NodeTour::visit(const LeafNodeIFace<NodeIdentIFace::FLOAT>* _node)
{
    std::cout << _node->value();
}

void NodeTour::visit(const LeafNodeIFace<NodeIdentIFace::STRING>* _node)
{
    std::cout << '\"' << _node->value() << '\"';
}

void NodeTour::visit(const LeafNodeIFace<NodeIdentIFace::CHAR>* _node)
{
    std::cout << '\'' << _node->value() << '\'';
}

void NodeTour::visit(const LeafNodeIFace<NodeIdentIFace::IDENT>* _node)
{
    std::cout << *_node->value();
}

#ifdef HAVE_COROUTINE
static int get_next_asc(ccrContParam, const InnerNodeIFace* _node)
{
	ccrBeginContext;
	int i;
	ccrEndContext(foo);
	ccrBegin(foo);
	for(foo->i = 0; foo->i < static_cast<int>(_node->size()); foo->i++)
		ccrReturn(foo->i);
	ccrFinish(-1);
}
#endif

int NodeTour::visit(const InnerNodeIFace* _node)
{
#ifdef HAVE_COROUTINE
	int index = get_next_asc(&const_cast<InnerNodeIFace*>(_node)->getVisitState(), _node);
	if(index == -1)
		return -1;
	NodeIdentIFace* child = _node->operator[](index);
	switch(child->type())
	{
		case NodeIdentIFace::INT:
			visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::INT>*>(child));
			break;
		case NodeIdentIFace::FLOAT:
			visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::FLOAT>*>(child));
			break;
		case NodeIdentIFace::STRING:
			visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::STRING>*>(child));
			break;
		case NodeIdentIFace::CHAR:
			visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::CHAR>*>(child));
			break;
		case NodeIdentIFace::IDENT:
			visit(dynamic_cast<const node::LeafNodeIFace<node::NodeIdentIFace::IDENT>*>(child));
			break;
		case NodeIdentIFace::INNER:
			visit(dynamic_cast<const node::InnerNodeIFace*>(child));
			break;
	}
	return index;
#else
	return -1;
#endif
}

void NodeTour::flush(const InnerNodeIFace* _node)
{
	int index;
	do
	{
		index = visit(_node);
	} while(index != -1);
}

}
