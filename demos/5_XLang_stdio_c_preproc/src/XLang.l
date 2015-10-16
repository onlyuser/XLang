/* XLang
 * -- A parser framework for language modeling
 * Copyright (C) 2011 Jerry Chen <mailto:onlyuser@gmail.com>
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
%option prefix="_XLANG_"

%{

#include "XLang.h"
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "XLangTreeContext.h" // TreeContext
#include "SymbolTable.h" // SymbolTable
#include <stdlib.h> // atoi
#include <stack> // std::stack

#define PUSH_STATE(x) yy_push_state(x)
#define POP_STATE     yy_pop_state()
#define LVAL         _xl(lval)
#define TREE_CONTEXT (*tree_context())

extern FILE* _xl(in);
extern std::string _dirname;

std::stack<std::string> _define_stack;

extern "C" int _xl(wrap)()
{
    fclose(_xl(in)); // NOTE: both for top and included file
    return 1;
}

%}

%option noyy_top_state
%option stack
%x ST_COMMENT_BLOCK ST_COMMENT_LINE ST_INCLUDE ST_DEFINE

ws        [ \t\n]+
any       .|[\n]
enddef    [\$]
lit_char  [;()+\-*/^=]
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

"/*"                    { PUSH_STATE(ST_COMMENT_BLOCK); }
<ST_COMMENT_BLOCK>{any} ;
<ST_COMMENT_BLOCK>"*/"  { POP_STATE; }

"//"                  { PUSH_STATE(ST_COMMENT_LINE); }
<ST_COMMENT_LINE>.    ;
<ST_COMMENT_LINE>"\n" { POP_STATE; }

 /*==========================================================================*/
 /* PREPROC INCLUDE */

"#include"            { PUSH_STATE(ST_INCLUDE); } // begin include
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
                _xl(in) = fopen(full_filename.c_str(), "r");
                if(!_xl(in)) {
                    _xl(error)("can't open file");
                    yyterminate();
                }
                yypush_buffer_state(yy_create_buffer(_xl(in), YY_BUF_SIZE));
                POP_STATE;
            }
<<EOF>>     { // end include
                if(!yy_buffer_stack_top) { // NOTE: avoid deleting top buffer
                    return 0;
                }
                yypop_buffer_state();
                if(!YY_CURRENT_BUFFER) {
                    yyterminate();
                }
            }

 /*==========================================================================*/
 /* PREPROC DEFINE */

"#define"               { PUSH_STATE(ST_DEFINE); }      // begin define
<ST_DEFINE>[ \t]+       ;                               // white space
<ST_DEFINE>[^ \t\n\\]+  { _define_stack.push(yytext); } // non white space (exclude line continuation)
<ST_DEFINE>"\\"[ \t]*\n ;                               // line continuation (include carriage return)
<ST_DEFINE>"\n" {                                       // end define
                POP_STATE;
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
"#elseif"   { return ID_ELSEIF; }

 /*==========================================================================*/
 /* LITERALS */

"struct"    { return ID_STRUCT; }

"void"      {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_TYPE;
            }

{lit_ident} {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                Symbol::type_t type;
                if(SymbolTable::instance()->lookup_symbol(&type, yytext)) {
                    switch(type) {
                        case Symbol::TYPE: return ID_TYPE;
                        case Symbol::FUNC: return ID_FUNC;
                        case Symbol::VAR:  return ID_VAR;
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
                _xl(error)("unknown character");
                YY_FLUSH_BUFFER;
            }

%%