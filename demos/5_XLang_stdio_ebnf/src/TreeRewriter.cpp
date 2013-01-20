// Variations of a Flex-Bison parser
// -- based on "A COMPACT GUIDE TO LEX & YACC" by Tom Niemann
// Copyright (C) 2011 Jerry Chen <mailto:onlyuser@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "TreeRewriter.h" // TreeRewriter
#include "visitor/XLangVisitorDFS.h" // visitor::VisitorDFS
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLangTreeContext.h" // TreeContext
#include "TreeChanges.h" // TreeChanges
#include "SetTreeChangesIFace.h" // SetTreeChangesIFace
#include <iostream> // std::cout

//#define DEBUG_EBNF

static std::stringstream _cout_buf;
static std::streambuf* _prev_stream_buf;

static void _begin_redirect_stdout()
{
    _prev_stream_buf = std::cout.rdbuf(_cout_buf.rdbuf());
}

static std::string _end_redirect_stdout()
{
    std::string s = _cout_buf.str();
    std::cout.rdbuf(_prev_stream_buf);
    _cout_buf.str(std::string());
    _cout_buf.clear();
    return s;
}

void rewrite_tree_until_stable(
        xl::node::NodeIdentIFace* ast, // NOTE: non-const ast
        xl::visitor::VisitorDFS* v,
        xl::TreeContext* tc)
{
    std::string captured_stdout;
    bool changed = false;
    size_t n = 0;
    do
    {
        #ifdef DEBUG_EBNF
            std::cout << "(iter #" << n << ") <<<" << std::endl;
        #endif
        TreeChanges changes(tc);
        dynamic_cast<SetTreeChangesIFace*>(v)->setTreeChanges(&changes);
        {
            #ifdef DEBUG_EBNF
                v->visit_any(ast); // visit while recording changes
            #else
                _begin_redirect_stdout();                 // begin capture stdout
                v->visit_any(ast);                        // visit while recording changes
                captured_stdout = _end_redirect_stdout(); // end capture stdout
            #endif
        }
        changed = changes.apply(); // apply changes
        #ifdef DEBUG_EBNF
            std::cout << ">>> (iter #" << n << ")" << std::endl;
        #endif
        n++;
    } while(changed); // repeat until stabilize
    #ifndef DEBUG_EBNF
        std::cout << captured_stdout;
    #endif
}
