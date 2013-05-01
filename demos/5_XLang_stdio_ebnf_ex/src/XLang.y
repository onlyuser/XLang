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

// EBNF-EXPANDED:
#include <vector> // std::vector
#include <tuple> // std::tuple

// NOTE: no way to automatically detect this dependency, the user must manually pull it up front
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace

// EBNF-EXPANDED:
typedef std::tuple<xl::node::TermInternalType<xl::node::NodeIdentIFace::SYMBOL>::type, char> program_1_type_t;
typedef std::vector<program_1_type_t> program_0_type_t;
typedef std::tuple<xl::node::TermInternalType<xl::node::NodeIdentIFace::IDENT>::type, char> statement_1_type_t;
typedef statement_1_type_t statement_0_type_t;

#include "XLang.h"
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "XLangAlloc.h" // Allocator
#include "mvc/XLangMVCView.h" // mvc::MVCView
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "XLangTreeContext.h" // TreeContext
#include "XLangType.h" // uint32_t
#include <stdio.h> // size_t
#include <stdarg.h> // va_start
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout
#include <stdlib.h> // EXIT_SUCCESS
#include <getopt.h> // getopt_long

#define MAKE_TERM(lexer_id, ...)   xl::mvc::MVCModel::make_term(tree_context(), lexer_id, ##__VA_ARGS__)
#define MAKE_SYMBOL(...)           xl::mvc::MVCModel::make_symbol(tree_context(), ##__VA_ARGS__)
#define ERROR_LEXER_ID_NOT_FOUND   "missing lexer id handler, most likely you forgot to register one"
#define ERROR_LEXER_NAME_NOT_FOUND "missing lexer name handler, most likely you forgot to register one"

// report error
void _XLANG_error(const char* s)
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
        case ID_UMINUS: return "uminus";
        case '+':       return "+";
        case '-':       return "-";
        case '*':       return "*";
        case '/':       return "/";
        case '=':       return "=";
        case ',':       return ",";
    }
    throw ERROR_LEXER_ID_NOT_FOUND;
    return "";
}
uint32_t name_to_id(std::string name)
{
    if(name == "int")    return ID_INT;
    if(name == "float")  return ID_FLOAT;
    if(name == "ident")  return ID_IDENT;
    if(name == "uminus") return ID_UMINUS;
    if(name == "+")      return '+';
    if(name == "-")      return '-';
    if(name == "*")      return '*';
    if(name == "/")      return '/';
    if(name == "=")      return '=';
    if(name == ",")      return ',';
    throw ERROR_LEXER_NAME_NOT_FOUND;
    return 0;
}
xl::TreeContext* &tree_context()
{
    static xl::TreeContext* tc = NULL;
    return tc;
}

%}

// type of yylval to be set by scanner actions
// implemented as %union in non-reentrant mode
//
%union
{
    xl::node::TermInternalType<xl::node::NodeIdentIFace::INT>::type    int_value;    // int value
    xl::node::TermInternalType<xl::node::NodeIdentIFace::FLOAT>::type  float_value;  // float value
    xl::node::TermInternalType<xl::node::NodeIdentIFace::IDENT>::type  ident_value;  // symbol table index
    xl::node::TermInternalType<xl::node::NodeIdentIFace::SYMBOL>::type symbol_value; // node pointer

    // EBNF-EXPANDED:
    program_1_type_t* program_1_type;
    program_0_type_t* program_0_type;
    statement_1_type_t* statement_1_type;
    statement_0_type_t* statement_0_type;
}

// show detailed parse errors
%error-verbose

%nonassoc ID_BASE

%token<int_value>   ID_INT
%token<float_value> ID_FLOAT
%token<ident_value> ID_IDENT
%type<symbol_value> program statement expression

// EBNF-EXPANDED:
%type<program_1_type> program_1
%type<program_0_type> program_0
%type<statement_1_type> statement_1
%type<statement_0_type> statement_0

%left '+' '-'
%left '*' '/'
%nonassoc ID_UMINUS

%%

root:
      program { tree_context()->root() = $1; YYACCEPT; }
    | error   { yyclearin; /* yyerrok; YYABORT; */ }
    ;

//=============================================================================
// ORIGINAL:
//program:
//      statement             { $$ = $1; }
//    | program ',' statement { $$ = MAKE_SYMBOL(',', 2, $1, $3); }
//    ;

// EBNF:
//program:
//      (
//            statement ',' { /* AAA */ $$ = new program_1_type_t($1, ','); }
//      )* statement {
//              /* BBB */
//              if(!$1->empty())
//              {
//                  auto symbol_node = MAKE_SYMBOL(',', 0);
//                  for(auto p = $1->begin(); p != $1->end(); p++)
//                      symbol_node->push_back(std::get<0>(*p));
//                  symbol_node->push_back($2);
//                  $$ = symbol_node;
//              }
//              else
//                  $$ = $2;
//          }
//    ;

// EBNF-XML:
//<symbol type="rule">                   // <-- rule_node
//    <term type="ident" value=program/> // <-- lhs body
//    <symbol type="alts">               // <-- rhs body
//        <symbol type="alt">
//            <symbol type="terms">
//                <symbol type="*">            // <-- isolate this part as "program_0"
//                    <symbol type="(">        // <-- paren_node
//                        <symbol type="alts"> // <-- alts_node
//                            <symbol type="alt">
//                                <symbol type="terms">
//                                    <term type="ident" value=statement/>
//                                    <term type="char" value=','/>
//                                </symbol>
//                                <symbol type="action_block">
//                                    <term type="string" value=" /* AAA */ ... "/>
//                                </symbol>
//                            </symbol>
//                        </symbol>
//                    </symbol>
//                </symbol>
//                <term type="ident" value=statement/>
//            </symbol>
//            <symbol type="action_block">
//                <term type="string" value=" /* BBB */ ... "/>
//            </symbol>
//        </symbol>
//    </symbol>
//</symbol>

// EBNF-EXPANDED:
// [
program:
      program_0 statement {
                /* BBB */
                if(!$1->empty())
                {
                    auto symbol_node = MAKE_SYMBOL(',', 0);
                    for(auto p = $1->begin(); p != $1->end(); p++)
                        symbol_node->push_back(std::get<0>(*p));
                    symbol_node->push_back($2);
                    $$ = symbol_node;
                }
                else
                    $$ = $2;

                // EBNF-EXPANDED:
                delete $1;
            }
    ;

program_0:
      /* empty */         { /* PPP */ $$ = new program_0_type_t; }
    | program_0 program_1 { /* QQQ */ $1->push_back(*$2); delete $2; $$ = $1; }
    ;

program_1:
      statement ',' { /* AAA */ $$ = new program_1_type_t($1, ','); }
    ;
// ]

// EBNF-EXPANDED-XML:
// [
//<symbol type="rule">
//    <term type="ident" value=program/>
//    <symbol type="alts">
//        <symbol type="alt">
//            <symbol type="terms">
//                <term type="ident" value=program_0/> // <-- replaced kleene closure '*'
//                <term type="ident" value=statement/>
//            </symbol>
//            <symbol type="action_block">
//                <term type="string" value=" /* BBB */ ... "/>
//            </symbol>
//        </symbol>
//    </symbol>
//</symbol>
//<symbol type="rule">
//    <term type="ident" value=program_0/>
//    <symbol type="alts">
//        <symbol type="alt">
//            <symbol type="action_block">
//                <term type="string" value=" /* PPP */ ... "/>
//            </symbol>
//        </symbol>
//        <symbol type="alt">
//            <symbol type="terms">
//                <term type="ident" value=program_0/>
//                <term type="ident" value=program_1/>
//            </symbol>
//            <symbol type="action_block">
//                <term type="string" value=" /* QQQ */ ... "/>
//            </symbol>
//        </symbol>
//    </symbol>
//</symbol>
//<symbol type="rule">
//    <term type="ident" value=program_1/>
//    <symbol type="alts">
//        <symbol type="alt">
//            <symbol type="terms">
//                <term type="ident" value=statement/>
//                <term type="char" value=','/>
//            </symbol>
//            <symbol type="action_block">
//                <term type="string" value=" /* AAA */ ... "/>
//            </symbol>
//        </symbol>
//    </symbol>
//</symbol>
// ]

//=============================================================================
// ORIGINAL:
//statement:
//      expression              { $$ = $1; }
//    | ID_IDENT '=' expression { $$ = MAKE_SYMBOL('=', 2, MAKE_TERM(ID_IDENT, $1), $3); }
//    ;

// EBNF:
//statement:
//      (
//            ID_IDENT '=' { /* CCC */ $$ = new statement_1_type_t($1, '='); }
//      )? expression      { /* DDD */ $$ = $1 ? MAKE_SYMBOL('=', 2, MAKE_TERM(ID_IDENT, std::get<0>(*$1)), $2) : $2; }
//    ;

// EBNF-XML:
//<symbol type="rule">                     // <-- rule_node
//    <term type="ident" value=statement/> // <-- lhs body
//    <symbol type="alts">                 // <-- rhs body
//        <symbol type="alt">
//            <symbol type="terms">
//                <symbol type="?">            // <-- isolate this part as "statement_0"
//                    <symbol type="(">        // <-- paren_node
//                        <symbol type="alts"> // <-- alts_node
//                            <symbol type="alt">
//                                <symbol type="terms">
//                                    <term type="ident" value=ID_IDENT/>
//                                    <term type="char" value='='/>
//                                </symbol>
//                                <symbol type="action_block">
//                                    <term type="string" value=" /* CCC */ ... "/>
//                                </symbol>
//                            </symbol>
//                        </symbol>
//                    </symbol>
//                </symbol>
//                <term type="ident" value=expression/>
//            </symbol>
//            <symbol type="action_block">
//                <term type="string" value=" /* DDD */ ... "/>
//            </symbol>
//        </symbol>
//    </symbol>
//</symbol>

// EBNF-EXPANDED:
// [
statement:
      statement_0 expression {
                /* DDD */ $$ = $1 ? MAKE_SYMBOL('=', 2, MAKE_TERM(ID_IDENT, std::get<0>(*$1)), $2) : $2;

                // EBNF-EXPANDED:
                if($1)
                    delete $1;
            }
    ;

statement_0:
      /* empty */ { /* RRR */ $$ = NULL; } // <== shift-reduce conflict
    | statement_1 { /* SSS */ $$ = $1; }
    ;

statement_1:
      ID_IDENT '=' { /* CCC */ $$ = new statement_1_type_t($1, '='); }
    ;
// ]

// // EBNF-EXPANDED (fixes conflict, but less intuitive):
// // [
// statement:
//       statement_1 expression { /* DDD_1 */ $$ = MAKE_SYMBOL('=', 2, MAKE_TERM(ID_IDENT, std::get<0>(*$1)), $2); }
//     | expression             { /* DDD_2 */ $$ = $1; }
//     ;
//
// statement_1:
//       ID_IDENT '=' { /* CCC */ $$ = new statement_1_type_t($1, '='); }
//     ;
// // ]

// EBNF-EXPANDED-XML:
// [
//<symbol type="rule">
//    <term type="ident" value=statement/>
//    <symbol type="alts">
//        <symbol type="alt">
//            <symbol type="terms">
//                <term type="ident" value=statement_0/> // <-- replaced kleene closure '?'
//                <term type="ident" value=expression/>
//            </symbol>
//            <symbol type="action_block">
//                <term type="string" value=" /* DDD */ ... "/>
//            </symbol>
//        </symbol>
//    </symbol>
//</symbol>
//<symbol type="rule">
//    <term type="ident" value=statement_0/>
//    <symbol type="alts">
//        <symbol type="alt">
//            <symbol type="action_block">
//                <term type="string" value=" /* RRR */ ... "/>
//            </symbol>
//        </symbol>
//        <symbol type="alt">
//            <symbol type="terms">
//                <term type="ident" value=statement_1/>
//            </symbol>
//            <symbol type="action_block">
//                <term type="string" value=" /* SSS */ ... "/>
//            </symbol>
//        </symbol>
//    </symbol>
//</symbol>
//<symbol type="rule">
//    <term type="ident" value=statement_1/>
//    <symbol type="alts">
//        <symbol type="alt">
//            <symbol type="terms">
//                <term type="ident" value=ID_IDENT/>
//                <term type="char" value='='/>
//            </symbol>
//            <symbol type="action_block">
//                <term type="string" value=" /* CCC */ ... "/>
//            </symbol>
//        </symbol>
//    </symbol>
//</symbol>
// ]

expression:
      ID_INT                         { $$ = MAKE_TERM(ID_INT, $1); }
    | ID_FLOAT                       { $$ = MAKE_TERM(ID_FLOAT, $1); }
    | ID_IDENT                       { $$ = MAKE_TERM(ID_IDENT, $1); } // <== shift-reduce conflict
    | '-' expression %prec ID_UMINUS { $$ = MAKE_SYMBOL(ID_UMINUS, 1, $2); }
    | expression '+' expression      { $$ = MAKE_SYMBOL('+', 2, $1, $3); }
    | expression '-' expression      { $$ = MAKE_SYMBOL('-', 2, $1, $3); }
    | expression '*' expression      { $$ = MAKE_SYMBOL('*', 2, $1, $3); }
    | expression '/' expression      { $$ = MAKE_SYMBOL('/', 2, $1, $3); }
    | '(' expression ')'             { $$ = $2; }
    ;

%%

xl::node::NodeIdentIFace* make_ast(xl::Allocator &alloc)
{
    tree_context() = new (PNEW(alloc, xl::, TreeContext))
            xl::TreeContext(alloc);
    int error_code = _XLANG_parse(); // parser entry point
    _XLANG_lex_destroy();
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
                << "  -i, --in-xml=FILE (de-serialize from xml)" << std::endl
                << std::endl
                << "Output control:" << std::endl
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
    std::string in_xml;
    bool dump_memory;

    args_t()
        : mode(MODE_NONE), dump_memory(false)
    {}
};

bool parse_args(int argc, char** argv, args_t &args)
{
    int opt = 0;
    int longIndex = 0;
    static const char *optString = "i:lxgdmh?";
    static const struct option longOpts[] = {
                { "in-xml", required_argument, NULL, 'i' },
                { "lisp",   no_argument,       NULL, 'l' },
                { "xml",    no_argument,       NULL, 'x' },
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
            case 'i': args.in_xml = optarg; break;
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

bool import_ast(args_t &args, xl::Allocator &alloc, xl::node::NodeIdentIFace* &ast)
{
    if(args.in_xml != "")
    {
        ast = xl::mvc::MVCModel::make_ast(
                new (PNEW(alloc, xl::, TreeContext)) xl::TreeContext(alloc),
                args.in_xml);
        if(!ast)
        {
            std::cout << "de-serialize from xml fail!" << std::endl;
            return false;
        }
    }
    else
    {
        ast = make_ast(alloc);
        if(!ast)
        {
            std::cout << error_messages().str().c_str() << std::endl;
            return false;
        }
    }
    return true;
}

void export_ast(args_t &args, const xl::node::NodeIdentIFace* ast)
{
    switch(args.mode)
    {
        case args_t::MODE_LISP:  xl::mvc::MVCView::print_lisp(ast); break;
        case args_t::MODE_XML:   xl::mvc::MVCView::print_xml(ast); break;
        case args_t::MODE_GRAPH: xl::mvc::MVCView::print_graph(ast); break;
        case args_t::MODE_DOT:   xl::mvc::MVCView::print_dot(ast); break;
        default:
            break;
    }
}

bool do_work(args_t &args)
{
    try
    {
        if(args.mode == args_t::MODE_HELP)
        {
            display_usage(true);
            return true;
        }
        xl::Allocator alloc(__FILE__);
        xl::node::NodeIdentIFace* ast = NULL;
        if(!import_ast(args, alloc, ast))
            return false;
        export_ast(args, ast);
        if(args.dump_memory)
            alloc.dump(std::string(1, '\t'));
    }
    catch(const char* s)
    {
        std::cout << "ERROR: " << s << std::endl;
        return false;
    }
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
