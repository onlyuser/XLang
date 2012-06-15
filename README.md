**Variations of a Lex-Yacc parser**

based on "A COMPACT GUIDE TO LEX & YACC" by Tom Niemann

Copyright (C) 2011-2012 Jerry Chen <mailto:onlyuser@gmail.com>

**About:**

XLang is a Lex-Yacc parser "framework" for rapid language prototyping. It
is also a demonstration of various ways one could build a yacc-based
parser. Seven variations of parser are presented, varying in feature
completeness.

The first variation (0_XLang_full), is a full-featured parser. With each
successive variation, non-critical features are stripped until there is
nothing left but a flex-less parser, the last variation (7_XLang_no-flex).

full, no-strings, no-comments, no-locations, no-reentrant, stdio, file, no-flex

The order in which features are removed is completely arbitrary. There's no
good reason one cannot cannot have both "comments" and "file input"
enabled, or both "reentrant" and "hand-rolled lexer" enabled.

The code is organized in a way such that anyone unfamiliar with Lex-Yacc
can quickly isolate a variation that is most relevent, and most readily
customizable to his/her specific requirements.

**Variation vs Feature Table:**

<table>
    <tr><th> variation    </th><th> lexer       </th><th> input  </th><th> reentrant </th><th> locations </th><th> comments </th><th> strings </th></tr>
    <tr><td> full         </td><td> flex        </td><td> buffer </td><td> yes       </td><td> yes       </td><td> yes      </td><td> yes     </td><tr/>
    <tr><td> no-strings   </td><td> flex        </td><td> buffer </td><td> yes       </td><td> yes       </td><td> yes      </td><td> no      </td><tr/>
    <tr><td> no-comments  </td><td> flex        </td><td> buffer </td><td> yes       </td><td> yes       </td><td> no       </td><td> no      </td><tr/>
    <tr><td> no-locations </td><td> flex        </td><td> buffer </td><td> yes       </td><td> no        </td><td> no       </td><td> no      </td><tr/>
    <tr><td> no-reentrant </td><td> flex        </td><td> buffer </td><td> no        </td><td> no        </td><td> no       </td><td> no      </td><tr/>
    <tr><td> stdio        </td><td> flex        </td><td> stdio  </td><td> no        </td><td> no        </td><td> no       </td><td> no      </td><tr/>
    <tr><td> file         </td><td> flex        </td><td> file   </td><td> no        </td><td> no        </td><td> no       </td><td> no      </td><tr/>
    <tr><td> no-flex      </td><td> hand-rolled </td><td> file   </td><td> no        </td><td> no        </td><td> no       </td><td> no      </td><tr/>
</table>

    HINT: The "stdio" variation is easiest to implement -- The flex lexer
          accepts standard stream input by default. It takes extra effort to
          hack flex into using "buffer input" or "file input".

**Feature Detail:**

    lexer:

        flex: Use flex to generate a lexer.
              This option is preferred when regex significantly simplifies the
              lexer.

        hand-rolled: Use a custom lexer.
                     This option is preferred when speed is critical -- Flex is
                     slow. This option is also preferred when unicode support
                     is needed -- Flex has no unicode support as of the time of
                     this writing.

    input:

        buffer: Use in-memory buffer as input.
                This option is preferred when speed is critical -- Standard
                streams are slow. This option is also preferred when the input
                string already resides in memory and does not require passing
                through a standard stream in order to be used.

        stdio: Use standard input.
               This is the default behavior.

    reentrant: Tell the parser not to use global variables.
               This option is required to make the parser multi-thread
               compliant.

    locations: Tell the parser to provide a location variable "$@" for each BNF
               production. This option demonstrates the standard way to track
               syntax error line/column number using Lex-Yacc.

    comments: Tell the lexer to conditionally enable/disable certain lexer
              rules for stateful scanning. This option demonstrates the
              standard way to ignore line/block comments.

    strings: Tell the lexer to conditionally enable/disable certain lexer rules
             for stateful scanning. This option demonstrates *one* of the many
             ways to handle strings, characters, and escaped characters.

**Requirements:**

<dl>
    <dt>Unix tools and 3rd party components (accessible from $PATH):</dt>
    <dd>gcc (with -std=c++0x support), flex, bison, valgrind, cppcheck, doxygen, graphviz, ticpp</dd>
    <dt>Environment variables:</dt>
    <dd>$EXTERN_INCLUDE_PATH -- where "ticpp/ticpp.h" resides</dd>
    <dd>$EXTERN_LIB_PATH     -- where "libticppd.a" resides</dd>
</dl>

**Make targets:**

    all, test, pure, dot, lint, doc, xml, import, clean.

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

**FAQ:**

1.  What is XLang ?

    XLang is a starting point for people looking to construct their own
    language using Lex-Yacc. XLang is thoroughly tested and comes with its
    own test suite.

2.  What isn't XLang ?

    XLang isn't a unified parser front-end for every language under the sun.
    It makes some assumptions about the target language, meaning, literal
    values must be C-like. Otherwise, it places no restrictions on the
    grammar.

3.  How should one use XLang ?

    I recommend hacking the "stdio" example to your liking, but any of the
    other variations equally suitable as a base for beginners.

4.  What licenses apply when using XLang ?

    GPL3.

5.  What's the motivation behind writing XLang ?

    Lex-Yacc is a pragmatic solution that will get the job done, but it
    doesn't do so right out of the box. It needs some infrastructure to get
    most sizeable projects going.

    I built XLang out of my own need for a parser SDK that takes care of the
    tedious tree building business so I could focus more on the fun stuff --
    AST visitation.

    But isn't this exactly what production rule actions are for ? No. In
    order to perform multi-pass visitation to an AST, using Yacc, one would
    have to re-parse the input everytime, and that's wasteful. XLang builds
    an AST for you so you can visit it as many times as needed.

    I owe everything I know about Lex-Yacc to Tom Niemann. Please read his
    excellent tutorial.

6.  How does XLang differ from the example in Tom's tutorial ?

    a)  XLang uses C++, with unions used only where necessary. Tom's tutorial
        uses a C-style polymorphism technique where the last member of a
        class is a union of several types. I prefer C++ polymorphism.

    b)  XLang has its own memory management system based on allocators. Tom's
        tutorial omits AST node freeing entirely.

    c)  XLang offers several flavors of Lex-Yacc whereas Tom's tutorial
        offers just one "stdio" parser.

    d)  XLang's ASTs are "flattened", meaning lists are interpreted as lists
        instead of deep-recursing binary trees. This tree organization lends
        itself better to AST visitation (less likely to stack-overflow).

            (1+2+3)        (1+2+3)
            non-flattened: flattened:
            
                +             +
               / \          / | \
              +   3        1  2  3
             / \
            1   2

    e)  XLang's visitor borrows concepts from Anand Shankar Krishnamoorthi's
        cooperative visitor and Jeremy Blosser's Java Tip 98 to enable
        visit double-dispatch without the need to implement the "accept"
        method for every single AST node class.

    f)  XLang uses namespaces and isolates all the AST building code into a
        static library.

7.  Why allocators ?

    I've incorporated a simple memory allocator in this project because I
    wanted to keep my AST node classes as clean as possible, without
    destructors that delete child nodes. But the way the allocator was
    written really doesn't improve the program's memory usage patterns. With
    some effort, I suppose one could upgrade that into a real allocator with
    true memory pooling.

8.  Why c++0x ?

    Lambda functions are the only c++0x feature used here, and only because
    they solve the problem elegantly. The goal is to deliberately avoid
    implementing clean-up code within the AST node destructors. This goes
    along a grander design decision to keep the AST node class clear of
    method clutter by moving all the predicate code into visitors. Lambdas
    help us provide temporary anonymous callbacks for the allocator, whose
    job is to perform clean-up originally intended for the destructor.

9.  Why coroutines ?

    I needed a way to store the "progress" of a visitation, so it can be
    resumed later. A direct approach would be keeping a reference to the
    most recently visited child node and tucking that away in a static
    variable, but this assumes a linear visitation limited to ascending or
    descending traversals. Enter the coroutine, whose main purpose is to
    provide flow control for systems that need suspending and resuming of
    execution, exactly what is needed for my visitor.

    I use Simon Tatham's excellent implementation of this immensely useful
    concept. Please visit his page to learn more (as his use of macros is
    quite involving).

**References:**

<dl>
    <dt>"Flex your lexical analysis muscles"</dt>
    <dd>http://www.codeguru.com/cpp/cpp/algorithms/strings/article.php/c12717/Flex-Your-Lexical-Analysis-Muscles.htm</dd>
    <dd>http://www.developer.com/net/cplus/article.php/3636641/Flex-Your-Lexical-Analysis-Muscles.htm</dd>

    <dt>"Classic Parsing with Flex-Bison"</dt>
    <dd>http://www.codeguru.com/csharp/.net/net_general/patterns/article.php/c12805/Classic-Parsing-with-Flex-and-Bison.htm</dd>
    <dd>http://www.developer.com/net/cplus/article.php/3642516/Classic-Parsing-with-Flex-and-Bison.htm</dd>

    <dt>"Tom Niemann Flex-Bison AST examples"</dt>
    <dd>http://epaperpress.com/lexandyacc/</dd>

    <dt>"UsualCoding.eu Reentrant Flex-Bison example"</dt>
    <dd>http://www.usualcoding.eu/post/2007/09/03/Building-a-reentrant-parser-in-C-with-Flex/Bison</dd>
    <dd>(code example needs "void yyerror() {}" inserted in declaration section of parser.y)</dd>
    <dd>(page seems down as of May 19, 2012)</dd>

    <dt>"Make a reentrant parser with Flex and Bison"</dt>
    <dd>http://www.lemoda.net/c/reentrant-parser/index.html</dd>

    <dt>"Better error handling using Flex and Bison"</dt>
    <dd>http://www.ibm.com/developerworks/library/l-flexbison.html</dd>

    <dt>"ProgTools Flex-Bison AST tutorial (with classes)"</dt>
    <dd>http://www.progtools.org/compilers/tutorials/cxx_and_bison/cxx_and_bison.html</dd>

    <dt>"O-Reilly Lex-Yacc book examples"</dt>
    <dd>http://examples.oreilly.com/lex/</dd>

    <dt>"Cooperative Visitor: A Template Technique for Visitor Creation"</dt>
    <dd>http://www.artima.com/cppsource/cooperative_visitor.html</dd>

    <dt>"Java Tip 98: Reflect on the Visitor design pattern"</dt>
    <dd>http://www.javaworld.com/javaworld/javatips/jw-javatip98.html</dd>

    <dt>"Coroutines in C"</dt>
    <dd>http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html</dd>
</dl>

**Additional Reading:**

* http://osdir.com/ml/lex.flex.windows/2003-05/msg00017.html
* http://tldp.org/HOWTO/Lex-YACC-HOWTO-5.html
* http://net.pku.edu.cn/~course/cs201/2003/mirrorWebster.cs.ucr.edu/Page_softeng/softDevGuide_6.html

**Keywords:**

    Lex, Yacc, Flex, Bison, Parsing, C++, Reentrant C++ Parser
