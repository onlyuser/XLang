// XLang
// -- A parser framework for language modeling
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
#include <string> // std::string
#include <sstream> // std::stringstream

namespace xl { namespace node { class NodeIdentIFace; } }
namespace xl { class TreeContext; }

// forward declaration of lexer/parser functions
// so the compiler shuts up about warnings
//
int _XLANG_lex();
int _XLANG_lex_destroy();
int _XLANG_parse();
void _XLANG_error(const char* s);

std::stringstream &error_messages();
std::string id_to_name(uint32_t lexer_id);
xl::TreeContext* &tree_context();

xl::node::NodeIdentIFace* make_ast(xl::Allocator &alloc);

#endif
