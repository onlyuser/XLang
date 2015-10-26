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

//%output="XLang.tab.c"
%name-prefix="_XLANG_"

%{

#include "XLang.h"
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "XLangAlloc.h" // Allocator
#include "mvc/XLangMVCView.h" // mvc::MVCView
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "XLangTreeContext.h" // TreeContext
#include "XLangType.h" // uint32_t
#include "SymbolTable.h" // SymbolTable
#include "NodeEvaluator.h" // NodeEvaluator
#include <stdio.h> // size_t
#include <stdarg.h> // va_start
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout
#include <stdlib.h> // EXIT_SUCCESS
#include <getopt.h> // getopt_long
#include <libgen.h> // dirname

#define MAKE_TERM(lexer_id, ...)   xl::mvc::MVCModel::make_term(tree_context(), lexer_id, ##__VA_ARGS__)
#define MAKE_SYMBOL(...)           xl::mvc::MVCModel::make_symbol(tree_context(), ##__VA_ARGS__)
#define ERROR_LEXER_ID_NOT_FOUND   "missing lexer id handler, most likely you forgot to register one"
#define ERROR_LEXER_NAME_NOT_FOUND "missing lexer name handler, most likely you forgot to register one"

extern FILE* _xl(in);

// report error
void _xl(error)(const char* s)
{
    error_messages() << s;
}

// get resource
std::stringstream &error_messages()
{
    static std::stringstream _error_messages;
    return _error_messages;
}
std::string id_to_name(uint32_t lexer_id)
{
    static const char* _id_to_name[] = {
        "int",
        "float",
        "ident"
        };
    int index = static_cast<int>(lexer_id)-ID_BASE-1;
    if(index >= 0 && index < static_cast<int>(sizeof(_id_to_name)/sizeof(*_id_to_name)))
        return _id_to_name[index];
    switch(lexer_id)
    {
        case ID_UMINUS:         return "uminus";
        case ID_STRUCT:         return "struct";
        case ID_STRUCT_DECL:    return "struct_decl";
        case ID_FUNC_DECL:      return "func_decl";
        case ID_VAR_DECL:       return "var_decl";
        case ID_LB:             return "(";
        case ID_RB:             return ")";
        case ID_DEFINE:         return "#define";
        case ID_DEFINE_SYMBOL:  return "define_symbol";
        case ID_DEFINE_MACRO:   return "define_macro";
        case ID_IDENT_LIST:     return "ident_list";
        case ID_IF:             return "#if";
        case ID_IFDEF:          return "#ifdef";
        case ID_IFNDEF:         return "#ifndef";
        case ID_ENDIF:          return "#endif";
        case ID_ELSE:           return "#else";
        case ID_ELIF:           return "#elif";
        case ID_ELIF_STMT_LIST: return "elif_stmt_list";
        case ID_DEFINED:        return "defined";
        case ID_AND:            return "&&";
        case ID_OR:             return "||";
        case '+':               return "+";
        case '-':               return "-";
        case '*':               return "*";
        case '/':               return "/";
        case '^':               return "^";
        case '=':               return "=";
        case ';':               return ";";
    }
    std::cout << lexer_id << std::endl;
    throw ERROR_LEXER_ID_NOT_FOUND;
    return "";
}
uint32_t name_to_id(std::string name)
{
    if(name == "int")            return ID_INT;
    if(name == "float")          return ID_FLOAT;
    if(name == "ident")          return ID_IDENT;
    if(name == "uminus")         return ID_UMINUS;
    if(name == "struct")         return ID_STRUCT;
    if(name == "struct_decl")    return ID_STRUCT_DECL;
    if(name == "func_decl")      return ID_FUNC_DECL;
    if(name == "var_decl")       return ID_VAR_DECL;
    if(name == "(")              return ID_LB;
    if(name == ")")              return ID_RB;
    if(name == "#define")        return ID_DEFINE;
    if(name == "define_symbol")  return ID_DEFINE_SYMBOL;
    if(name == "define_macro")   return ID_DEFINE_MACRO;
    if(name == "ident_list")     return ID_IDENT_LIST;
    if(name == "#if")            return ID_IF;
    if(name == "#ifdef")         return ID_IFDEF;
    if(name == "#ifndef")        return ID_IFNDEF;
    if(name == "#endif")         return ID_ENDIF;
    if(name == "#else")          return ID_ELSE;
    if(name == "#elif")          return ID_ELIF;
    if(name == "elif_stmt_list") return ID_ELIF_STMT_LIST;
    if(name == "defined")        return ID_DEFINED;
    if(name == "&&")             return ID_AND;
    if(name == "||")             return ID_OR;
    if(name == "+")              return '+';
    if(name == "-")              return '-';
    if(name == "*")              return '*';
    if(name == "/")              return '/';
    if(name == "^")              return '^';
    if(name == "=")              return '=';
    if(name == ";")              return ';';
    throw ERROR_LEXER_NAME_NOT_FOUND;
    return 0;
}
xl::TreeContext* &tree_context()
{
    static xl::TreeContext* tc = NULL;
    return tc;
}

std::string _dirname;

%}

// type of yylval to be set by scanner actions
// implemented as %union in non-reentrant mode
%union
{
    xl::node::TermInternalType<xl::node::NodeIdentIFace::INT>::type    int_value;    // int value
    xl::node::TermInternalType<xl::node::NodeIdentIFace::FLOAT>::type  float_value;  // float value
    xl::node::TermInternalType<xl::node::NodeIdentIFace::IDENT>::type  ident_value;  // symbol table index
    xl::node::TermInternalType<xl::node::NodeIdentIFace::SYMBOL>::type symbol_value; // node pointer
}

// show detailed parse errors
%error-verbose

%nonassoc ID_BASE

%token<int_value>   ID_INT
%token<float_value> ID_FLOAT
%token<ident_value> ID_IDENT ID_TYPE ID_FUNC ID_VAR
%type<symbol_value> block stmt expr
%type<symbol_value> decl struct_decl func_decl var_decl
%type<symbol_value> opt_elif_stmt_list elif_stmt_list elif_stmt
%type<symbol_value> opt_ident_list ident_list

%left ID_AND ID_OR
%left '+' '-'
%left '*' '/'
%left '^'
%nonassoc ID_UMINUS ID_STRUCT ID_STRUCT_DECL ID_FUNC_DECL ID_VAR_DECL ID_LB ID_RB
%nonassoc ID_DEFINE ID_DEFINE_SYMBOL ID_DEFINE_MACRO ID_IDENT_LIST ID_ENDDEF ID_IF ID_IFDEF ID_IFNDEF ID_ENDIF ID_ELSE ID_ELIF ID_ELIF_STMT_LIST ID_DEFINED

%%

root:
      block { tree_context()->root() = $1; YYACCEPT; }
    | error { yyclearin; /* yyerrok; YYABORT; */ }
    ;

block:
      stmt       { $$ = $1; }
    | block stmt { $$ = MAKE_SYMBOL(';', 2, $1, $2); }
    ;

stmt:
      decl ';'                                                     { $$ = $1; }
    | ID_DEFINE ID_IDENT expr ID_ENDDEF                            { $$ = MAKE_SYMBOL(ID_DEFINE_SYMBOL, 2, MAKE_TERM(ID_IDENT, $2), $3); }
    | ID_DEFINE ID_IDENT ID_LB opt_ident_list ID_RB expr ID_ENDDEF { $$ = MAKE_SYMBOL(ID_DEFINE_MACRO, 3, MAKE_TERM(ID_IDENT, $2), $4, $6); }
    | ID_IF expr block ID_ENDIF {
          NodeEvaluator v;
          v.dispatch_visit($2);
          $$ = MAKE_SYMBOL(ID_IF, 2, $2, $3);
      }
    | ID_IFDEF ID_IDENT block ID_ENDIF { $$ = MAKE_SYMBOL(ID_IFDEF, 2, MAKE_TERM(ID_IDENT, $2), $3); }
    | ID_IFDEF ID_IDENT block opt_elif_stmt_list ID_ELSE block ID_ENDIF {
          $$ = MAKE_SYMBOL(ID_IFDEF, 4, MAKE_TERM(ID_IDENT, $2), $3, $4, $6);
      }
    ;

opt_ident_list:
      /*empty*/  { $$ = NULL; }
    | ident_list { $$ = $1; }
    ;

ident_list:
      ID_IDENT                { $$ = MAKE_TERM(ID_IDENT, $1); }
    | ident_list ',' ID_IDENT { $$ = MAKE_SYMBOL(ID_IDENT_LIST, 2, $1, MAKE_TERM(ID_IDENT, $3)); }
    ;

opt_elif_stmt_list:
      /*empty*/      { $$ = NULL; }
    | elif_stmt_list { $$ = $1; }
    ;

elif_stmt_list:
      elif_stmt                { $$ = $1; }
    | elif_stmt_list elif_stmt { $$ = MAKE_SYMBOL(ID_ELIF_STMT_LIST, 2, $1, $2); }
    ;

elif_stmt:
      ID_ELIF expr block { $$ = MAKE_SYMBOL(ID_ELIF, 2, $2, $3); }
    ;

expr:
      ID_INT                          { $$ = MAKE_TERM(ID_INT, $1); }
    | ID_FLOAT                        { $$ = MAKE_TERM(ID_FLOAT, $1); }
    | ID_IDENT                        { $$ = MAKE_TERM(ID_IDENT, $1); }
    | ID_DEFINED ID_LB ID_IDENT ID_RB { $$ = MAKE_SYMBOL(ID_DEFINED, 1, MAKE_TERM(ID_IDENT, $3)); }
    | '-' expr %prec ID_UMINUS        { $$ = MAKE_SYMBOL(ID_UMINUS, 1, $2); }
    | expr ID_AND expr                { $$ = MAKE_SYMBOL(ID_AND, 2, $1, $3); }
    | expr ID_OR expr                 { $$ = MAKE_SYMBOL(ID_OR, 2, $1, $3); }
    | expr '+' expr                   { $$ = MAKE_SYMBOL('+', 2, $1, $3); }
    | expr '-' expr                   { $$ = MAKE_SYMBOL('-', 2, $1, $3); }
    | expr '*' expr                   { $$ = MAKE_SYMBOL('*', 2, $1, $3); }
    | expr '/' expr                   { $$ = MAKE_SYMBOL('/', 2, $1, $3); }
    | expr '^' expr                   { $$ = MAKE_SYMBOL('^', 2, $1, $3); }
    | ID_LB expr ID_RB                { $$ = $2; }
    ;

decl:
      struct_decl { $$ = $1; }
    | func_decl   { $$ = $1; }
    | var_decl    { $$ = $1; }
    ;

struct_decl:
      ID_STRUCT ID_IDENT { // struct g2
          SymbolTable::instance()->add_type(*$2);
          $$ = MAKE_SYMBOL(ID_STRUCT_DECL, 1, MAKE_TERM(ID_IDENT, $2)); }
    ;

func_decl:
      ID_TYPE ID_IDENT ID_LB ID_TYPE ID_LB ID_RB ID_RB { // void f2(g2())
          SymbolTable::instance()->add_func(*$2);
          $$ = MAKE_SYMBOL(ID_FUNC_DECL, 3,
                                         MAKE_TERM(ID_TYPE, $1),
                                         MAKE_TERM(ID_IDENT, $2),
                                         MAKE_TERM(ID_TYPE, $4)); }
    | ID_TYPE ID_IDENT ID_LB ID_TYPE ID_RB { // void g(void)
          SymbolTable::instance()->add_func(*$2);
          $$ = MAKE_SYMBOL(ID_FUNC_DECL, 3,
                                         MAKE_TERM(ID_TYPE, $1),
                                         MAKE_TERM(ID_IDENT, $2),
                                         MAKE_TERM(ID_TYPE, $4)); }
    ;

var_decl:
      ID_TYPE ID_IDENT ID_LB ID_FUNC ID_LB ID_RB ID_RB { // void f(g())
          SymbolTable::instance()->add_var(*$2);
          $$ = MAKE_SYMBOL(ID_VAR_DECL, 3,
                                         MAKE_TERM(ID_TYPE, $1),
                                         MAKE_TERM(ID_IDENT, $2),
                                         MAKE_TERM(ID_FUNC, $4)); }
    ;

%%

xl::node::NodeIdentIFace* make_ast(xl::Allocator &alloc)
{
    tree_context() = new (PNEW(alloc, xl::, TreeContext)) xl::TreeContext(alloc);
    int error_code = _xl(parse)(); // parser entry point
    _xl(lex_destroy)(); // NOTE: necessary to avoid memory leak
    return (!error_code && error_messages().str().empty()) ? tree_context()->root() : NULL;
}

void display_usage(bool verbose)
{
    std::cout << "Usage: XLang [-i] OPTION [-m]" << std::endl;
    if(verbose)
    {
        std::cout << "Parses input and prints a syntax tree to standard out" << std::endl
                << std::endl
                << "Input control:" << std::endl
                << "  -i, --in-xml FILENAME (de-serialize from xml)" << std::endl
                << "  -f, --in-file FILENAME" << std::endl
                << std::endl
                << "Output control:" << std::endl
                << "  -e, --eval" << std::endl
                << "  -l, --lisp" << std::endl
                << "  -x, --xml" << std::endl
                << "  -g, --graph" << std::endl
                << "  -d, --dot" << std::endl
                << "  -m, --memory" << std::endl
                << "  -h, --help" << std::endl;
    }
    else
        std::cout << "Try `XLang --help\' for more information." << std::endl;
}

struct options_t
{
    typedef enum
    {
        MODE_NONE,
        MODE_EVAL,
        MODE_LISP,
        MODE_XML,
        MODE_GRAPH,
        MODE_DOT,
        MODE_HELP
    } mode_e;

    mode_e      mode;
    std::string in_file;
    std::string in_xml;
    bool        dump_memory;

    options_t()
        : mode(MODE_NONE), dump_memory(false)
    {}
};

bool extract_options_from_args(options_t* options, int argc, char** argv)
{
    if(!options)
        return false;
    int opt = 0;
    int longIndex = 0;
    static const char *optString = "i:f:elxgdmh?";
    static const struct option longOpts[] = {
                { "in-xml",  required_argument, NULL, 'i' },
                { "in-file", required_argument, NULL, 'f' },
                { "eval",    no_argument,       NULL, 'e' },
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
            case 'i': options->in_xml = optarg; break;
            case 'f': options->in_file = optarg; break;
            case 'e': options->mode = options_t::MODE_EVAL; break;
            case 'l': options->mode = options_t::MODE_LISP; break;
            case 'x': options->mode = options_t::MODE_XML; break;
            case 'g': options->mode = options_t::MODE_GRAPH; break;
            case 'd': options->mode = options_t::MODE_DOT; break;
            case 'm': options->dump_memory = true; break;
            case 'h':
            case '?': options->mode = options_t::MODE_HELP; break;
            case 0: // reserved
            default:
                break;
        }
        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }
    return options->mode != options_t::MODE_NONE || options->dump_memory;
}

bool import_ast(options_t &options, xl::Allocator &alloc, xl::node::NodeIdentIFace* &ast)
{
    if(options.in_xml.size())
    {
        ast = xl::mvc::MVCModel::make_ast(
                new (PNEW(alloc, xl::, TreeContext)) xl::TreeContext(alloc),
                options.in_xml);
        if(!ast)
        {
            std::cerr << "ERROR: de-serialize from xml fail!" << std::endl;
            return false;
        }
    }
    else
    {
        _xl(in) = fopen(options.in_file.c_str(), "rb"); // NOTE: fclose in yywrap
        if(!_xl(in))
        {
            std::cerr << "ERROR: cannot open file" << std::endl;
            return false;
        }
        _dirname = dirname(const_cast<char*>(options.in_file.c_str()));
        ast = make_ast(alloc);
        if(!ast)
        {
            std::cerr << "ERROR: " << error_messages().str().c_str() << std::endl;
            return false;
        }
    }
    return true;
}

void export_ast(options_t &options, const xl::node::NodeIdentIFace* ast)
{
    switch(options.mode)
    {
        case options_t::MODE_EVAL:  SymbolTable::instance()->print(); break;
        case options_t::MODE_LISP:  xl::mvc::MVCView::print_lisp(ast); break;
        case options_t::MODE_XML:   xl::mvc::MVCView::print_xml(ast); break;
        case options_t::MODE_GRAPH: xl::mvc::MVCView::print_graph(ast); break;
        case options_t::MODE_DOT:   xl::mvc::MVCView::print_dot(ast, true); break;
        default:
            break;
    }
}

bool apply_options(options_t &options)
{
    try
    {
        if(options.mode == options_t::MODE_HELP)
        {
            display_usage(true);
            return true;
        }
        xl::Allocator alloc(__FILE__);
        xl::node::NodeIdentIFace* ast = NULL;
        if(!import_ast(options, alloc, ast))
            return false;
        export_ast(options, ast);
        if(options.dump_memory)
            alloc.dump(std::string(1, '\t'));
    }
    catch(const char* s)
    {
        std::cerr << "ERROR: " << s << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    options_t options;
    if(!extract_options_from_args(&options, argc, argv))
    {
        display_usage(false);
        return EXIT_FAILURE;
    }
    if(!apply_options(options))
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
