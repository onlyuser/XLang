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
#include <stack> // std::stack
#include <map> // std::map
#include <string> // std::string

void permute_lexer_id_map(
        std::map<std::string, std::vector<uint32_t>>* lexer_id_maps,
        std::map<std::string, uint32_t>*              lexer_id_map);

// eats shoots and leaves
// V    V      C   V
//      N          N

// flying saucers are dangerous
// Adj    N       Aux Adj
// V

void build_pos_permutations(
        std::list<std::vector<int>>           &pos_permutations,           // OUT
        std::vector<std::vector<std::string>> &sentence_pos_options_table, // IN
        std::stack<int>                       &sentence_pos_indices,       // TEMP
        int                                    word_index);                // TEMP

void build_pos_permutations(
        std::list<std::vector<int>>           &pos_permutations,            // OUT
        std::vector<std::vector<std::string>> &sentence_pos_options_table); // IN

bool get_wordnet_pos(std::string word, std::vector<std::string> &word_pos);
void test_build_pos_permutations();
void test_build_pos_permutations(std::string s);

#endif
