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

//%output="XLang.tab.c"
%name-prefix="_XLANG_"

%{

#include "XLangType.h" // char*
#include "XLang.h" // node::NodeBase (owner)
#include "XLang.tab.h" // ID_XXX (generated code)
#include "XLangAlloc.h" // Allocator
#include "XLangView.h" // View
#include "XLangModel.h" // mvc::Model
#include <stdio.h> // size_t
#include <stdarg.h> // va_start
#include <string.h> // strlen
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout

// report error
void _XLANG_error(ParseContext* pc, yyscan_t Scanner, const char* s)
{
    errors() << s;
}
void _XLANG_error(const char* s)
{
    _XLANG_error(NULL, NULL, s);
}

// get resource
std::stringstream &errors()
{
    static std::stringstream _errors;
    return _errors;
}
const char* sym_name(uint32 sym_id)
{
    static const char* _sym_name[ID_COUNT - ID_BASE - 1] = {
        "ID_FLOAT",
        "ID_IDENT"
        };
    switch (sym_id)
    {
        case '+': return "+";
        case '-': return "-";
        case '*': return "*";
        case '/': return "/";
        case '=': return "=";
        case ',': return ",";
    }
    return _sym_name[sym_id - ID_BASE - 1];
}

%}

// 'pure_parser' tells bison to use no global variables and create a
// reentrant parser.
//
%pure_parser
%parse-param {ParseContext* pc}
%parse-param {yyscan_t scanner}
%lex-param   {scanner}

// show detailed parse errors
%error-verbose

%nonassoc ID_BASE

%token<value> ID_FLOAT
%token<name> ID_IDENT
%type<node> program statement expression

%left '+' '-'
%left '*' '/'

%nonassoc ID_COUNT

%%

root:
      program { pc->root().node = $1; }
    | error   { yyclearin; /* yyerrok; YYABORT; */ }
    ;

program:
      statement             { $$ = $1; }
    | statement ',' program { $$ = mvc::Model::make_inner(pc, ',', 2, $1, $3); }
    ;

statement:
      expression              { $$ = $1; }
    | ID_IDENT '=' expression { $$ = mvc::Model::make_inner(pc, '=', 2,
                                        mvc::Model::make_ident(pc, ID_IDENT, $1), $3); }
    ;

expression:
      ID_FLOAT                  { $$ = mvc::Model::make_value(pc, ID_FLOAT, $1); }
    | ID_IDENT                  { $$ = mvc::Model::make_ident(pc, ID_IDENT, $1); }
    | expression '+' expression { $$ = mvc::Model::make_inner(pc, '+', 2, $1, $3); }
    | expression '-' expression { $$ = mvc::Model::make_inner(pc, '-', 2, $1, $3); }
    | expression '*' expression { $$ = mvc::Model::make_inner(pc, '*', 2, $1, $3); }
    | expression '/' expression { $$ = mvc::Model::make_inner(pc, '/', 2, $1, $3); }
    | '(' expression ')'        { $$ = $2; }
    ;

%%

ScanContext::ScanContext(char* buf)
    : m_buf(buf), m_pos(0), m_length(strlen(buf))
{
}

node::NodeBase* make_ast(Allocator &alloc, char* s)
{
    ParseContext pc(alloc, s);
    yyscan_t scanner = pc.scan_context().mcanner;
    _XLANG_lex_init(&scanner);
    _XLANG_set_extra(&pc, scanner);
    int error = _XLANG_parse(&pc, scanner); // parser entry point
    _XLANG_lex_destroy(scanner);
    return ((0 == error) && errors().str().empty()) ? (node::NodeBase*) pc.root().node : NULL;
}

int main(int argc, char** argv)
{
    if (2 != argc)
    {
        std::cout << "ERROR: requires 1 argument" << std::endl;
        return 1;
    }
    Allocator alloc(__FILE__);
    node::NodeBase* node = make_ast(alloc, argv[1]);
    if (NULL == node)
    {
        std::cout << argv[1] << std::endl << errors().str().c_str() << std::endl;
        return 1;
    }
    std::cout << "PARSE: ";
    mvc::View::print_lisp(node); std::cout << std::endl;
    std::cout << "GRAPH:";
    mvc::View::print_graph(node);
    std::cout << std::endl;
    alloc.dump();
    return 0;
}
