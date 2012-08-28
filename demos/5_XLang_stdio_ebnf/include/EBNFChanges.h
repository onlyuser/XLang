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

#ifndef EBNF_CHANGES_H_
#define EBNF_CHANGES_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLangTreeContext.h" // TreeContext
#include <string> // std::string
#include <list> // std::list
#include <map> // std::map
#include <set> // std::set

struct EBNFChanges
{
    EBNFChanges(xl::TreeContext* tc)
        : m_tc(tc), m_symbols_attach_loc(NULL), m_rules_attach_loc(NULL)
    {}
    void reset();
    bool apply();

private:
    xl::TreeContext* m_tc;
public:
    const xl::node::NodeIdentIFace *m_symbols_attach_loc, *m_rules_attach_loc;
    std::list<std::string> m_new_symbols;
    std::set<std::string> m_existing_symbols;
    std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>> m_insertions_after;
    std::map<const xl::node::NodeIdentIFace*, xl::node::NodeIdentIFace*> m_replacements;
#if 0 // NOTE: unused
    std::list<xl::node::NodeIdentIFace*> m_new_rules;
    std::set<const xl::node::NodeIdentIFace*> m_removals;
#endif
};

#endif
