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
#include "XLang.h" // NodeBase
#include "XLang.tab.h" // ID_XXX (generated code)
#include <string.h> // memcpy
#include <stdlib.h> // atoi

// When in the lexer you have to access parm through the extra data.
//
#define PARM yyget_extra(yyscanner)->scan_context()

#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// We want to read from a the buffer in parm so we have to redefine the
// YY_INPUT macro (see section 10 of the flex manual 'The generated scanner')
//
#define YY_INPUT(buffer, res, max_size) \
    do { \
        if(PARM.m_pos >= PARM.m_length) \
            (res) = YY_NULL; \
        else { \
            (res) = MIN(PARM.m_length - PARM.m_pos, 1); \
            (res) > (int) (max_size) ? (res) = (max_size) : 0; \
            if('\n' == PARM.m_buf[PARM.m_pos]) { \
                PARM.m_line_num++; \
                PARM.m_col_num = 1; \
            } \
            else \
                PARM.m_col_num++; \
            memcpy((buffer), PARM.m_buf + PARM.m_pos, (res)); \
            PARM.m_pos += (res); \
        } \
    } while(0)

void begin_token(yyscan_t yyscanner, char* s);
char escape_char(char c);

%}

%option noyywrap
%option nounput
%option reentrant bison-bridge bison-locations
%x STATE_COMMENT_BLOCK STATE_COMMENT_LINE STATE_STRING STATE_CHAR

digit [0-9]
alpha [a-zA-Z]
id {alpha}|[_]
ident {id}+({digit}|{id})*
_int {digit}+
_float {digit}*[.]{digit}+
unnamed [,()+\-*/=]
ignored [ \t\n]+

%%

{ident} {
                begin_token(yyscanner, yytext);
                yylval->ident_value = yyget_extra(yyscanner)->alloc_unique_string(yytext);
                return ID_IDENT;
            }

{_int} {
                begin_token(yyscanner, yytext);
                yylval->int_value = atoi(yytext);
                return ID_INT;
            }

{_float} {
                begin_token(yyscanner, yytext);
                yylval->float_value = atof(yytext);
                return ID_FLOAT;
            }

{unnamed} {
                begin_token(yyscanner, yytext);
                return *yytext;
            }

{ignored} ; // ignore whitespace

[\"] { begin_token(yyscanner, yytext); BEGIN STATE_STRING; }
<STATE_STRING>[^\"]* {
                            begin_token(yyscanner, yytext);
                            yylval->string_value = yyget_extra(yyscanner)->alloc_string(yytext);
                            char* buf = new char[yylval->string_value->length()+1];
                            strcpy(buf, yylval->string_value->c_str());
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
                            *yylval->string_value = buf;
                            delete []buf;
                            return ID_STRING;
                        }
<STATE_STRING>[\"] { begin_token(yyscanner, yytext); BEGIN INITIAL; }

[\'] { begin_token(yyscanner, yytext); BEGIN STATE_CHAR; }
<STATE_CHAR>[^\']* {
                            begin_token(yyscanner, yytext);
                            if('\\' == *yytext)
                            {
                                yytext++;
                                *yytext = escape_char(*yytext);
                            }
                            if('\0' != *(yytext + 1))
                                _XLANG_error("bad escape character");
                            yylval->char_value = *yytext;
                            return ID_CHAR;
                        }
<STATE_CHAR>[\'] { begin_token(yyscanner, yytext); BEGIN INITIAL; }

"/*" {
                BEGIN STATE_COMMENT_BLOCK;
            }
<STATE_COMMENT_BLOCK>. {}
<STATE_COMMENT_BLOCK>"*/" {
                BEGIN INITIAL;
            }

"//" {
                BEGIN STATE_COMMENT_LINE;
            }
<STATE_COMMENT_LINE>. {}
<STATE_COMMENT_LINE>"\n" {
            BEGIN INITIAL;
            }

. {
                begin_token(yyscanner, yytext);
                _XLANG_error("unknown character");
                YY_FLUSH_BUFFER;
                BEGIN INITIAL;
            }

%%

void begin_token(yyscan_t yyscanner, char* s)
{
    int n = strlen(s);
    YYLTYPE* loc = yyget_lloc(yyscanner);
    loc->first_line = PARM.m_line_num;
    loc->first_column = PARM.m_prev_col_num;
    loc->last_line = PARM.m_line_num;
    loc->last_column = PARM.m_prev_col_num + n - 1;
    PARM.m_prev_col_num = PARM.m_col_num;
    if(n > 1)
        PARM.m_prev_col_num--;
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
