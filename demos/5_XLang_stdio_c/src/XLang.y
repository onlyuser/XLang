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
        "IDENTIFIER", "ID_INT", "ID_FLOAT",
        "CONSTANT", "STRING_LITERAL", "SIZEOF",
        "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP", "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP",
        "AND_OP", "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN",
        "SUB_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN",
        "XOR_ASSIGN", "OR_ASSIGN", "TYPE_NAME",

        "TYPEDEF", "EXTERN", "STATIC", "AUTO", "REGISTER",
        "CHAR", "SHORT", "INT", "LONG", "SIGNED", "UNSIGNED", "FLOAT", "DOUBLE", "CONST", "VOLATILE", "VOID",
        "STRUCT", "UNION", "ENUM", "ELLIPSIS",

        "CASE", "DEFAULT", "IF", "ELSE", "SWITCH", "WHILE", "DO", "FOR", "GOTO", "CONTINUE", "BREAK", "RETURN",
        };
    int index = static_cast<int>(lexer_id)-ID_BASE-1;
    if(index >= 0 && index < static_cast<int>(sizeof(_id_to_name)/sizeof(*_id_to_name)))
        return _id_to_name[index];
    switch(lexer_id)
    {
        case '+': return "+";
        case '-': return "-";
        case '*': return "*";
        case '/': return "/";
        case '%': return "%";
        case '^': return "^";
        case '&': return "&";
        case '|': return "|";
        case '=': return "=";
        case ',': return ",";
    }
    throw ERROR_LEXER_ID_NOT_FOUND;
    return "";
}
uint32_t name_to_id(std::string name)
{
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
}

// show detailed parse errors
%error-verbose

%nonassoc ID_BASE

%type<symbol_value> primary_expression postfix_expression unary_expression unary_operator
        multiplicative_expression additive_expression shift_expression relational_expression
        equality_expression and_expression exclusive_or_expression inclusive_or_expression
        logical_and_expression logical_or_expression conditional_expression assignment_expression
        assignment_operator expression type_specifier statement compound_statement statement_list
        expression_statement

%token<ident_value> IDENTIFIER
%token<int_value>   ID_INT
%token<float_value> ID_FLOAT

%token CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

//%start translation_unit
%start root

%nonassoc ID_COUNT

%%

///////////////////////////////////////////////////////////////////////////////
//root:
//      program { tree_context()->root() = $1; YYACCEPT; }
//    | error   { yyclearin; /* yyerrok; YYABORT; */ }
//    ;
//
//program:
//      statement             { $$ = $1; }
//    | statement ',' program { $$ = MAKE_SYMBOL(',', 2, $1, $3); }
//    ;
//
//statement:
//      expression                { $$ = $1; }
//    | IDENTIFIER '=' expression { $$ = MAKE_SYMBOL('=', 2, MAKE_TERM(IDENTIFIER, $1), $3); }
//    ;
//
//expression:
//      ID_INT                    { $$ = MAKE_TERM(ID_INT, $1); }
//    | ID_FLOAT                  { $$ = MAKE_TERM(ID_FLOAT, $1); }
//    | IDENTIFIER                { $$ = MAKE_TERM(IDENTIFIER, $1); }
//    | expression '+' expression { $$ = MAKE_SYMBOL('+', 2, $1, $3); }
//    | expression '-' expression { $$ = MAKE_SYMBOL('-', 2, $1, $3); }
//    | expression '*' expression { $$ = MAKE_SYMBOL('*', 2, $1, $3); }
//    | expression '/' expression { $$ = MAKE_SYMBOL('/', 2, $1, $3); }
//    | '(' expression ')'        { $$ = $2; }
//    ;
///////////////////////////////////////////////////////////////////////////////

root
    : statement { tree_context()->root() = $1; YYACCEPT; }
    | error     { yyclearin; /* yyerrok; YYABORT; */ }
    ;

primary_expression
    : IDENTIFIER         { $$ = MAKE_TERM(IDENTIFIER, $1); }
    | ID_INT             { $$ = MAKE_TERM(ID_INT, $1); }
    | ID_FLOAT           { $$ = MAKE_TERM(ID_FLOAT, $1); }
    //| STRING_LITERAL
    | '(' expression ')' { $$ = $2; }
    ;

postfix_expression
    : primary_expression
    | postfix_expression '[' expression ']'
    | postfix_expression '(' ')'
    //| postfix_expression '(' argument_expression_list ')'
    | postfix_expression '.' IDENTIFIER
    | postfix_expression PTR_OP IDENTIFIER
    | postfix_expression INC_OP
    | postfix_expression DEC_OP
    ;

//argument_expression_list
//    : assignment_expression
//    | argument_expression_list ',' assignment_expression
//    ;

unary_expression
    : postfix_expression
    | INC_OP unary_expression
    | DEC_OP unary_expression
    //| unary_operator cast_expression
    | SIZEOF unary_expression
    //| SIZEOF '(' type_name ')'
    ;

unary_operator
    //: '&'
    //| '*'
    : '+'
    | '-'
    | '~'
    | '!'
    ;

//cast_expression
//    : unary_expression
//    | '(' type_name ')' cast_expression
//    ;

multiplicative_expression
    : unary_expression
    | multiplicative_expression '*' unary_expression
    | multiplicative_expression '/' unary_expression
    | multiplicative_expression '%' unary_expression
    ;

additive_expression
    : multiplicative_expression
    | additive_expression '+' multiplicative_expression
    | additive_expression '-' multiplicative_expression
    ;

shift_expression
    : additive_expression
    | shift_expression LEFT_OP additive_expression
    | shift_expression RIGHT_OP additive_expression
    ;

relational_expression
    : shift_expression
    | relational_expression '<' shift_expression
    | relational_expression '>' shift_expression
    | relational_expression LE_OP shift_expression
    | relational_expression GE_OP shift_expression
    ;

equality_expression
    : relational_expression
    | equality_expression EQ_OP relational_expression
    | equality_expression NE_OP relational_expression
    ;

and_expression
    : equality_expression
    | and_expression '&' equality_expression
    ;

exclusive_or_expression
    : and_expression
    | exclusive_or_expression '^' and_expression
    ;

inclusive_or_expression
    : exclusive_or_expression
    | inclusive_or_expression '|' exclusive_or_expression
    ;

logical_and_expression
    : inclusive_or_expression
    | logical_and_expression AND_OP inclusive_or_expression
    ;

logical_or_expression
    : logical_and_expression
    | logical_or_expression OR_OP logical_and_expression
    ;

conditional_expression
    : logical_or_expression
    | logical_or_expression '?' expression ':' conditional_expression
    ;

assignment_expression
    : conditional_expression
    | unary_expression assignment_operator assignment_expression
    ;

assignment_operator
    : '='
    | MUL_ASSIGN
    | DIV_ASSIGN
    | MOD_ASSIGN
    | ADD_ASSIGN
    | SUB_ASSIGN
    | LEFT_ASSIGN
    | RIGHT_ASSIGN
    | AND_ASSIGN
    | XOR_ASSIGN
    | OR_ASSIGN
    ;

expression
    : assignment_expression
    | expression ',' assignment_expression
    ;

//constant_expression
//    : conditional_expression
//    ;
//
//declaration
//    : declaration_specifiers ';'
//    | declaration_specifiers init_declarator_list ';'
//    ;
//
//declaration_specifiers
//    : storage_class_specifier
//    | storage_class_specifier declaration_specifiers
//    | type_specifier
//    | type_specifier declaration_specifiers
//    | type_qualifier
//    | type_qualifier declaration_specifiers
//    ;
//
//init_declarator_list
//    : init_declarator
//    | init_declarator_list ',' init_declarator
//    ;
//
//init_declarator
//    : declarator
//    | declarator '=' initializer
//    ;
//
//storage_class_specifier
//    : TYPEDEF
//    | EXTERN
//    | STATIC
//    | AUTO
//    | REGISTER
//    ;

type_specifier
    : VOID
    | CHAR
    | SHORT
    | INT
    | LONG
    | FLOAT
    | DOUBLE
    | SIGNED
    | UNSIGNED
//    | struct_or_union_specifier
//    | enum_specifier
//    | TYPE_NAME
    ;

//struct_or_union_specifier
//    : struct_or_union IDENTIFIER '{' struct_declaration_list '}'
//    | struct_or_union '{' struct_declaration_list '}'
//    | struct_or_union IDENTIFIER
//    ;
//
//struct_or_union
//    : STRUCT
//    | UNION
//    ;
//
//struct_declaration_list
//    : struct_declaration
//    | struct_declaration_list struct_declaration
//    ;
//
//struct_declaration
//    : specifier_qualifier_list struct_declarator_list ';'
//    ;
//
//specifier_qualifier_list
//    : type_specifier specifier_qualifier_list
//    | type_specifier
//    | type_qualifier specifier_qualifier_list
//    | type_qualifier
//    ;
//
//struct_declarator_list
//    : struct_declarator
//    | struct_declarator_list ',' struct_declarator
//    ;
//
//struct_declarator
//    : declarator
//    | ':' constant_expression
//    | declarator ':' constant_expression
//    ;
//
//enum_specifier
//    : ENUM '{' enumerator_list '}'
//    | ENUM IDENTIFIER '{' enumerator_list '}'
//    | ENUM IDENTIFIER
//    ;
//
//enumerator_list
//    : enumerator
//    | enumerator_list ',' enumerator
//    ;
//
//enumerator
//    : IDENTIFIER
//    | IDENTIFIER '=' constant_expression
//    ;
//
//type_qualifier
//    : CONST
//    | VOLATILE
//    ;
//
//declarator
//    : pointer direct_declarator
//    | direct_declarator
//    ;
//
//direct_declarator
//    : IDENTIFIER
//    | '(' declarator ')'
//    | direct_declarator '[' constant_expression ']'
//    | direct_declarator '[' ']'
//    | direct_declarator '(' parameter_type_list ')'
//    | direct_declarator '(' identifier_list ')'
//    | direct_declarator '(' ')'
//    ;
//
//pointer
//    : '*'
//    | '*' type_qualifier_list
//    | '*' pointer
//    | '*' type_qualifier_list pointer
//    ;
//
//type_qualifier_list
//    : type_qualifier
//    | type_qualifier_list type_qualifier
//    ;
//
//
//parameter_type_list
//    : parameter_list
//    | parameter_list ',' ELLIPSIS
//    ;
//
//parameter_list
//    : parameter_declaration
//    | parameter_list ',' parameter_declaration
//    ;
//
//parameter_declaration
//    : declaration_specifiers declarator
//    | declaration_specifiers abstract_declarator
//    | declaration_specifiers
//    ;
//
//identifier_list
//    : IDENTIFIER
//    | identifier_list ',' IDENTIFIER
//    ;
//
//type_name
//    : specifier_qualifier_list
//    | specifier_qualifier_list abstract_declarator
//    ;
//
//abstract_declarator
//    : pointer
//    | direct_abstract_declarator
//    | pointer direct_abstract_declarator
//    ;
//
//direct_abstract_declarator
//    : '(' abstract_declarator ')'
//    | '[' ']'
//    | '[' constant_expression ']'
//    | direct_abstract_declarator '[' ']'
//    | direct_abstract_declarator '[' constant_expression ']'
//    | '(' ')'
//    | '(' parameter_type_list ')'
//    | direct_abstract_declarator '(' ')'
//    | direct_abstract_declarator '(' parameter_type_list ')'
//    ;
//
//initializer
//    : assignment_expression
//    | '{' initializer_list '}'
//    | '{' initializer_list ',' '}'
//    ;
//
//initializer_list
//    : initializer
//    | initializer_list ',' initializer
//    ;

statement
//    : labeled_statement
    : compound_statement
    | expression_statement
//    | selection_statement
//    | iteration_statement
//    | jump_statement
    ;

//labeled_statement
//    : IDENTIFIER ':' statement
//    | CASE constant_expression ':' statement
//    | DEFAULT ':' statement
//    ;

compound_statement
    : '{' '}'
    | '{' statement_list '}'
//    | '{' declaration_list '}'
//    | '{' declaration_list statement_list '}'
    ;

//declaration_list
//    : declaration
//    | declaration_list declaration
//    ;

statement_list
    : statement
    | statement_list statement
    ;

expression_statement
    : ';'
    | expression ';'
    ;

//selection_statement
//    : IF '(' expression ')' statement
//    | IF '(' expression ')' statement ELSE statement
//    | SWITCH '(' expression ')' statement
//    ;
//
//iteration_statement
//    : WHILE '(' expression ')' statement
//    | DO statement WHILE '(' expression ')' ';'
//    | FOR '(' expression_statement expression_statement ')' statement
//    | FOR '(' expression_statement expression_statement expression ')' statement
//    ;
//
//jump_statement
//    : GOTO IDENTIFIER ';'
//    | CONTINUE ';'
//    | BREAK ';'
//    | RETURN ';'
//    | RETURN expression ';'
//    ;
//
//translation_unit
//    : external_declaration
//    | translation_unit external_declaration
//    ;
//
//external_declaration
//    : function_definition
//    | declaration
//    ;
//
//function_definition
//    : declaration_specifiers declarator declaration_list compound_statement
//    | declaration_specifiers declarator compound_statement
//    | declarator declaration_list compound_statement
//    | declarator compound_statement
//    ;

%%

xl::node::NodeIdentIFace* make_ast(xl::Allocator &alloc)
{
    tree_context() = new (PNEW(alloc, xl::, TreeContext)) xl::TreeContext(alloc);
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
