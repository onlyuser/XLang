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
#include "XLangString.h" // xl::unescape
#include <stdlib.h> // atoi

static char* rules_start;

%}

%option noyywrap nounput
%option stack
%x ST_COMMENT_BLOCK ST_COMMENT_LINE ST_STRING ST_CHAR
%s ST_DEFINITION_CODE ST_RULES ST_CODE ST_RULES_CODE

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

"/*"                      { yy_push_state(ST_COMMENT_BLOCK); }
<ST_COMMENT_BLOCK>"*/" { yy_pop_state(); }
<ST_COMMENT_BLOCK>.    ;

"//"                     { yy_push_state(ST_COMMENT_LINE); }
<ST_COMMENT_LINE>"\n" { yy_pop_state(); }
<ST_COMMENT_LINE>.    ;

"%{"                        { yy_push_state(ST_DEFINITION_CODE); }
<ST_DEFINITION_CODE>"%}" { yy_pop_state(); }
<ST_DEFINITION_CODE>.    ;
"%%"                        { BEGIN ST_RULES; }
<ST_RULES>[\{] {
                if(yy_top_state() == INITIAL)
                    rules_start = yytext;
                yy_push_state(ST_RULES_CODE);
            }
<ST_RULES_CODE>[\{]      { yy_push_state(ST_RULES_CODE); }
<ST_RULES_CODE>[\}]      {
                yy_pop_state();
                if(yy_top_state() == INITIAL)
                {
                    _XLANG_lval.string_value = tree_context()->alloc_string(
                            xl::unescape(rules_start)
                            );
                    return ID_STRING;
                }
            }
<ST_RULES_CODE>.         ;
<ST_RULES>"%%"           { BEGIN ST_CODE; }
<ST_CODE>.               ;

<ST_RULES>{ident} {
                _XLANG_lval.ident_value = tree_context()->alloc_unique_string(yytext);
                return ID_IDENT;
            }
<ST_RULES>{_int} {
                _XLANG_lval.int_value = atoi(yytext);
                return ID_INT;
            }
<ST_RULES>{_float} {
                _XLANG_lval.float_value = atof(yytext);
                return ID_FLOAT;
            }
<ST_RULES>{literal} { return *yytext; }

[\"]        { yy_push_state(ST_STRING); }
<ST_STRING>[^\"]* {
                _XLANG_lval.string_value = tree_context()->alloc_string(
                        xl::unescape(yytext)
                        );
                return ID_STRING;
            }
<ST_STRING>[\"] { yy_pop_state(); }

[\']        { yy_push_state(ST_CHAR); }
<ST_CHAR>[^\']* {
                std::string s = xl::unescape(yytext);
                _XLANG_lval.char_value = s[0];
                return ID_CHAR;
            }
<ST_CHAR>[\'] { yy_pop_state(); }

<ST_RULES>. {
                char buf[40];
                sprintf(buf, "unknown character: \'%c\'\n", *yytext);
                _XLANG_error(buf);
                YY_FLUSH_BUFFER;
                BEGIN INITIAL;
            }

.           ;

%%