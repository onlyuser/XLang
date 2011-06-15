// Variations of a Flex-Bison parser -- based on
// "A COMPACT GUIDE TO LEX & YACC" by Tom Niemann
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

#ifndef XLANG_NODE_BASE_H_
#define XLANG_NODE_BASE_H_

#include "XLangType.h" // uint32
#include <string> // std::string

namespace node {

class NodeBase
{
public:
    typedef enum { INT, FLOAT, STRING, CHAR, IDENT, INNER } type_e;

    virtual type_e type() const = 0;
    virtual uint32 sym_id() const = 0;
    bool is_same_type(NodeBase* _node) const
    {
        return type() == _node->type() && sym_id() == _node->sym_id();
    }
};

template<NodeBase::type_e>
class LeafValueType;

template<>
class LeafValueType<NodeBase::INT> { public: typedef long type; };
template<>
class LeafValueType<NodeBase::FLOAT> { public: typedef float32 type; };
template<>
class LeafValueType<NodeBase::STRING> { public: typedef std::string type; };
template<>
class LeafValueType<NodeBase::CHAR> { public: typedef char type; };
template<>
class LeafValueType<NodeBase::IDENT> { public: typedef const std::string* type; };

template<NodeBase::type_e type>
class LeafNodeBase
{
public:
    virtual typename LeafValueType<type>::type value() const = 0;
};

class InnerNodeBase
{
public:
    virtual std::string name() const = 0;
    virtual NodeBase* child(uint32 index) const = 0;
    virtual size_t child_count() const = 0;
};

}

#endif
