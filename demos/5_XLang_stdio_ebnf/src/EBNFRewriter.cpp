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

void ebnf2bnf(xl::TreeContext* tc, xl::node::NodeIdentIFace* ast) // NOTE: non-const ast
{
    bool changed = false;
    do
    {
        EBNFPrinter v(tc);
        std::string captured_stdout;
        {
            v.redirect_stdout(); // begin capture stdout
            v.visit_any(ast); // accumulate changes encountered during visit
            captured_stdout = v.restore_stdout(); // end capture stdout
        }
        changed = v.get_changes().apply_changes(); // change EBNF --> BNF
        //if(!changed) // only keep stdout from last iteration
        {
            std::cout << "stdout: <<<" << std::endl;
            std::cout << captured_stdout;
            std::cout << ">>>" << std::endl;
        }
    } while(false);//while(changed); // repeat until no more changes
}
