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
#include <stdlib.h> // atoi

#define LVAL         _xl(lval)
#define TREE_CONTEXT (*tree_context())

%}

%option noyywrap nounput

ws        [ \t\n]+
lit_char  [,()+\-*/^=]
digit     [0-9]
alpha     [a-zA-Z]
id        {alpha}|[_]
lit_ident {id}+({digit}|{id})*
lit_int   {digit}+
lit_float {digit}*[.]{digit}+

noun_pron   "i"|"you"|"he"|"she"|"him"|"her"|"it"|"them"|"we"|"us"|"here"|"there"|"everywhere"|"nowhere"|"someone"|"somewhere"
noun_places "house"|"home"|"school"|"library"|"store"
noun_actors "boy"|"girl"|"man"|"woman"|"fox"|"dog"|"book"|"apple"
noun        {noun_pron}|{noun_places}|{noun_actors}

verb_motion_move "move"|"moves"|"moving"|"moved"
verb_motion_come "come"|"comes"|"coming"|"came"
verb_motion_go   "go"|"goes"|"going"|"went"|"gone"
verb_motion_run  "run"|"runs"|"running"|"ran"
verb_motion_jump "jump"|"jumps"|"jumping"|"jumped"
verb_motion      {verb_motion_move}|{verb_motion_come}|{verb_motion_go}|{verb_motion_run}|{verb_motion_jump}

verb_access_create  "create"|"creates"|"creating"|"created"
verb_access_destroy "destroy"|"destroys"|"destroying"|"destroyed"
verb_access_open    "open"|"opens"|"opening"|"opened"
verb_access_close   "close"|"closes"|"closing"|"closed"
verb_access_change  "change"|"changes"|"changing"|"changed"
verb_access         {verb_access_create}|{verb_access_destroy}|{verb_access_open}|{verb_access_close}|{verb_access_change}

verb_misc_do      "do"|"does"|"doing"|"did"|"done"
verb_misc_make    "make"|"makes"|"making"|"made"
verb_misc_get     "get"|"gets"|"getting"|"got"|"gotten"
verb_misc_put     "put"|"puts"|"putting"
verb_misc_give    "give"|"gives"|"giving"|"gave"|"given"
verb_misc_take    "take"|"takes"|"taking"|"took"|"taken"
verb_misc_receive "receive"|"receives"|"receiving"|"received"
verb_misc         {verb_misc_do}|{verb_misc_make}|{verb_misc_get}|{verb_misc_put}|{verb_misc_give}|{verb_misc_take}|{verb_misc_receive}

verb {verb_motion}|{verb_access}|{verb_misc}

adj_size  "big"|"small"
adj_color "red"|"orange"|"yellow"|"green"|"blue"|"purple"|"black"|"white"|"brown"
adj_speed "fast"|"slow"|"quick"
adj       {adj_size}|{adj_color}|{adj_speed}|"lazy"

prep_basic             "to"|"from"|"of"|"in"|"on"|"at"
prep_spatial_distance  "near"|"far"
prep_spatial_motion    "onto"|"into"|"through"
prep_spatial_container "inside"|"outside"|"within"|"without"
prep_spatial_vertical  "above"|"below"|"over"|"under"|"underneath"|"beneath"
prep_spatial_order     "behind"|"before"|"after"|"beyond"|"around"
prep_spatial           {prep_spatial_distance}|{prep_spatial_motion}|{prep_spatial_container}|{prep_spatial_vertical}|{prep_spatial_order}
prep                   {prep_basic}|{prep_spatial}

adv   "always"|"must"|"will"|"would"|"may"|"might"|"could"|"can"|"cannot"|"not"|"never"|"very"
aux   "is"|"was"|"were"
det   "a"|"the"|"this"|"that"|"these"|"those"
conj  "for"|"and"|"nor"|"but"|"or"|"yet"|"so"
conj2 "for2"|"and2"|"nor2"|"but2"|"or2"|"yet2"|"so2"
conj3 "for3"|"and3"|"nor3"|"but3"|"or3"|"yet3"|"so3"

%%

{ws}        ;

 /*==========================================================================*/
 /* LITERALS */

{noun}      {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_NOUN;
            }

{verb}      {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_VERB;
            }

{adj}       {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_ADJ;
            }

{adv}       {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_ADV;
            }

{prep}      {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_PREP;
            }

{aux}       {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_AUX;
            }

{det}       {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_DET;
            }

{conj}      {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_CONJ;
            }

{conj2}     {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_CONJ_2;
            }

{conj3}     {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_CONJ_3;
            }

{lit_ident} {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_IDENT;
            }

{lit_int}   {
                LVAL.int_value = atoi(yytext);
                return ID_INT;
            }

{lit_float} {
                LVAL.float_value = atof(yytext);
                return ID_FLOAT;
            }

{lit_char}  {
                return *yytext;
            }

 /*==========================================================================*/
 /* OTHER */

.           {
                _xl(error)("unknown character");
                YY_FLUSH_BUFFER;
            }

%%