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

#include "XLang.h" // node::NodeIdentIFace
#include "XLang.tab.h" // ID_XXX (generated code)
#include "XLangAlloc.h" // Allocator
#include "mvc/XLangMVCView.h" // mvc::MVCView
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "node/XLangNodePrinterVisitor.h" // node::NodePrinterVisitor
#include "XLangType.h" // uint32_t
#include <stdio.h> // size_t
#include <stdarg.h> // va_start
#include <string.h> // strlen
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout

#define MAKE_INT(...) mvc::MVCModel::make_leaf<node::NodeIdentIFace::INT>(pc, ID_INT, ##__VA_ARGS__)
#define MAKE_FLOAT(...) mvc::MVCModel::make_leaf<node::NodeIdentIFace::FLOAT>(pc, ID_FLOAT, ##__VA_ARGS__)
#define MAKE_IDENT(...) mvc::MVCModel::make_leaf<node::NodeIdentIFace::IDENT>(pc, ID_IDENT, ##__VA_ARGS__)
#define MAKE_INNER(...) mvc::MVCModel::make_inner(pc, ##__VA_ARGS__)

// report error
void _XLANG_error(ParserContext* pc, yyscan_t scanner, const char* s)
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
std::string sym_name(uint32_t sym_id)
{
    static const char* _sym_name[ID_COUNT - ID_BASE - 1] = {
        "ID_INT",
        "ID_FLOAT",
        "ID_IDENT"
        };
    switch(sym_id)
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
// reentrant parser (NOTE: deprecated, use "%define api.pure" instead).
//
%define api.pure
%parse-param {ParserContext* pc}
%parse-param {yyscan_t scanner}
%lex-param {scanner}

// show detailed parse errors
%error-verbose

%nonassoc ID_BASE

%token<int_value> ID_INT
%token<float_value> ID_FLOAT
%token<ident_value> ID_IDENT
%type<inner_value> program statement expression

%left '+' '-'
%left '*' '/'

%nonassoc ID_COUNT

%%

root:
      program { pc->root().inner_value = $1; }
    | error { yyclearin; /* yyerrok; YYABORT; */ }
    ;

program:
      statement { $$ = $1; }
    | statement ',' program { $$ = MAKE_INNER(',', 2, $1, $3); }
    ;

statement:
      expression { $$ = $1; }
    | ID_IDENT '=' expression { $$ = MAKE_INNER('=', 2, MAKE_IDENT($1), $3); }
    ;

expression:
      ID_INT { $$ = MAKE_INT($1); }
    | ID_FLOAT { $$ = MAKE_FLOAT($1); }
    | ID_IDENT { $$ = MAKE_IDENT($1); }
    | expression '+' expression { $$ = MAKE_INNER('+', 2, $1, $3); }
    | expression '-' expression { $$ = MAKE_INNER('-', 2, $1, $3); }
    | expression '*' expression { $$ = MAKE_INNER('*', 2, $1, $3); }
    | expression '/' expression { $$ = MAKE_INNER('/', 2, $1, $3); }
    | '(' expression ')' { $$ = $2; }
    ;

%%

ScannerContext::ScannerContext(char* buf)
    : m_scanner(NULL), m_buf(buf), m_pos(0), m_length(strlen(buf))
{
}

node::NodeIdentIFace* make_ast(Allocator &alloc, char* s)
{
    ParserContext parser_context(alloc, s);
    yyscan_t scanner = parser_context.scanner_context().m_scanner;
    _XLANG_lex_init(&scanner);
    _XLANG_set_extra(&parser_context, scanner);
    int error = _XLANG_parse(&parser_context, scanner); // parser entry point
    _XLANG_lex_destroy(scanner);
    return ((0 == error) && errors().str().empty()) ? parser_context.root().inner_value : NULL;
}

int main(int argc, char** argv)
{
    if(2 != argc)
    {
        std::cout << "ERROR: requires 1 argument" << std::endl;
        return 1;
    }
    Allocator alloc(__FILE__);
    node::NodeIdentIFace* ast = make_ast(alloc, argv[1]);
    if(NULL == ast)
    {
        std::cout << argv[1] << std::endl << errors().str().c_str() << std::endl;
        return 1;
    }
    std::cout << "LISP: ";
#if 0 // use mvc-pattern pretty-printer
    mvc::MVCView::print_lisp(ast); std::cout << std::endl;
#else // use visitor-pattern pretty-printer
    node::NodePrinterVisitor visitor;
    if(ast->type() == node::NodeIdentIFace::INNER)
    {
        dynamic_cast<const node::InnerNode*>(ast)->accept(&visitor);
        std::cout << std::endl;
    }
    else
        std::cout << "visitor can only print inner-node" << std::endl;
#endif
    std::cout << "GRAPH:";
    mvc::MVCView::print_graph(ast); std::cout << std::endl;
    alloc.dump();
    return 0;
}