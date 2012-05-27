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

%option noyywrap nounput

digit   [0-9]
alpha   [a-zA-Z]
id      {alpha}|[_]
ident   {id}+({digit}|{id})*
_int    {digit}+
_float  {digit}*[.]{digit}+
unnamed [,()+\-*/=]
ignored [ \t\n]+

D  [0-9]
L  [a-zA-Z_]
H  [a-fA-F0-9]
E  [Ee][+-]?{D}+
FS (f|F|l|L)
IS (u|U|l|L)*

%%

{ident}     {
                _XLANG_lval.ident_value = tree_context()->alloc_unique_string(yytext);
                return IDENTIFIER;
            }

{_int}      {
                _XLANG_lval.int_value = atoi(yytext);
                return ID_INT;
            }

{_float}    {
                _XLANG_lval.float_value = atof(yytext);
                return ID_FLOAT;
            }

"auto"     { return(AUTO); }
"break"    { return(BREAK); }
"case"     { return(CASE); }
"char"     { return(CHAR); }
"const"    { return(CONST); }
"continue" { return(CONTINUE); }
"default"  { return(DEFAULT); }
"do"       { return(DO); }
"double"   { return(DOUBLE); }
"else"     { return(ELSE); }
"enum"     { return(ENUM); }
"extern"   { return(EXTERN); }
"float"    { return(FLOAT); }
"for"      { return(FOR); }
"goto"     { return(GOTO); }
"if"       { return(IF); }
"int"      { return(INT); }
"long"     { return(LONG); }
"register" { return(REGISTER); }
"return"   { return(RETURN); }
"short"    { return(SHORT); }
"signed"   { return(SIGNED); }
"sizeof"   { return(SIZEOF); }
"static"   { return(STATIC); }
"struct"   { return(STRUCT); }
"switch"   { return(SWITCH); }
"typedef"  { return(TYPEDEF); }
"union"    { return(UNION); }
"unsigned" { return(UNSIGNED); }
"void"     { return(VOID); }
"volatile" { return(VOLATILE); }
"while"    { return(WHILE); }

{L}({L}|{D})*          { }

0[xX]{H}+{IS}?         { return(CONSTANT); }
0{D}+{IS}?             { return(CONSTANT); }
{D}+{IS}?              { return(CONSTANT); }
L?'(\\.|[^\\'])+'      { return(CONSTANT); }

{D}+{E}{FS}?           { return(CONSTANT); }
{D}*"."{D}+({E})?{FS}? { return(CONSTANT); }
{D}+"."{D}*({E})?{FS}? { return(CONSTANT); }

L?\"(\\.|[^\\"])*\"    { return(STRING_LITERAL); }

"..."       { return(ELLIPSIS); }
">>="       { return(RIGHT_ASSIGN); }
"<<="       { return(LEFT_ASSIGN); }
"+="        { return(ADD_ASSIGN); }
"-="        { return(SUB_ASSIGN); }
"*="        { return(MUL_ASSIGN); }
"/="        { return(DIV_ASSIGN); }
"%="        { return(MOD_ASSIGN); }
"&="        { return(AND_ASSIGN); }
"^="        { return(XOR_ASSIGN); }
"|="        { return(OR_ASSIGN); }
">>"        { return(RIGHT_OP); }
"<<"        { return(LEFT_OP); }
"++"        { return(INC_OP); }
"--"        { return(DEC_OP); }
"->"        { return(PTR_OP); }
"&&"        { return(AND_OP); }
"||"        { return(OR_OP); }
"<="        { return(LE_OP); }
">="        { return(GE_OP); }
"=="        { return(EQ_OP); }
"!="        { return(NE_OP); }
";"         { return(';'); }
("{"|"<%")  { return('{'); }
("}"|"%>")  { return('}'); }
","         { return(','); }
":"         { return(':'); }
"="         { return('='); }
"("         { return('('); }
")"         { return(')'); }
("["|"<:")  { return('['); }
("]"|":>")  { return(']'); }
"."         { return('.'); }
"&"         { return('&'); }
"!"         { return('!'); }
"~"         { return('~'); }
"-"         { return('-'); }
"+"         { return('+'); }
"*"         { return('*'); }
"/"         { return('/'); }
"%"         { return('%'); }
"<"         { return('<'); }
">"         { return('>'); }
"^"         { return('^'); }
"|"         { return('|'); }
"?"         { return('?'); }

{unnamed}   {
                return *yytext;
            }

{ignored} ; // ignore whitespace

.           {
                _XLANG_error("unknown character");
                YY_FLUSH_BUFFER;
            }

%%