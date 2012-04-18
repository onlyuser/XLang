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
#include "XLang.tab.h" // ID_XXX (yacc generated)
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
#include <stdlib.h> // EXIT_SUCCESS
#include <getopt.h> // getopt_long

#define SIZE_BUF_SMALL 160

#define MAKE_LEAF(sym_id, ...) mvc::MVCModel::make_leaf(parse_context(), sym_id, ##__VA_ARGS__)
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
    | error   { yyclearin; /* yyerrok; YYABORT; */ }
    ;

program:
      statement             { $$ = $1; }
    | statement ',' program { $$ = MAKE_INNER(',', 2, $1, $3); }
    ;

statement:
      expression              { $$ = $1; }
    | ID_IDENT '=' expression { $$ = MAKE_INNER('=', 2, MAKE_LEAF(ID_IDENT, $1), $3); }
    ;

expression:
      ID_INT                    { $$ = MAKE_LEAF(ID_INT, $1); }
    | ID_FLOAT                  { $$ = MAKE_LEAF(ID_FLOAT, $1); }
    | ID_IDENT                  { $$ = MAKE_LEAF(ID_IDENT, $1); }
    | expression '+' expression { $$ = MAKE_INNER('+', 2, $1, $3); }
    | expression '-' expression { $$ = MAKE_INNER('-', 2, $1, $3); }
    | expression '*' expression { $$ = MAKE_INNER('*', 2, $1, $3); }
    | expression '/' expression { $$ = MAKE_INNER('/', 2, $1, $3); }
    | '(' expression ')'        { $$ = $2; }
    ;

%%

ScannerContext::ScannerContext(FILE* file)
    : m_file(file), m_pos(0)
{
    fseek(file, 0, SEEK_END);
    m_length = ftell(file);
    rewind(file);
}

const std::string* ParserContext::alloc_unique_string(std::string name)
{
    string_set_t::iterator p = m_string_set.find(&name);
    if(p == m_string_set.end())
    {
        m_string_set.insert(new (m_alloc, __FILE__, __LINE__, [](void *x) {
                reinterpret_cast<std::string*>(x)->~basic_string();
                }) std::string(name));
        p = m_string_set.find(&name);
    }
    return *p;
}

node::NodeIdentIFace* make_ast(Allocator &alloc, FILE* file)
{
    parse_context() = new (alloc, __FILE__, __LINE__, [](void* x) {
            reinterpret_cast<ParserContext*>(x)->~ParserContext();
            }) ParserContext(alloc, file);
    int error = _XLANG_parse(); // parser entry point
    return ((0 == error) && errors().str().empty()) ? parse_context()->root() : NULL;
}

void display_usage(bool verbose)
{
    std::cout << "Usage: XLang OPTION [-m] -f FILE" << std::endl;
    if(verbose)
        std::cout << "Parses input and prints a syntax tree to standard out" << std::endl
                << "Output control:" << std::endl
                << "  -f, --file" << std::endl
                << "  -l, --lisp" << std::endl
                << "  -g, --graph" << std::endl
                << "  -d, --dot" << std::endl
                << "  -m, --memory" << std::endl;
    else
        std::cout << "Try `XLang --help\' for more information." << std::endl;
}

struct args_t
{
    typedef enum
    {
        MODE_NONE,
        MODE_LISP,
        MODE_GRAPH,
        MODE_DOT,
        MODE_HELP
    } mode_e;

    mode_e mode;
    std::string filename;
    bool dump_memory;

    args_t()
        : mode(MODE_NONE), dump_memory(false) {}
};

bool parse_args(int argc, char** argv, args_t &args)
{
    int opt = 0;
    int longIndex = 0;
    static const char *optString = "flgdmh?";
    static const struct option longOpts[] = {
                { "file",   required_argument, NULL, 'f' },
                { "lisp",   no_argument,       NULL, 'l' },
                { "graph",  no_argument,       NULL, 'g' },
                { "dot",    no_argument,       NULL, 'd' },
                { "memory", no_argument,       NULL, 'm' },
                { "help",   no_argument,       NULL, 'h' },
                { NULL,     no_argument,       NULL, 0 }
            };
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while(opt != -1)
    {
        switch(opt)
        {
            case 'f': args.filename = optarg; break;
            case 'l': args.mode = args_t::MODE_LISP; break;
            case 'g': args.mode = args_t::MODE_GRAPH; break;
            case 'd': args.mode = args_t::MODE_DOT; break;
            case 'm': args.dump_memory = true; break;
            case 'h':
            case '?': args.mode = args_t::MODE_HELP; break;
            case 0: // reserved
            default:
                break;
        }
        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }
    if(args_t::MODE_NONE == args.mode)
    {
        display_usage(false);
        return false;
    }
    return true;
}

bool do_work(args_t &args)
{
    Allocator alloc(__FILE__);
    FILE* file = fopen(args.filename.c_str(), "rb");
    if(NULL == file)
    {
        std::cout << "cannot open file" << std::endl;
        return false;
    }
    node::NodeIdentIFace* ast = make_ast(alloc, file);
    fclose(file);
    if(NULL == ast)
    {
        std::cout << errors().str().c_str() << std::endl;
        return false;
    }
    switch(args.mode)
    {
        case args_t::MODE_LISP:
            {
                #if 1 // use mvc-pattern pretty-printer
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
            }
            break;
        case args_t::MODE_GRAPH: mvc::MVCView::print_graph(ast); break;
        case args_t::MODE_DOT:   mvc::MVCView::print_dot(ast); break;
        case args_t::MODE_HELP:  display_usage(true); break;
        default:
            break;
    }
    if(args.dump_memory)
        alloc.dump(std::string(1, '\t'));
    return true;
}

int main(int argc, char** argv)
{
    args_t args;
    if(!parse_args(argc, argv, args))
        return EXIT_FAILURE;
    if(!do_work(args))
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}