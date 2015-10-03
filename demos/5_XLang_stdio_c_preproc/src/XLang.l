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

#define PUSH_STATE(x) yy_push_state(x)
#define POP_STATE     yy_pop_state()
#define LVAL         _xl(lval)
#define TREE_CONTEXT (*tree_context())

extern FILE* _xl(in);
extern std::string _dirname;

%}

%option noyywrap nounput noyy_top_state
%option stack
%x ST_COMMENT_BLOCK ST_COMMENT_LINE ST_INCLUDE

ws        [ \t\n]+
any       .|[\n]
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

"/*"        {
                PUSH_STATE(ST_COMMENT_BLOCK);
            }
<ST_COMMENT_BLOCK>"*/" {
                POP_STATE;
            }
<ST_COMMENT_BLOCK>{any} ;

"//"        {
                PUSH_STATE(ST_COMMENT_LINE);
            }
<ST_COMMENT_LINE>"\n" {
                POP_STATE;
            }
<ST_COMMENT_LINE>. ;

 /*==========================================================================*/
 /* PREPROC */

"#include"  {
                PUSH_STATE(ST_INCLUDE);
            }
<ST_INCLUDE>[ \t]* ;
<ST_INCLUDE>[^ \t\n]+ {
                std::string filename = _dirname + "/" + yytext;
                _xl(in) = fopen(filename.c_str(), "r");
                if(!_xl(in)) {
                    printf("Error: Can't open include file\n");
                }
                yypush_buffer_state(yy_create_buffer(_xl(in), YY_BUF_SIZE));
                POP_STATE;
            }

 /*==========================================================================*/
 /* LITERALS */

"struct"    {
                return ID_STRUCT;
            }

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

{lb}        {
                return ID_LB;
            }

{rb}        {
                return ID_RB;
            }

{lit_char}  {
                return *yytext;
            }

 /*==========================================================================*/
 /* OTHER */

<<EOF>>     {
                yypop_buffer_state();
                if(!YY_CURRENT_BUFFER) {
                    yyterminate();
                }
            }

.           {
                _xl(error)("unknown character");
                YY_FLUSH_BUFFER;
            }

%%