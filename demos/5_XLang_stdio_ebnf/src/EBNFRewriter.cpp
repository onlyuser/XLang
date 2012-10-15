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

#include "EBNFRewriter.h" // EBNFRewriter
#include "EBNFPrinter.h" // EBNFPrinter
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLangTreeContext.h" // TreeContext
#include <iostream> // std::cout

//#define DEBUG_EBNF

void ebnf_to_bnf(xl::TreeContext* tc, xl::node::NodeIdentIFace* ast) // NOTE: non-const ast
{
    std::string captured_stdout;
    bool changed = false;
    size_t n = 0;
    do
    {
        #ifdef DEBUG_EBNF
            std::cout << "(iter #" << n << ") <<<" << std::endl;
        #endif
        EBNFChanges changes(tc);
        EBNFPrinter v(tc, &changes);
        {
            #ifdef DEBUG_EBNF
                v.visit_any(ast); // visit while recording changes
            #else
                v.redirect_stdout();                  // begin capture stdout
                v.visit_any(ast);                     // visit while recording changes
                captured_stdout = v.restore_stdout(); // end capture stdout
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
