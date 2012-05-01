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
#include <stdio.h> // FILE
#include <string> // std::string
#include <set> // std::set
#include <sstream> // std::stringstream

struct ScannerContext
{
    FILE* m_file; // buffer we read from
    int m_pos; // current position in buf
    int m_length; // length of buf

    ScannerContext(FILE* file);
};

// context type to hold shared data between bison and flex
class ParserContext
{
public:
    ParserContext(Allocator &alloc, FILE* file)
        : m_tree_context(alloc), m_scanner_context(file) {}
    TreeContext<> &tree_context() { return m_tree_context; }
    ScannerContext &scanner_context() { return m_scanner_context; }

private:
    TreeContext<> m_tree_context;
    ScannerContext m_scanner_context;
};

// forward declaration of lexer/parser functions
// so the compiler shuts up about warnings
//
int _XLANG_lex();
int _XLANG_lex_destroy();
int _XLANG_parse();
void _XLANG_error(const char* s);

std::stringstream &errors();
std::string sym_name(uint32_t sym_id);
ParserContext* &parser_context();

node::NodeIdentIFace* make_ast(Allocator &alloc, FILE* file);

#endif
