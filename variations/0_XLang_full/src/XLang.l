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

#include "XLangType.h" // char*
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
            (result) = std::min(PARM.m_length - PARM.m_pos, std::min(static_cast<int>(max_size), 1)); \
            if('\n' == PARM.m_buf[PARM.m_pos]) { \
                PARM.m_line++; \
                PARM.m_column = 1; \
            } else \
                PARM.m_column++; \
            memcpy((buf), PARM.m_buf + PARM.m_pos, (result)); \
            PARM.m_pos += (result); \
        } \
    } while(0)

void begin_token(yyscan_t yyscanner, size_t length);
char escape_char(char c);
std::string escape_string(std::string s);

%}

%option noyywrap nounput noyy_top_state
%option reentrant bison-bridge bison-locations
%option stack
%x STATE_COMMENT_BLOCK STATE_COMMENT_LINE STATE_STRING STATE_CHAR

digit   [0-9]
alpha   [a-zA-Z]
id      {alpha}|[_]
ident   {id}+({digit}|{id})*
_int    {digit}+
_float  {digit}*[.]{digit}+
unnamed [,()+\-*/=]
ignored [ \t\n]+

%%

{ident}     {
                begin_token(yyscanner, yyleng);
                yylval->ident_value = yyget_extra(yyscanner)->tree_context().alloc_unique_string(yytext);
                return ID_IDENT;
            }

{_int}      {
                begin_token(yyscanner, yyleng);
                yylval->int_value = atoi(yytext);
                return ID_INT;
            }

{_float}    {
                begin_token(yyscanner, yyleng);
                yylval->float_value = atof(yytext);
                return ID_FLOAT;
            }

{unnamed}   {
                begin_token(yyscanner, yyleng);
                return *yytext;
            }

{ignored} ; // ignore whitespace

[\"]        {
                begin_token(yyscanner, yyleng);
                yy_push_state(STATE_STRING, yyscanner);
            }
<STATE_STRING>[^\"]* {
                begin_token(yyscanner, yyleng);
                yylval->string_value = yyget_extra(yyscanner)->tree_context().alloc_string(
                        escape_string(yytext)
                        );
                return ID_STRING;
            }
<STATE_STRING>[\"] {
                begin_token(yyscanner, yyleng);
                yy_pop_state(yyscanner);
            }

[\']        {
                begin_token(yyscanner, yyleng);
                yy_push_state(STATE_CHAR, yyscanner);
            }
<STATE_CHAR>[^\']* {
                begin_token(yyscanner, yyleng);
                std::string s = escape_string(yytext);
                yylval->char_value = s[0];
                return ID_CHAR;
            }
<STATE_CHAR>[\'] {
                begin_token(yyscanner, yyleng);
                yy_pop_state(yyscanner);
            }

"/*"                      { yy_push_state(STATE_COMMENT_BLOCK, yyscanner); }
<STATE_COMMENT_BLOCK>"*/" { yy_pop_state(yyscanner); }
<STATE_COMMENT_BLOCK>.    ; // ignore comment

"//"                     { yy_push_state(STATE_COMMENT_LINE, yyscanner); }
<STATE_COMMENT_LINE>"\n" { yy_pop_state(yyscanner); }
<STATE_COMMENT_LINE>.    ; // ignore comment

.           {
                begin_token(yyscanner, yyleng);
                _XLANG_error("unknown character");
                YY_FLUSH_BUFFER;
                BEGIN INITIAL;
            }

%%

void begin_token(yyscan_t yyscanner, size_t length)
{
    YYLTYPE* loc = yyget_lloc(yyscanner);
    loc->first_line = PARM.m_line;
    loc->first_column = PARM.m_prev_column;
    loc->last_line = PARM.m_line;
    loc->last_column = PARM.m_prev_column + length - 1;
    PARM.m_prev_column = PARM.m_column;
    if(length > 1)
        PARM.m_prev_column--;
}

std::string escape_string(std::string s)
{
    std::string s2;
    char* buf = new char[s.length()+1]; // can't use allocator for arrays
    strcpy(buf, s.c_str());
    int n = 0;
    bool escape_next_char = false;
    for(int i = 0; '\0' != buf[i]; i++)
    {
        if(escape_next_char)
        {
            escape_next_char = false;
            buf[i] = escape_char(buf[i]);
        }
        else
            if('\\' == buf[i])
            {
                escape_next_char = true;
                continue;
            }
        buf[n++] = buf[i];
    }
    buf[n] = '\0';
    s2 = buf;
    delete []buf;
    return s2;
}

char escape_char(char c)
{
    switch(c)
    {
        case 'r': return '\r';
        case 'n': return '\n';
        case 't': return '\t';
        case '0': return '\0';
    }
    return c;
}