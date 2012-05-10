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

#ifndef XLANG_MVC_MODEL_H_
#define XLANG_MVC_MODEL_H_

#include "XLangAlloc.h"
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "node/XLangNode.h" // node::LeafNode
#include "XLangTreeContext.h" // TreeContext
#include "XLangType.h" // uint32_t
#include "XLang.tab.h" // YYLTYPE
#include <string> // std::string

namespace mvc {

struct MVCModel
{
    template<class T>
    static node::NodeIdentIFace* make_leaf(TreeContext* tc, uint32_t sym_id, YYLTYPE &loc, T value)
    {
        return new (tc->alloc(), __FILE__, __LINE__) node::LeafNode<
                static_cast<node::NodeIdentIFace::type_id_t>(node::LeafTraitsTypeID<T>::type_id)
                >(sym_id, loc, value);
    }
    static node::InnerNode* make_inner(TreeContext* tc, uint32_t sym_id, YYLTYPE &loc, size_t size, ...);
    static node::NodeIdentIFace* make_ast(TreeContext* tc, std::string filename);
};

}

#endif
