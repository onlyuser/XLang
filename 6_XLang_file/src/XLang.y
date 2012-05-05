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
#include "XLangTreeContext.h" // TreeContext
#include "node/XLangNodePrinterVisitor.h" // node::NodePrinterVisitor
#include "XLangType.h" // uint32_t
#include <stdio.h> // size_t
#include <stdarg.h> // va_start
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout
#include <stdlib.h> // EXIT_SUCCESS
#include <getopt.h> // getopt_long

#define MAKE_LEAF(sym_id, ...) mvc::MVCModel::make_leaf(&parser_context()->tree_context(), sym_id, ##__VA_ARGS__)
#define MAKE_INNER(...) mvc::MVCModel::make_inner(&parser_context()->tree_context(), ##__VA_ARGS__)

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
    switch(sym_id)
    {
        case UMINUS: return "negate";
        case '+':    return "+";
        case '-':    return "-";
        case '*':    return "*";
        case '/':    return "/";
        case '=':    return "=";
        case ',':    return ",";
    }
    static const char* _sym_name[ID_COUNT - ID_BASE - 1] = {
        "int",
        "float",
        "ident"
        };
    return _sym_name[sym_id - ID_BASE - 1];
}
uint32_t sym_name_r(std::string name)
{
    if(name == "negate") return UMINUS;
    if(name == "+")      return '+';
    if(name == "-")      return '-';
    if(name == "*")      return '*';
    if(name == "/")      return '/';
    if(name == "=")      return '=';
    if(name == ",")      return ',';
    if(name == "int")    return ID_INT;
    if(name == "float")  return ID_FLOAT;
    if(name == "ident")  return ID_IDENT;
    return 0;
}
ParserContext* &parser_context()
{
    static ParserContext* pc = NULL;
    return pc;
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
%nonassoc UMINUS

%nonassoc ID_COUNT

%%

root:
      program { parser_context()->tree_context().root() = $1; }
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
      ID_INT                      { $$ = MAKE_LEAF(ID_INT, $1); }
    | ID_FLOAT                    { $$ = MAKE_LEAF(ID_FLOAT, $1); }
    | ID_IDENT                    { $$ = MAKE_LEAF(ID_IDENT, $1); }
    | '-' expression %prec UMINUS { $$ = MAKE_INNER(UMINUS, 1, $2); }
    | expression '+' expression   { $$ = MAKE_INNER('+', 2, $1, $3); }
    | expression '-' expression   { $$ = MAKE_INNER('-', 2, $1, $3); }
    | expression '*' expression   { $$ = MAKE_INNER('*', 2, $1, $3); }
    | expression '/' expression   { $$ = MAKE_INNER('/', 2, $1, $3); }
    | '(' expression ')'          { $$ = $2; }
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
    parser_context() = new (alloc, __FILE__, __LINE__, [](void* x) {
            reinterpret_cast<ParserContext*>(x)->~ParserContext();
            }) ParserContext(alloc, file);
    int error = _XLANG_parse(); // parser entry point
    _XLANG_lex_destroy();
    return ((0 == error) && errors().str().empty()) ? parser_context()->tree_context().root() : NULL;
}

void display_usage(bool verbose)
{
    std::cout << "Usage: XLang [-i|-f] OPTION [-m]" << std::endl;
    if(verbose)
    {
        std::cout << "Parses input and prints a syntax tree to standard out" << std::endl
                << std::endl
                << "Input control:" << std::endl
                << "  -i, --in-xml=FILE (de-serialize from xml)" << std::endl
                << "  -f, --in-file=FILE (regular parse)" << std::endl
                << std::endl
                << "Output control:" << std::endl
                << "  -l, --lisp" << std::endl
                << "  -x, --xml" << std::endl
                << "  -g, --graph" << std::endl
                << "  -d, --dot" << std::endl
                << "  -m, --memory" << std::endl;
    }
    else
        std::cout << "Try `XLang --help\' for more information." << std::endl;
}

struct args_t
{
    typedef enum
    {
        MODE_NONE,
        MODE_LISP,
        MODE_XML,
        MODE_GRAPH,
        MODE_DOT,
        MODE_HELP
    } mode_e;

    mode_e mode;
    std::string in_file;
    std::string in_xml;
    bool dump_memory;

    args_t()
        : mode(MODE_NONE), dump_memory(false) {}
};

bool parse_args(int argc, char** argv, args_t &args)
{
    int opt = 0;
    int longIndex = 0;
    static const char *optString = "i:f:lxgdmh?";
    static const struct option longOpts[] = {
                { "in-xml",  required_argument, NULL, 'i' },
                { "in-file", required_argument, NULL, 'f' },
                { "lisp",    no_argument,       NULL, 'l' },
                { "xml",     no_argument,       NULL, 'x' },
                { "graph",   no_argument,       NULL, 'g' },
                { "dot",     no_argument,       NULL, 'd' },
                { "memory",  no_argument,       NULL, 'm' },
                { "help",    no_argument,       NULL, 'h' },
                { NULL,      no_argument,       NULL, 0 }
            };
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while(opt != -1)
    {
        switch(opt)
        {
            case 'i': args.in_xml = optarg; break;
            case 'f': args.in_file = optarg; break;
            case 'l': args.mode = args_t::MODE_LISP; break;
            case 'x': args.mode = args_t::MODE_XML; break;
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
    if(args_t::MODE_NONE == args.mode && !args.dump_memory)
    {
        display_usage(false);
        return false;
    }
    return true;
}

bool do_import(args_t &args, Allocator &alloc, node::NodeIdentIFace* &ast)
{
    if(args.in_xml != "")
    {
        ast = mvc::MVCModel::make_ast(new (alloc, __FILE__, __LINE__, [](void* x) {
                reinterpret_cast<TreeContext*>(x)->~TreeContext();
                }) TreeContext(alloc), args.in_xml);
        if(NULL == ast)
        {
            std::cout << "de-serialize from xml fail!" << std::endl;
            return false;
        }
    }
    else
    {
        FILE* file = fopen(args.in_file.c_str(), "rb");
        if(NULL == file)
        {
            std::cout << "cannot open file" << std::endl;
            return false;
        }
        ast = make_ast(alloc, file);
        fclose(file);
        if(NULL == ast)
        {
            std::cout << errors().str().c_str() << std::endl;
            return false;
        }
    }
    return true;
}

void do_export(args_t &args, node::NodeIdentIFace* ast)
{
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
        case args_t::MODE_XML:   mvc::MVCView::print_xml(ast); break;
        case args_t::MODE_GRAPH: mvc::MVCView::print_graph(ast); break;
        case args_t::MODE_DOT:   mvc::MVCView::print_dot(ast); break;
        default:
            break;
    }
}

bool do_work(args_t &args)
{
    if(args.mode == args_t::MODE_HELP)
    {
        display_usage(true);
        return true;
    }
    Allocator alloc(__FILE__);
    node::NodeIdentIFace* ast = NULL;
    if(!do_import(args, alloc, ast))
        return false;
    do_export(args, ast);
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
