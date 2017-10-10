[![Build Status](https://secure.travis-ci.org/onlyuser/XLang.png)](http://travis-ci.org/onlyuser/XLang)

XLang
=====

Copyright (C) 2011-2017 <mailto:onlyuser@gmail.com>

About
-----

XLang is a parser framework for language modeling.

It handles AST MVC, so as a language developer, you can focus on more important things, such as tuning the grammar.

A Motivating Example
--------------------

input:
<pre>
(-b+(b^2-4*a*c))/(2*a)
</pre>

output:

![picture alt](https://sites.google.com/site/onlyuser/files/ast_math.png "ast_math")

Usage
-----

<pre>
echo "(-b+(b^2-4*a*c))/(2*a)" | ./bin/XLang -d | dot -Tpng -oquadratic.png
</pre>

Requirements
------------

Unix tools and 3rd party components (accessible from $PATH):

    gcc flex bison valgrind cppcheck doxygen graphviz ticpp

**Environment variables:**

* $INCLUDE_PATH_EXTERN -- where "ticpp/ticpp.h" resides
* $LIB_PATH_EXTERN     -- where "libticppd.a" resides

**Ubuntu packages:**

* sudo apt-get install flex
* sudo apt-get install bison

Make Targets
------------

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

References
----------

<dl>
    <dt>"Tom Niemann Flex-Bison AST examples"</dt>
    <dd>http://epaperpress.com/lexandyacc/</dd>
    <dt>"Most vexing parse"</dt>
    <dd>https://en.wikipedia.org/wiki/Most_vexing_parse</dd>
    <dt>"Lexical Analysis With Flex - Start Conditions"</dt>
    <dd>http://flex.sourceforge.net/manual/Start-Conditions.html</dd>
    <dt>"Multiple Input Buffers"</dt>
    <dd>http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html</dd>
    <dt>"Flex, A fast scanner generator - Actions"</dt>
    <dd>http://dinosaur.compilertools.net/flex/flex_9.html</dd>
</dl>

Keywords
--------

    Lex, Yacc, Flex, Bison, Parser, Reentrant, Abstract Syntax Tree, Model-View-Controller, Scott Meyer's "Most Vexing Parse", Multiple Input Buffers
