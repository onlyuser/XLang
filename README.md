[![Build Status](https://secure.travis-ci.org/onlyuser/XLang.png)](http://travis-ci.org/onlyuser/XLang)

XLang
=====

Copyright (C) 2011-2013 Jerry Chen <mailto:onlyuser@gmail.com>

About:
------

XLang is a parser framework for language modeling.

A Motivating Example:
---------------------

<table>
    <tr>
        <td>
            <pre>
void NodeEvaluator::visit(const xl::node::SymbolNodeIFace* _node)
{
    if(_node->lexer_id() == ID_UMINUS)
    {
        visit_next_child(_node);
        value = -value;
        return;
    }
    float32_t _value = 0;
    bool more = visit_next_child(_node);
    _value = value;
    while(more)
    {
        more = visit_next_child(_node);
        switch(_node->lexer_id())
        {
            case '+': _value += value; break;
            case '-': _value -= value; break;
            case '*': _value *= value; break;
            case '/': _value /= value; break;
        }
    }
    value = _value;
    if(_node->is_root())
        std::cout << _value << std::endl;
}
            </pre>
        </td>
        <td>
            <pre>
%%

root:
      program { tree_context()->root() = $1; YYACCEPT; }
    | error   { yyclearin; /* yyerrok; YYABORT; */ }
    ;

program:
      statement             { $$ = $1; }
    | program ',' statement { $$ = MAKE_SYMBOL(',', 2, $1, $3); }
    ;

statement:
      expression              { $$ = $1; }
    | ID_IDENT '=' expression { $$ = MAKE_SYMBOL('=', 2, MAKE_TERM(ID_IDENT, $1), $3); }
    ;

expression:
      ID_INT                         { $$ = MAKE_TERM(ID_INT, $1); }
    | ID_FLOAT                       { $$ = MAKE_TERM(ID_FLOAT, $1); }
    | ID_IDENT                       { $$ = MAKE_TERM(ID_IDENT, $1); }
    | '-' expression %prec ID_UMINUS { $$ = MAKE_SYMBOL(ID_UMINUS, 1, $2); }
    | expression '+' expression      { $$ = MAKE_SYMBOL('+', 2, $1, $3); }
    | expression '-' expression      { $$ = MAKE_SYMBOL('-', 2, $1, $3); }
    | expression '*' expression      { $$ = MAKE_SYMBOL('*', 2, $1, $3); }
    | expression '/' expression      { $$ = MAKE_SYMBOL('/', 2, $1, $3); }
    | '(' expression ')'             { $$ = $2; }
    ;

%%
            </pre>
        </td>
    </tr>
</table>

Usage:
------

<pre>
cat input | ./XLang -x > output.xml
</pre>

Requirements:
-------------

Unix tools and 3rd party components (accessible from $PATH):

    gcc (with -std=c++0x support), flex, bison, valgrind, cppcheck, doxygen, graphviz, ticpp

**Environment variables:**

* $EXTERN_INCLUDE_PATH -- where "ticpp/ticpp.h" resides
* $EXTERN_LIB_PATH     -- where "libticppd.a" resides

Make targets:
-------------

<table>
    <tr><th> target </th><th> action                                                </th></tr>
    <tr><td> all    </td><td> make binaries                                         </td></tr>
    <tr><td> test   </td><td> all + run tests                                       </td></tr>
    <tr><td> pure   </td><td> test + use valgrind to check for memory leaks         </td></tr>
    <tr><td> dot    </td><td> test + generate .png graph for tests                  </td></tr>
    <tr><td> lint   </td><td> use cppcheck to perform static analysis on .cpp files </td></tr>
    <tr><td> doc    </td><td> use doxygen to generate documentation                 </td></tr>
    <tr><td> xml    </td><td> test + generate .xml for tests                        </td></tr>
    <tr><td> import </td><td> test + use ticpp to serialize-to/deserialize-from xml </td></tr>
    <tr><td> clean  </td><td> remove all intermediate files                         </td></tr>
</table>

References:
-----------

<dl>
    <dt>"Tom Niemann Flex-Bison AST examples"</dt>
    <dd>http://epaperpress.com/lexandyacc/</dd>
</dl>

Keywords:
---------

    Lex, Yacc, Flex, Bison, Parser, Reentrant
