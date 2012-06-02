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

#include "EBNFPrinter.h" // visitor::LispPrinter
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "XLangString.h" // xl::escape
#include <iostream> // std::cout

void EBNFPrinter::visit(const xl::node::SymbolNodeIFace* _node)
{
    bool more;
	switch(_node->sym_id())
	{
		case ID_GRAMMAR:
			std::cout << "%%" << std::endl << std::endl;
			xl::visitor::DefaultTour::visit(_node);
			std::cout << "%%" << std::endl;
			break;
		case ID_RULE:
			visit_next_child(_node);
			std::cout << ':' << std::endl;
			visit_next_child(_node);
			std::cout << ';' << std::endl << std::endl;
			break;
		case ID_RULE_RHS:
			std::cout << "\t  ";
			do
			{
				more = visit_next_child(_node);
				if(more)
					std::cout << std::endl << "\t| ";
			} while(more);
			break;
		case ID_ALT:
			more = visit_next_child(_node);
			if(more)
				std::cout << ' '
						<< dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
								_node->operator[](1)
								)->value();
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
			xl::visitor::DefaultTour::visit(_node);
			std::cout << '+';
			break;
		case '*':
			xl::visitor::DefaultTour::visit(_node);
			std::cout << '*';
			break;
		case '?':
			xl::visitor::DefaultTour::visit(_node);
			std::cout << '?';
			break;
		case '(':
			std::cout << '(';
			xl::visitor::DefaultTour::visit(_node);
			std::cout << ')';
			break;
	}
}