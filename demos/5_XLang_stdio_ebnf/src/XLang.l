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
#include "XLangString.h" // xl::unescape
#include <stdlib.h> // atoi
#include <string> // std::string
#include <sstream> // std::stringstream

std::stringstream &buf()
{
    static std::stringstream _buf;
    return _buf;
}
void reset_ss(std::stringstream &ss)
{
    ss.str("");
    ss.clear();
}

// Temporary workaround for flex bug: yy_top_state crashes on INITIAL state - ID: 3462982
// http://sourceforge.net/tracker/?func=detail&aid=3462982&group_id=97492&atid=618177
#define TOP_STATE (                                           \
        yy_push_state(INITIAL),                               \
        ((yy_start_stack_ptr>1) ? yy_pop_state() : [](){}()), \
        yy_top_state())

%}

%option noyywrap nounput
%option stack
%x ST_COMMENT_BLOCK ST_COMMENT_LINE ST_STRING ST_CHAR
%x ST_UNION_BLOCK ST_PROTO_BLOCK ST_ACTION_BLOCK ST_CODE
%s ST_RULES

ws          [ \t\n]+
any         .|[\n]
lit_initial [<>=]
lit_rules   [:|()+*?;]
digit       [0-9]
alpha       [a-zA-Z]
id          {alpha}|[_]
ident       {id}+({digit}|{id})*
ident_dash  {id}+({digit}|{id}|[-])*
ident_fqn   {id}+({digit}|{id}|[:<>,*])*
_int        {digit}+
_float      {digit}*[.]{digit}+
decl        [%][^%{}]

%%

{ws}        ;

 /*==========================================================================*/
 /* COMMENTS */

"/*"                    { yy_push_state(ST_COMMENT_BLOCK); }
<ST_COMMENT_BLOCK>"*/"  { yy_pop_state(); }
<ST_COMMENT_BLOCK>{any} ;

"//"                   { yy_push_state(ST_COMMENT_LINE); }
<ST_COMMENT_LINE>"\n"  { yy_pop_state(); }
<ST_COMMENT_LINE>{any} ;

 /*==========================================================================*/
 /* QUOTES */

[\"]              { yy_push_state(ST_STRING); }
<ST_STRING>[^\"]* {
                _XLANG_lval.string_value = tree_context()->alloc_string(
                        xl::unescape(yytext)
                        );
                return ID_STRING;
            }
<ST_STRING>[\"] { yy_pop_state(); }

[\']            { yy_push_state(ST_CHAR); }
<ST_CHAR>[^\']* {
                _XLANG_lval.char_value = xl::unescape(yytext)[0];
                return ID_CHAR;
            }
<ST_CHAR>[\'] { yy_pop_state(); }

 /*==========================================================================*/
 /* DEFINITIONS SECTION */

<INITIAL>[\{]               { yy_push_state(ST_UNION_BLOCK); return *yytext; }
<ST_UNION_BLOCK>{ident_fqn} {
                _XLANG_lval.string_value = tree_context()->alloc_string(yytext);
                return ID_STRING;
            }
<ST_UNION_BLOCK>[;]   { return *yytext; }
<ST_UNION_BLOCK>"/*"  { yy_push_state(ST_COMMENT_BLOCK); }
<ST_UNION_BLOCK>"//"  { yy_push_state(ST_COMMENT_LINE); }
<ST_UNION_BLOCK>[\}]  { yy_pop_state(); return *yytext; }
<ST_UNION_BLOCK>{any} ;
<INITIAL>{decl}       { yyless(1); return *yytext; }
<INITIAL>{ident_dash} {
                _XLANG_lval.ident_value = tree_context()->alloc_unique_string(yytext);
                return ID_IDENT;
            }
<INITIAL>"%{" {
                if(TOP_STATE == INITIAL)
                    reset_ss(buf());
                yy_push_state(ST_PROTO_BLOCK);
            }
<ST_PROTO_BLOCK>"%}" {
                yy_pop_state();
                if(TOP_STATE == INITIAL)
                {
                    _XLANG_lval.string_value = tree_context()->alloc_string(buf().str());
                    return ID_STRING;
                }
            }
<ST_PROTO_BLOCK>{any}  { buf() << yytext; }
<INITIAL>"%%"          { BEGIN ST_RULES; return ID_FENCE; }
<INITIAL>{lit_initial} { return *yytext; }
<INITIAL>{any}         ;

 /*==========================================================================*/
 /* RULES SECTION */

<ST_RULES>[\{] {
                if(TOP_STATE == INITIAL)
                    reset_ss(buf());
                yy_push_state(ST_ACTION_BLOCK);
            }
<ST_ACTION_BLOCK>[\{] {
                yy_push_state(ST_ACTION_BLOCK);
                if(TOP_STATE != INITIAL)
                    REJECT;
            }
<ST_ACTION_BLOCK>[\}] {
                yy_pop_state();
                if(TOP_STATE == INITIAL)
                {
                    _XLANG_lval.string_value = tree_context()->alloc_string(buf().str());
                    return ID_STRING;
                }
                else
                    REJECT;
            }
<ST_ACTION_BLOCK>{any} { buf() << yytext; }
<ST_RULES>"%%"         { reset_ss(buf()); BEGIN ST_CODE; return ID_FENCE; }

 /*==========================================================================*/
 /* CODE SECTION */

<ST_CODE><<EOF>> {
                _XLANG_lval.string_value = tree_context()->alloc_string(buf().str());
                return ID_STRING;
            }
<ST_CODE>{any} { buf() << yytext; }

 /*==========================================================================*/
 /* PRIMITIVES */

<ST_RULES>{ident}|"%prec" {
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
<ST_RULES>{lit_rules} { return *yytext; }
<ST_RULES>{any}       {
                char buf[40];
                sprintf(buf, "unknown character: \'%c\'\n", *yytext);
                _XLANG_error(buf);
                YY_FLUSH_BUFFER;
                BEGIN INITIAL;
            }

 /*==========================================================================*/
 /* OTHER */

.           ;

%%