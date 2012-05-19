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

#ifndef XLANG_NODE_IFACE_H_
#define XLANG_NODE_IFACE_H_

#include "visitor/XLangVisitableIFace.h" // visitor::VisitableIFace
#include "XLangType.h" // uint32_t
#include <string> // std::string

namespace node {

struct NodeIdentIFace
{
    typedef enum { INT, FLOAT, STRING, CHAR, IDENT, INNER } type_id_t;

    virtual ~NodeIdentIFace() {}
    virtual type_id_t type_id() const = 0;
    virtual uint32_t sym_id() const = 0;
    virtual std::string name() const = 0;
    bool is_same_type(const NodeIdentIFace* _node) const
    {
        return type_id() == _node->type_id() && sym_id() == _node->sym_id();
    }
};

template<NodeIdentIFace::type_id_t>
struct LeafTraitsType;
template<>
struct LeafTraitsType<NodeIdentIFace::INT> { typedef long type; };
template<>
struct LeafTraitsType<NodeIdentIFace::FLOAT> { typedef float32_t type; };
template<>
struct LeafTraitsType<NodeIdentIFace::STRING> { typedef std::string type; };
template<>
struct LeafTraitsType<NodeIdentIFace::CHAR> { typedef char type; };
template<>
struct LeafTraitsType<NodeIdentIFace::IDENT> { typedef const std::string* type; };

template<class T>
struct LeafTraitsTypeID;
template<>
struct LeafTraitsTypeID<long> { enum { type_id = NodeIdentIFace::INT}; };
template<>
struct LeafTraitsTypeID<float32_t> { enum { type_id = NodeIdentIFace::FLOAT}; };
template<>
struct LeafTraitsTypeID<std::string> { enum { type_id = NodeIdentIFace::STRING}; };
template<>
struct LeafTraitsTypeID<char> { enum { type_id = NodeIdentIFace::CHAR}; };
template<>
struct LeafTraitsTypeID<const std::string*> { enum { type_id = NodeIdentIFace::IDENT}; };

template<NodeIdentIFace::type_id_t T>
struct LeafNodeIFace : virtual public NodeIdentIFace
{
    virtual ~LeafNodeIFace() {}
    virtual typename LeafTraitsType<T>::type value() const = 0;
};

struct InnerNodeIFace : virtual public NodeIdentIFace, public visitor::VisitableIFace
{
    virtual ~InnerNodeIFace() {}
    virtual NodeIdentIFace* operator[](uint32_t index) const = 0;
    virtual size_t size() const = 0;
};

}

#endif
