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
#include "node/XLangNodeBase.h" // node::NodeBase
#include "node/XLangNode.h" // node::LeafNode
#include "XLangParserContextBase.h" // ParserContextBase
#include "XLangType.h" // uint32
#include "XLang.tab.h" // YYLTYPE
#include <string> // std::string

namespace mvc {

class MVCModel
{
public:
    template<node::NodeBase::type_e type>
    static node::NodeBase* make_leaf(ParserContextBase* pc, uint32 sym_id, YYLTYPE &loc,
            typename node::LeafValueType<type>::type value)
    {
        return new (pc->alloc(), __FILE__, __LINE__) node::LeafNode<type>(sym_id, loc, value);
    }
    static node::NodeBase* make_inner(ParserContextBase* pc, uint32 sym_id, YYLTYPE &loc, size_t child_count, ...);
};

}

#endif
