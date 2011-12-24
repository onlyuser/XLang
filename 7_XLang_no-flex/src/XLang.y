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
#include <stdio.h> // fread
#include <stdarg.h> // va_start
#include <string.h> // memset
#include <ctype.h> // isalpha
#include <stdlib.h> // atoi
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout
#include <algorithm> // std::min

#define SIZE_BUF_SMALL 160

#define MAKE_INT(...) mvc::MVCModel::make_leaf<node::NodeIdentIFace::INT>(parse_context(), ID_INT, ##__VA_ARGS__)
#define MAKE_FLOAT(...) mvc::MVCModel::make_leaf<node::NodeIdentIFace::FLOAT>(parse_context(), ID_FLOAT, ##__VA_ARGS__)
#define MAKE_IDENT(...) mvc::MVCModel::make_leaf<node::NodeIdentIFace::IDENT>(parse_context(), ID_IDENT, ##__VA_ARGS__)
#define MAKE_INNER(...) mvc::MVCModel::make_inner(parse_context(), ##__VA_ARGS__)

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
ParserContext* &parse_context()
{
    static ParserContext* pc = NULL;
    return pc;
}

// When in the lexer you have to access parm through the extra data.
//
#define PARM parse_context()->scanner_context()

// We want to read from a the buffer in parm so we have to redefine the
// YY_INPUT macro (see section 10 of the flex manual 'The generated scanner')
//
#define YY_INPUT(buf, result, max_size) \
    do { \
        if(PARM.m_pos >= PARM.m_length) \
            (result) = 0; \
        else { \
            (result) = std::min(PARM.m_length - PARM.m_pos, static_cast<int>(max_size)); \
            fread((buf), sizeof(char), (result), PARM.m_file); \
            PARM.m_pos += (result); \
        } \
    } while(0)

#define YY_REWIND(n_less) \
    do { \
        if(PARM.m_pos - (n_less) >= 0) { \
            fseek(PARM.m_file, sizeof(char) * -(n_less), SEEK_CUR); \
            PARM.m_pos -= (n_less); \
        } \
    } while(0)

int _XLANG_lex()
{
    char yytext[SIZE_BUF_SMALL];
    memset(yytext, 0, sizeof(yytext));
    int start_pos = PARM.m_pos;
    char* cur_ptr = &yytext[PARM.m_pos - start_pos];
    int bytes_read = 0;
    YY_INPUT(cur_ptr, bytes_read, 1);
    if(0 == bytes_read)
        return -1;
    if(isalpha(*cur_ptr) || *cur_ptr == '_')
    {
        do
        {
            cur_ptr = &yytext[PARM.m_pos - start_pos];
            YY_INPUT(cur_ptr, bytes_read, 1);
        } while(bytes_read != 0 && (isdigit(*cur_ptr) || isalpha(*cur_ptr) || *cur_ptr == '_'));
        if(bytes_read != 0)
        {
            YY_REWIND(1);
            yytext[PARM.m_pos - start_pos] = '\0';
        }
        _XLANG_lval.ident_value = parse_context()->alloc_unique_string(yytext);
        return ID_IDENT;
    }
    else if(isdigit(*cur_ptr))
    {
        bool find_decimal_point = false;
        do
        {
            cur_ptr = &yytext[PARM.m_pos - start_pos];
            YY_INPUT(cur_ptr, bytes_read, 1);
            if(*cur_ptr == '.')
                find_decimal_point = true;
        } while(bytes_read != 0 && (isdigit(*cur_ptr) || *cur_ptr == '.'));
        if(bytes_read != 0)
        {
            YY_REWIND(1);
            yytext[PARM.m_pos - start_pos] = '\0';
        }
        if(find_decimal_point)
        {
            _XLANG_lval.float_value = atof(yytext);
            return ID_FLOAT;
        }
        _XLANG_lval.int_value = atoi(yytext);
        return ID_INT;
    }
    else
        switch(*cur_ptr)
        {
        case ',':
        case '(': case ')':
        case '+': case '-':
        case '*': case '/':
        case '=':
        case '\n':
            return *cur_ptr;
        default:
            _XLANG_error("unknown character");
        }
    return -1;
}

%}

// type of yylval to be set by scanner actions
// implemented as %union in non-reentrant mode
//
%union
{
    long int_value; // int value
    float32_t float_value; // float value
    const std::string* ident_value; // symbol table index
    node::NodeIdentIFace* inner_value; // node pointer
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
      program { parse_context()->root() = $1; }
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

ScannerContext::ScannerContext(FILE* file)
    : m_file(file), m_pos(0)
{
    fseek(file, 0, SEEK_END);
    m_length = ftell(file);
    rewind(file);
}

node::NodeIdentIFace* make_ast(Allocator &alloc, FILE* file)
{
    parse_context() = new (alloc, __FILE__, __LINE__) ParserContext(alloc, file);
    int error = _XLANG_parse(); // parser entry point
    return ((0 == error) && errors().str().empty()) ? parse_context()->root() : NULL;
}

int main(int argc, char** argv)
{
    if(2 > argc)
    {
        std::cout << "ERROR: requires 1 or more filename arguments" << std::endl;
        return 1;
    }
    Allocator alloc(__FILE__);
    for(int i = 1; i < argc; i++)
    {
        FILE* file = fopen(argv[i], "rb");
        if(NULL == file)
            break;
        node::NodeIdentIFace* ast = make_ast(alloc, file);
        fclose(file);
        if(NULL == ast)
        {
            std::cout << argv[1] << std::endl << errors().str().c_str() << std::endl;
            continue;
        }
        std::cout << "INPUT: " << argv[i] << std::endl;
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
    }
    alloc.dump();
    return 0;
}
