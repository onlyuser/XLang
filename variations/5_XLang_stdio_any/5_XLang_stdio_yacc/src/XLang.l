/* Variations of a Flex-Bison parser
 * -- based on "A COMPACT GUIDE TO LEX & YACC" by Tom Niemann
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

#include "XLang.h" // node::NodeIdentIFace
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include <stdlib.h> // atoi

%}

%option noyywrap nounput noyy_top_state
%option stack
%x STATE_COMMENT_BLOCK STATE_COMMENT_LINE STATE_DEFINITION_CODE STATE_RULES STATE_CODE

digit      [0-9]
alpha      [a-zA-Z]
id         {alpha}|[_]
ident      {id}+({digit}|{id})*
_int       {digit}+
_float     {digit}*[.]{digit}+
unnamed    [:|()+*?;]
whitespace [ \t\n]+

%%

"%{"        {
                yy_push_state(STATE_DEFINITION_CODE);
            }
<STATE_DEFINITION_CODE>"%}" {
                yy_pop_state();
            }
<STATE_DEFINITION_CODE>. ;

"%%"        {
                BEGIN STATE_RULES;
            }
<STATE_RULES>{ident} {
                _XLANG_lval.ident_value = tree_context()->alloc_unique_string(yytext);
                return ID_IDENT;
            }
<STATE_RULES>{_int} {
                _XLANG_lval.int_value = atoi(yytext);
                return ID_INT;
            }
<STATE_RULES>{_float} {
                _XLANG_lval.float_value = atof(yytext);
                return ID_FLOAT;
            }
<STATE_RULES>{unnamed} {
                return *yytext;
            }
<STATE_RULES>"%%" {
                BEGIN STATE_CODE;
            }
<STATE_RULES>{whitespace} ;
<STATE_RULES>. {
                char buf[20];
                sprintf(buf, "unknown character: \"%s\"\n", yytext);
                _XLANG_error(buf);
                YY_FLUSH_BUFFER;
                BEGIN INITIAL;
            }

<STATE_CODE>. ;

<*>"/*"                   { yy_push_state(STATE_COMMENT_BLOCK); }
<STATE_COMMENT_BLOCK>"*/" { yy_pop_state(); }
<STATE_COMMENT_BLOCK>.    ;

<*>"//"                  { yy_push_state(STATE_COMMENT_LINE); }
<STATE_COMMENT_LINE>"\n" { yy_pop_state(); }
<STATE_COMMENT_LINE>.    ;

. ;

%%