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
#include "XLangTreeContext.h" // TreeContext
#include <stdio.h> // FILE
#include <string> // std::string
#include <sstream> // std::stringstream

namespace xl { namespace node { class NodeIdentIFace; } }

struct ScannerContext
{
    FILE* m_file;   // buffer we read from
    int   m_pos;    // current position in buf
    int   m_length; // length of buf

    ScannerContext(FILE* file);
};

// context type to hold shared data between bison and flex
class ParserContext
{
public:
    ParserContext(xl::Allocator &alloc, FILE* file)
        : m_tree_context(alloc), m_scanner_context(file)
    {}
    xl::TreeContext &tree_context()
    {
        return m_tree_context;
    }
    ScannerContext &scanner_context()
    {
        return m_scanner_context;
    }

private:
    xl::TreeContext m_tree_context;
    ScannerContext  m_scanner_context;
};

// forward declaration of lexer/parser functions
// so the compiler shuts up about warnings
#define _xl(x) _XLANG_##x
int _xl(lex)();
int _xl(lex_destroy)();
int _xl(parse)();
void _xl(error)(const char* s);
//#undef _xl

std::stringstream &error_messages();
std::string id_to_name(uint32_t lexer_id);
ParserContext* &parser_context();

xl::node::NodeIdentIFace* make_ast(xl::Allocator &alloc, FILE* file);

#endif
