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
#include <map> // std::map
#include <string> // std::string

void permute_lexer_id_map(
        std::map<std::string, std::vector<uint32_t>>* all_lexer_id_map,
        std::map<std::string, uint32_t>*              one_lexer_id_map);

void try_all_parses(
        xl::Allocator &alloc,
        std::string input,
        std::vector<std::string> *input_vec,
        std::vector<xl::node::NodeIdentIFace*> *ast_vec);

#endif
