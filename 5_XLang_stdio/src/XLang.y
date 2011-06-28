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
#include "mvc/XLangMVCView.h" // mvc::MVCView
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "node/XLangNodePrinter.h" // node::NodePrinter
#include <stdio.h> // size_t
#include <stdarg.h> // va_start
#include <string.h> // strlen
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout

// report error
void _XLANG_error(const char* s)
{
    errors() << s;
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
ParseContext* &parse_context()
{
    static ParseContext* pc = NULL;
    return pc;
}

%}

// type of yylval to be set by scanner actions
// implemented as %union in non-reentrant mode
//
%union
{
    long int_value; // int value
    float32 float_value; // float value
    const std::string* ident_value; // symbol table index
    node::NodeBase* inner_value; // node pointer
}

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
     program { parse_context()->root() = $1; YYACCEPT; }
    | error { yyclearin; /* yyerrok; YYABORT; */ }
    ;

program:
     statement { $$ = $1; }
    | statement ',' program { $$ = mvc::MVCModel::make_inner(parse_context(), ',', 2, $1, $3); }
    ;

statement:
     expression { $$ = $1; }
    | ID_IDENT '=' expression { $$ = mvc::MVCModel::make_inner(parse_context(), '=', 2,
                                     mvc::MVCModel::make_leaf<node::NodeBase::IDENT>(parse_context(), ID_IDENT, $1), $3); }
    ;

expression:
     ID_INT { $$ = mvc::MVCModel::make_leaf<node::NodeBase::INT>(parse_context(), ID_INT, $1); }
    | ID_FLOAT { $$ = mvc::MVCModel::make_leaf<node::NodeBase::FLOAT>(parse_context(), ID_FLOAT, $1); }
    | ID_IDENT { $$ = mvc::MVCModel::make_leaf<node::NodeBase::IDENT>(parse_context(), ID_IDENT, $1); }
    | expression '+' expression { $$ = mvc::MVCModel::make_inner(parse_context(), '+', 2, $1, $3); }
    | expression '-' expression { $$ = mvc::MVCModel::make_inner(parse_context(), '-', 2, $1, $3); }
    | expression '*' expression { $$ = mvc::MVCModel::make_inner(parse_context(), '*', 2, $1, $3); }
    | expression '/' expression { $$ = mvc::MVCModel::make_inner(parse_context(), '/', 2, $1, $3); }
    | '(' expression ')' { $$ = $2; }
    ;

%%

node::NodeBase* make_ast(Allocator &alloc)
{
    ParseContext* &pc = parse_context();
    pc = new (alloc, __FILE__, __LINE__) ParseContext(alloc);
    int error = _XLANG_parse(); // parser entry point
    return ((0 == error) && errors().str().empty()) ? (node::NodeBase*) pc->root() : NULL;
}

int main(int argc, char** argv)
{
    if(1 != argc)
    {
        std::cout << "ERROR: requires stdin arguments" << std::endl;
        return 1;
    }
    Allocator alloc(__FILE__);
    node::NodeBase* ast = make_ast(alloc);
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
