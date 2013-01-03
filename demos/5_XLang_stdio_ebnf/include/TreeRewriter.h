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

#ifndef TREE_REWRITER_H_
#define TREE_REWRITER_H_

namespace xl { namespace node { class NodeIdentIFace; } }
namespace xl { namespace visitor { class TraverseDFS; } }
namespace xl { class TreeContext; }

void rewrite_tree_until_stable(
        xl::node::NodeIdentIFace* ast,
        xl::visitor::TraverseDFS* v,
        xl::TreeContext* tc);

#endif
