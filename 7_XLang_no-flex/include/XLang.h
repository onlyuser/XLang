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

#include "XLangType.h" // uint32
#include "XLangAlloc.h" // Allocator
#include "node/XLangNodeBase.h" // node::NodeBase
#include "XLangParseContextBase.h" // ParseContextBase
#include <stdio.h> // FILE
#include <string> // std::string
#include <set> // std::set
#include <sstream> // std::stringstream

struct ScanContext
{
    FILE* m_file; // buffer we read from
    int m_pos; // current position in buf
    int m_length; // length of buf

    ScanContext(FILE* file);
};

// context type to hold shared data between bison and flex
class ParseContext : public ParseContextBase
{
private:
    Allocator &m_alloc;
    ScanContext m_sc;
    node::NodeBase* m_root; // parse result (AST root)

    struct str_ptr_compare
    {
        bool operator()(const std::string* s1, const std::string* s2)
        {
         return *s1 < *s2;
        }
    };
    typedef std::set<std::string*, str_ptr_compare> string_set_t;
    string_set_t m_string_set;

public:
    ParseContext(Allocator &alloc, FILE* file)
        : m_alloc(alloc), m_sc(file), m_root(NULL) {}
    Allocator &alloc() { return m_alloc; }
    ScanContext &scan_context() { return m_sc; }
    node::NodeBase* &root() { return m_root; }

    const std::string* alloc_unique_string(std::string name)
    {
        string_set_t::iterator p = m_string_set.find(&name);
        if(p == m_string_set.end())
        {
            m_string_set.insert(new (m_alloc, __FILE__, __LINE__) std::string(name));
            p = m_string_set.find(&name);
        }
        return *p;
    }
};

// forward declaration of lexer/parser functions
// so the compiler shuts up about warnings
//
int _XLANG_lex();
int _XLANG_parse();
void _XLANG_error(const char* s);

std::stringstream &errors();
std::string sym_name(uint32 sym_id);
ParseContext* &parse_context();

node::NodeBase* make_ast(Allocator &alloc, FILE* file);

#endif
