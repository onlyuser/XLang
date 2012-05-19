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

#ifndef XLANG_H_
#define XLANG_H_

#include "XLangType.h" // uint32_t
#include "XLangAlloc.h" // Allocator
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLangTreeContext.h" // TreeContext
#include <string> // std::string
#include <set> // std::set
#include <sstream> // std::stringstream

// forward declaration of lexer/parser functions
// so the compiler shuts up about warnings
//
int _XLANG_lex();
int _XLANG_lex_destroy();
int _XLANG_parse();
void _XLANG_error(const char* s);

std::stringstream &errors();
std::string id_to_name(uint32_t sym_id);
xlang::TreeContext* &tree_context();

xlang::node::NodeIdentIFace* make_ast(xlang::Allocator &alloc);

#endif
