// XLang
// -- A parser framework for language modeling
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

#ifndef TRY_ALL_PARSES_H_
#define TRY_ALL_PARSES_H_

#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLangAlloc.h" // Allocator
#include <vector> // std::vector

void try_all_parses(
        std::string input,
        xl::Allocator &alloc,
        std::vector<xl::node::NodeIdentIFace*> &ast_vec,
        std::vector<std::string> &input_vec);

#endif