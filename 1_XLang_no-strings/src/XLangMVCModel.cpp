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

#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "XLangParserContextIFace.h" // ParserContextIFace
#include "node/XLangNode.h" // node::NodeIdentIFace
#include "XLangType.h" // uint32_t
#include <stdarg.h> // va_list
#include <string> // std::string

#define TIXML_USE_TICPP
#ifdef TIXML_USE_TICPP
	#include <ticpp/ticpp.h>
#endif

namespace mvc {

node::NodeIdentIFace* MVCModel::make_ast(Allocator &alloc, std::string filename)
{
#ifdef TIXML_USE_TICPP
	ticpp::Document doc(filename.c_str());
	doc.LoadFile();
#endif
	return NULL;
}

node::NodeIdentIFace* MVCModel::make_inner(ParserContextIFace* pc, uint32_t sym_id, YYLTYPE &loc, size_t size, ...)
{
    va_list ap;
    va_start(ap, size);
    node::NodeIdentIFace* node = new (pc->alloc(), __FILE__, __LINE__, [](void* x) {
			reinterpret_cast<node::NodeIdentIFace*>(x)->~NodeIdentIFace();
			}) node::InnerNode(sym_id, loc, size, ap);
    va_end(ap);
    return node;
}

}
