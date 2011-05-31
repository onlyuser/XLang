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

#include "XLang.h" // NodeBase
#include "XLang.tab.h" // ID_XXX (generated code)
#include <string.h> // memcpy
#include <stdlib.h> // atoi

// When in the lexer you have to access parm through the extra data.
//
#define PARM yyget_extra(yyscanner)->scan_context()

// We want to read from a the buffer in parm so we have to redefine the
// YY_INPUT macro (see section 10 of the flex manual 'The generated scanner')
//
#define YY_INPUT(buffer, res, max_size)                        \
    do {                                                       \
        if(PARM.m_pos >= PARM.m_length)                        \
            (res) = YY_NULL;                                   \
        else {                                                 \
            (res) = PARM.m_length - PARM.m_pos;                \
            (res) > (int) (max_size) ? (res) = (max_size) : 0; \
            memcpy((buffer), PARM.m_buf + PARM.m_pos, (res));  \
            PARM.m_pos += (res);                               \
        }                                                      \
    } while (0)

%}

%option noyywrap
%option nounput
%option reentrant bison-bridge

digit    [0-9]
alpha    [a-zA-Z]
id       {alpha}|[_]
ident    {id}+({digit}|{id})*
_float   {digit}+([.]{digit}+)?
unnamed  [,()+\-*/=]
ignored  [ \t\n]+

%%

{ident}     {
                yylval->ident = yyget_extra(yyscanner)->alloc_unique_string(yytext);
                return ID_IDENT;
            }

{_float}    {
                yylval->_float = atof(yytext);
                return ID_FLOAT;
            }

{unnamed}   {
                return *yytext;
            }

{ignored}   ; // ignore whitespace

.           {
                _XLANG_error("unknown character");
                YY_FLUSH_BUFFER;
            }

%%
