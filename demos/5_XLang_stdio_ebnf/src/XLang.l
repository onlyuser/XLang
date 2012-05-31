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

std::string escape_string(std::string s);
char escape_char(char c);

%}

%option noyywrap nounput noyy_top_state
%option stack
%x STATE_COMMENT_BLOCK STATE_COMMENT_LINE STATE_STRING STATE_CHAR
%s STATE_DEFINITION_CODE STATE_RULES STATE_CODE STATE_RULES_CODE

whitespace [ \t\n]+
literal    [:|()+*?;]
digit      [0-9]
alpha      [a-zA-Z]
id         {alpha}|[_]
ident      [%]?{id}+({digit}|{id})*
_int       {digit}+
_float     {digit}*[.]{digit}+

%%

{whitespace} ;

"/*"                      { yy_push_state(STATE_COMMENT_BLOCK); }
<STATE_COMMENT_BLOCK>"*/" { yy_pop_state(); }
<STATE_COMMENT_BLOCK>.    ;

"//"                     { yy_push_state(STATE_COMMENT_LINE); }
<STATE_COMMENT_LINE>"\n" { yy_pop_state(); }
<STATE_COMMENT_LINE>.    ;

"%{"                        { yy_push_state(STATE_DEFINITION_CODE); }
<STATE_DEFINITION_CODE>"%}" { yy_pop_state(); }
<STATE_DEFINITION_CODE>.    ;
"%%"                        { BEGIN STATE_RULES; }
<STATE_RULES>[\{]           { yy_push_state(STATE_RULES_CODE); }
<STATE_RULES_CODE>[\{]      { yy_push_state(STATE_RULES_CODE); }
<STATE_RULES_CODE>[\}]      { yy_pop_state(); }
<STATE_RULES_CODE>.         ;
<STATE_RULES>"%%"           { BEGIN STATE_CODE; }
<STATE_CODE>.               ;

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
<STATE_RULES>{literal} { return *yytext; }

[\"]        { yy_push_state(STATE_STRING); }
<STATE_STRING>[^\"]* {
                _XLANG_lval.string_value = tree_context()->alloc_string(
                        escape_string(yytext)
                        );
                return ID_STRING;
            }
<STATE_STRING>[\"] { yy_pop_state(); }

[\']        { yy_push_state(STATE_CHAR); }
<STATE_CHAR>[^\']* {
                std::string s = escape_string(yytext);
                _XLANG_lval.char_value = s[0];
                return ID_CHAR;
            }
<STATE_CHAR>[\'] { yy_pop_state(); }

<STATE_RULES>. {
                char buf[20];
                sprintf(buf, "unknown character: \'%c\'\n", *yytext);
                _XLANG_error(buf);
                YY_FLUSH_BUFFER;
                BEGIN INITIAL;
            }

.           ;

%%

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