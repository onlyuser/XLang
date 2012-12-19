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

#ifndef TREE_CHANGES_H_
#define TREE_CHANGES_H_

#include <string> // std::string
#include <list> // std::list
#include <map> // std::map

namespace xl { namespace node { class NodeIdentIFace; } }
namespace xl { class TreeContext; }

struct TreeChanges
{
    TreeChanges(xl::TreeContext* tc) : m_tc(tc)
    {}
    void reset();
    bool apply();

private:
    xl::TreeContext* m_tc;
public:
    std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>> m_node_insertions_after;
    std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>> m_node_appends_to_back;
    std::map<const xl::node::NodeIdentIFace*, std::list<std::string>> m_string_appends_to_back;
    std::map<const xl::node::NodeIdentIFace*, std::list<std::string>> m_string_insertions_to_front;
    std::map<const xl::node::NodeIdentIFace*, xl::node::NodeIdentIFace*> m_node_replacements;
};

#endif
