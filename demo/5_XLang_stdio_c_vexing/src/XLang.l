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

#define LVAL         yylval
#define TREE_CONTEXT (*tree_context())

%}

%option noyywrap nounput noyy_top_state
%option stack
%x ST_COMMENT_BLOCK ST_COMMENT_LINE

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
                yy_push_state(ST_COMMENT_BLOCK);
            }
<ST_COMMENT_BLOCK>"*/" {
                yy_pop_state();
            }
<ST_COMMENT_BLOCK>{any} ;

"//"        {
                yy_push_state(ST_COMMENT_LINE);
            }
<ST_COMMENT_LINE>"\n" {
                yy_pop_state();
            }
<ST_COMMENT_LINE>. ;

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

.           {
                yyerror("unknown character");
                YY_FLUSH_BUFFER;
            }

%%
