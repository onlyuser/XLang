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

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLangTreeContext.h" // TreeContext

// EBNF-EXPANDED:
#include <vector> // std::vector
#include <tuple> // std::tuple

// EBNF-EXPANDED:
class program_1_type_t;
class program_0_type_t;
class statement_1_type_t;
class statement_0_type_t;

#include "XLang.h" // tree_context
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include <stdlib.h> // atoi

%}

%option noyywrap nounput

ws      [ \t\n]+
literal [,()+\-*/=]
digit   [0-9]
alpha   [a-zA-Z]
id      {alpha}|[_]
ident   {id}+({digit}|{id})*
_int    {digit}+
_float  {digit}*[.]{digit}+

%%

{ws}        ;

 /*==========================================================================*/
 /* PRIMITIVES */

{ident}     {
                _XLANG_lval.ident_value = tree_context()->alloc_unique_string(yytext);
                return ID_IDENT;
            }

{_int}      {
                _XLANG_lval.int_value = atoi(yytext);
                return ID_INT;
            }

{_float}    {
                _XLANG_lval.float_value = atof(yytext);
                return ID_FLOAT;
            }

 /*==========================================================================*/
 /* OTHER */

{literal}   { return *yytext; }

.           {
                _XLANG_error("unknown character");
                YY_FLUSH_BUFFER;
            }

%%