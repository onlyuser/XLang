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

struct TreeChange
{
    typedef enum
    {
        NODE_INSERTIONS_AFTER,
        NODE_APPENDS_TO_BACK,
        NODE_REPLACEMENTS
    } node_type_change_t;

    typedef enum
    {
        STRING_APPENDS_TO_BACK,
        STRING_INSERTIONS_TO_FRONT
    } string_type_change_t;

    virtual ~TreeChange()
    {}
};

class TreeChangeNode : public TreeChange
{
public:
    TreeChangeNode(const xl::node::NodeIdentIFace* _node, xl::node::NodeIdentIFace* new_node)
        : m_node(_node), m_new_node(new_node)
    {}

private:
    const xl::node::NodeIdentIFace* m_node;
    xl::node::NodeIdentIFace*       m_new_node;
};

class TreeChangeString : public TreeChange
{
public:
    TreeChangeString(const xl::node::NodeIdentIFace* _node, std::string new_string)
        : m_node(_node), m_new_string(new_string)
    {}

private:
    const xl::node::NodeIdentIFace* m_node;
    std::string                     m_new_string;
};

class TreeChanges
{
public:
    TreeChanges()
    {}
    void reset();
    void add_node_change(
            TreeChange::node_type_change_t _type,
            const xl::node::NodeIdentIFace* _node,
            xl::node::NodeIdentIFace* new_node);
    void add_string_change(
            TreeChange::string_type_change_t _type,
            const xl::node::NodeIdentIFace* _node,
            std::string new_string);
    bool apply();

private:
    std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>> m_node_insertions_after;
    std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>> m_node_appends_to_back;
    std::map<const xl::node::NodeIdentIFace*, std::list<std::string>> m_string_appends_to_back;
    std::map<const xl::node::NodeIdentIFace*, std::list<std::string>> m_string_insertions_to_front;
    std::map<const xl::node::NodeIdentIFace*, xl::node::NodeIdentIFace*> m_node_replacements;

    std::list<TreeChange*> m_tree_changes;
};

#endif
