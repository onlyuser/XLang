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

#include "visitor/XLangNodeVisitableIFace.h" // visitor::NodeVisitableIFace
#include "XLangType.h" // uint32_t
#include <string> // std::string

namespace node {

struct NodeIdentIFace
{
    typedef enum { INT, FLOAT, STRING, CHAR, IDENT, INNER } type_e;

    virtual ~NodeIdentIFace() {}
    virtual type_e type() const = 0;
    virtual uint32_t sym_id() const = 0;
    virtual std::string name() const = 0;
    bool is_same_type(const NodeIdentIFace* _node) const
    {
        return type() == _node->type() && sym_id() == _node->sym_id();
    }
};

template<NodeIdentIFace::type_e>
struct LeafTypeTraits;
template<>
struct LeafTypeTraits<NodeIdentIFace::INT> { typedef long type; };
template<>
struct LeafTypeTraits<NodeIdentIFace::FLOAT> { typedef float32_t type; };
template<>
struct LeafTypeTraits<NodeIdentIFace::STRING> { typedef std::string type; };
template<>
struct LeafTypeTraits<NodeIdentIFace::CHAR> { typedef char type; };
template<>
struct LeafTypeTraits<NodeIdentIFace::IDENT> { typedef const std::string* type; };

template<class T>
struct LeafTypeTraitsR;
template<>
struct LeafTypeTraitsR<long> { enum { value = NodeIdentIFace::INT}; };
template<>
struct LeafTypeTraitsR<float32_t> { enum { value = NodeIdentIFace::FLOAT}; };
template<>
struct LeafTypeTraitsR<std::string> { enum { value = NodeIdentIFace::STRING}; };
template<>
struct LeafTypeTraitsR<char> { enum { value = NodeIdentIFace::CHAR}; };
template<>
struct LeafTypeTraitsR<const std::string*> { enum { value = NodeIdentIFace::IDENT}; };

template<NodeIdentIFace::type_e T>
struct LeafNodeIFace : virtual public NodeIdentIFace
{
    virtual ~LeafNodeIFace() {}
    virtual typename LeafTypeTraits<T>::type value() const = 0;
};

struct InnerNodeIFace : virtual public NodeIdentIFace, public visitor::NodeVisitableIFace
{
    virtual ~InnerNodeIFace() {}
    virtual NodeIdentIFace* operator[](uint32_t index) const = 0;
    virtual size_t size() const = 0;
};

}

#endif
