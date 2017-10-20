/* XLang
 * -- A parser framework for language modeling
 * Copyright (C) 2011 onlyuser <mailto:onlyuser@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

%option outfile="lex.XLang.c"

%{

#include "XLang.h"
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLangLexerIDWrapper.h" // ID_XXX (yacc generated)
#include "XLangTreeContext.h" // TreeContext
#include "SymbolTable.h" // SymbolTable
#include <stdlib.h> // atoi
#include <stack> // std::stack

#define LVAL         yylval
#define TREE_CONTEXT (*tree_context())

extern FILE* yyin;
extern std::string _dirname;

std::stack<std::string> _define_stack;

extern "C" int yywrap()
{
    fclose(yyin); // NOTE: for both top and include file
    return 1;
}

%}

%option noyy_top_state
%option stack
%x ST_COMMENT_BLOCK ST_COMMENT_LINE ST_INCLUDE ST_DEFINE

ws        [ \t\n]+
any       .|[\n]
enddef    [\$]
lit_char  [;,()+\-*/^=]
digit     [0-9]
alpha     [a-zA-Z]
id        {alpha}|[_]
lb        [(]
rb        [)]
lit_ident {id}+({digit}|{id})*
lit_int   {digit}+
lit_float {digit}*[.]{digit}+

%%

{ws}        ;

 /*==========================================================================*/
 /* COMMENTS */

"/*"                    { yy_push_state(ST_COMMENT_BLOCK); }
<ST_COMMENT_BLOCK>{any} ;
<ST_COMMENT_BLOCK>"*/"  { yy_pop_state(); }

"//"                  { yy_push_state(ST_COMMENT_LINE); }
<ST_COMMENT_LINE>.    ;
<ST_COMMENT_LINE>"\n" { yy_pop_state(); }

 /*==========================================================================*/
 /* PREPROC INCLUDE */

"#include"            { yy_push_state(ST_INCLUDE); } // begin include
<ST_INCLUDE>[ \t]+    ;                           // white space
<ST_INCLUDE>[^ \t\n]+ {                           // non white space
                std::string filename = yytext;
                if(
                    filename.length() >= 2 && (
                        (filename[0] == '\"' && filename[filename.length()-1] == '\"') ||
                        (filename[0] == '<' && filename[filename.length()-1] == '>')
                        )
                ) {
                    filename = filename.substr(1, filename.length()-2);
                }
                std::string full_filename = _dirname + "/" + filename;
                yyin = fopen(full_filename.c_str(), "r");
                if(!yyin) {
                    yyerror("can't open file");
                    yyterminate();
                }
                yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
                yy_pop_state(); // NOTE: necessary to continue lexing include file
            }
<<EOF>>     { // end include
                if(!yy_buffer_stack_top) { // NOTE: necessary to avoid deleting top buffer
                    return 0;
                }
                yypop_buffer_state();
                if(!YY_CURRENT_BUFFER) {
                    yyterminate();
                }
            }

 /*==========================================================================*/
 /* PREPROC DEFINE */

"#define"               { yy_push_state(ST_DEFINE); }      // begin define
<ST_DEFINE>[ \t]+       ;                               // white space
<ST_DEFINE>[^ \t\n\\]+  { _define_stack.push(yytext); } // non white space (exclude line continuation)
<ST_DEFINE>"\\"[ \t]*\n ;                               // line continuation (include carriage return)
<ST_DEFINE>"\n" {                                       // end define
                yy_pop_state();
                unput('$'); // push end define token to back of input stream
                while(_define_stack.size()) { // unwind stack
                    std::string s = _define_stack.top();
                    for(int i = static_cast<int>(s.length())-1; i >= 0; i--) {
                        unput(s[i]);
                    }
                    if(_define_stack.size() > 1) {
                        unput(' '); // push space between tokens
                    }
                    _define_stack.pop();
                }
                return ID_DEFINE;
            }
{enddef}    { return ID_ENDDEF; } // end define token

 /*==========================================================================*/
 /* PREPROC OTHER */

"#if"       { return ID_IF; } // NOTE: why isn't yyless needed here?
"#ifdef"    { return ID_IFDEF; }
"#ifndef"   { return ID_IFNDEF; }
"#endif"    { return ID_ENDIF; }
"#else"     { return ID_ELSE; }
"#elif"     { return ID_ELIF; }

 /*==========================================================================*/
 /* LITERALS */

"defined"   { return ID_DEFINED; }

"struct"    { return ID_STRUCT; }

"void"      {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_TYPE;
            }

"&&"        { return ID_AND; }

"||"        { return ID_OR; }

{lit_ident} {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                Symbol::type_t type;
                if(SymbolTable::instance()->lookup_symbol(&type, yytext)) {
                    switch(type) {
                        case Symbol::TYPE:        return ID_TYPE;
                        case Symbol::FUNC:        return ID_FUNC;
                        case Symbol::VAR:         return ID_VAR;
                        case Symbol::PREPROC_SYM: return ID_PREPROC_SYM;
                        default:
                            break;
                    }
                }
                return ID_IDENT;
            }

{lit_int}   {
                LVAL.int_value = atoi(yytext);
                return ID_INT;
            }

{lit_float} {
                LVAL.float_value = atof(yytext);
                return ID_FLOAT;
            }

{lb}        { return ID_LB; }
{rb}        { return ID_RB; }
{lit_char}  { return *yytext; }

 /*==========================================================================*/
 /* OTHER */

.           {
                yyerror("unknown character");
                YY_FLUSH_BUFFER;
            }

%%
