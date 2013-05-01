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

#include "XLang.h" // NodeIdentIFace
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include <string.h> // memcpy
#include <stdlib.h> // atoi
#include <algorithm> // std::min

// When in the lexer you have to access parm through the extra data.
//
#define PARM yyget_extra(yyscanner)->scanner_context()

// We want to read from a the buffer in parm so we have to redefine the
// YY_INPUT macro (see section 10 of the flex manual 'The generated scanner')
//
#define YY_INPUT(buf, result, max_size) \
    do { \
        if(PARM.m_pos >= PARM.m_length) \
            (result) = YY_NULL; \
        else { \
            (result) = std::min(PARM.m_length - PARM.m_pos, static_cast<int>(max_size)); \
            memcpy((buf), PARM.m_buf + PARM.m_pos, (result)); \
            PARM.m_pos += (result); \
        } \
    } while(0)

%}

%option noyywrap nounput
%option reentrant bison-bridge

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
                yylval->ident_value = yyget_extra(yyscanner)->tree_context().alloc_unique_string(yytext);
                return ID_IDENT;
            }

{_int}      {
                yylval->int_value = atoi(yytext);
                return ID_INT;
            }

{_float}    {
                yylval->float_value = atof(yytext);
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