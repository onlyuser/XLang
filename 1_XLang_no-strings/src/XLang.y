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

#include "XLang.h" // node::NodeBase
#include "XLang.tab.h" // ID_XXX (generated code)
#include "XLangAlloc.h" // Allocator
#include "mvc/XLangMVCView.h" // mvc::MVCView
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "node/XLangNodePrinter.h" // node::NodePrinter
#include "XLangType.h" // uint32
#include <stdio.h> // size_t
#include <stdarg.h> // va_start
#include <string.h> // strlen
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout

// report error
void _XLANG_error(YYLTYPE* loc, ParserContext* pc, yyscan_t scanner, const char* s)
{
    if(NULL != loc)
    {
        std::stringstream ss;
        ss << std::string(loc->first_column-1, '-') <<
                std::string(loc->last_column - loc->first_column + 1, '^') << std::endl <<
                loc->first_line << ":c" << loc->first_column << " to " <<
                loc->last_line << ":c" << loc->last_column << std::endl;
        errors() << ss.str();
    }
    errors() << s;
}
void _XLANG_error(const char* s)
{
    _XLANG_error(NULL, NULL, NULL, s);
}

// get resource
std::stringstream &errors()
{
    static std::stringstream _errors;
    return _errors;
}
std::string sym_name(uint32 sym_id)
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
// reentrant parser.
//
%pure_parser
%parse-param {ParserContext* pc}
%parse-param {yyscan_t scanner}
%lex-param {scanner}

// show detailed parse errors
%error-verbose

// record where each token occurs in input
%locations

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
    | statement ',' program { $$ = mvc::MVCModel::make_inner(pc, ',', @$, 2, $1, $3); }
    ;

statement:
      expression { $$ = $1; }
    | ID_IDENT '=' expression { $$ = mvc::MVCModel::make_inner(pc, '=', @$, 2,
                                     mvc::MVCModel::make_leaf<node::NodeBase::IDENT>(pc, ID_IDENT, @$, $1), $3); }
    ;

expression:
      ID_INT { $$ = mvc::MVCModel::make_leaf<node::NodeBase::INT>(pc, ID_INT, @$, $1); }
    | ID_FLOAT { $$ = mvc::MVCModel::make_leaf<node::NodeBase::FLOAT>(pc, ID_FLOAT, @$, $1); }
    | ID_IDENT { $$ = mvc::MVCModel::make_leaf<node::NodeBase::IDENT>(pc, ID_IDENT, @$, $1); }
    | expression '+' expression { $$ = mvc::MVCModel::make_inner(pc, '+', @$, 2, $1, $3); }
    | expression '-' expression { $$ = mvc::MVCModel::make_inner(pc, '-', @$, 2, $1, $3); }
    | expression '*' expression { $$ = mvc::MVCModel::make_inner(pc, '*', @$, 2, $1, $3); }
    | expression '/' expression { $$ = mvc::MVCModel::make_inner(pc, '/', @$, 2, $1, $3); }
    | '(' expression ')' { $$ = $2; }
    ;

%%

ScannerContext::ScannerContext(char* buf)
    : m_buf(buf), m_pos(0), m_length(strlen(buf)),
      m_line(1), m_column(1), m_prev_column(1)
{
}

node::NodeBase* make_ast(Allocator &alloc, char* s)
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
    node::NodeBase* ast = make_ast(alloc, argv[1]);
    if(NULL == ast)
    {
        std::cout << argv[1] << std::endl << errors().str().c_str() << std::endl;
        return 1;
    }
    std::cout << "LISP: ";
#if 0 // use mvc-pattern pretty-printer
    mvc::MVCView::print_lisp(ast); std::cout << std::endl;
#else // use visitor-pattern pretty-printer
    node::NodePrinter visitor;
    if(ast->type() == node::NodeBase::INNER)
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
