// XLang
// -- A parser framework for language modeling
// Copyright (C) 2011 onlyuser <mailto:onlyuser@gmail.com>
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
#include <string> // std::string
#include <sstream> // std::stringstream

namespace xl { namespace node { class NodeIdentIFace; } }

struct ScannerContext
{
    const char* m_buf;    // buffer we read from
    int         m_pos;    // current position in buf
    int         m_length; // length of buf

    ScannerContext(const char* buf);
};

// context type to hold shared data between bison and flex
class ParserContext
{
public:
    ParserContext(xl::Allocator &alloc, const char* buf)
        : m_tree_context(alloc), m_scanner_context(buf)
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
int yylex();
int yylex_destroy();
int yyparse();
void yyerror(const char* s);

std::stringstream &error_messages();
std::string id_to_name(uint32_t lexer_id);
ParserContext* &parser_context();

xl::node::NodeIdentIFace* make_ast(xl::Allocator &alloc, char* s);

#endif
