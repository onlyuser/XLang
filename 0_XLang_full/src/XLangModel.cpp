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

#include "XLangModel.h" // mvc::Model
#include "XLangParseContextBase.h" // ParseContextBase
#include "XLangNode.h" // node::NodeBase
#include "XLangType.h" // uint32
#include <stdarg.h> // va_list
#include <string> // std::string

namespace mvc {

node::NodeBase* Model::make_float(ParseContextBase* pc, uint32 sym_id, YYLTYPE &loc, float32 value)
{
    return new (pc->alloc(), __FILE__, __LINE__) node::LeafNode<float32>(sym_id, loc, value);
}

node::NodeBase* Model::make_string(ParseContextBase* pc, uint32 sym_id, YYLTYPE &loc, std::string value)
{
    return new (pc->alloc(), __FILE__, __LINE__) node::LeafNode<std::string>(sym_id, loc, value);
}

node::NodeBase* Model::make_char(ParseContextBase* pc, uint32 sym_id, YYLTYPE &loc, char value)
{
    return new (pc->alloc(), __FILE__, __LINE__) node::LeafNode<char>(sym_id, loc, value);
}

node::NodeBase* Model::make_ident(ParseContextBase* pc, uint32 sym_id, YYLTYPE &loc, const std::string* name)
{
    return new (pc->alloc(), __FILE__, __LINE__) node::LeafNode<const std::string*>(sym_id, loc, name);
}

node::NodeBase* Model::make_inner(ParseContextBase* pc, uint32 sym_id, YYLTYPE &loc, size_t child_count, ...)
{
    va_list ap;
    va_start(ap, child_count);
    node::NodeBase* node = new (pc->alloc(), __FILE__, __LINE__) node::InnerNode(sym_id, loc, child_count, ap);
    va_end(ap);
    return node;
}

}
