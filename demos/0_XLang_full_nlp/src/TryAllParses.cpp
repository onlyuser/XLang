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

#include "TryAllParses.h"
#include "XLang.h"
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "XLangAlloc.h" // Allocator
#include "XLangString.h" // xl::tokenize
#include "XLangSystem.h" // xl::system::shell_capture
#include <vector> // std::vector
#include <list> // std::list
#include <stack> // std::stack
#include <string> // std::string
#include <iostream> // std::cout

bool get_pos_values_from_lexer(
        std::string               word,
        std::vector<std::string>* pos_values)
{
    if(word.empty() || !pos_values)
        return false;
    std::string pos_value;
    std::string word_alt = std::string("<") + word + ">";
    try
    {
        uint32_t lexer_id = quick_lex(word_alt.c_str());
        pos_value = id_to_name(lexer_id);
    }
    catch(const char* s)
    {
        std::cerr << "ERROR: " << s << std::endl;
        return false;
    }
    if(pos_value.empty())
        return false;
    pos_values->push_back(pos_value);
    return true;
}

bool get_pos_values_from_wordnet(
        std::string               word,
        std::vector<std::string>* pos_values)
{
    if(word.empty() || !pos_values)
        return false;
    std::string which_wn_stdout = xl::system::shell_capture("which wn");
    if(which_wn_stdout.empty())
    {
        std::cerr << "ERROR: wordnet not found" << std::endl;
        return false;
    }
    bool found_match = false;
    const char* wordnet_faml_types[] = {"n", "v", "a", "r"};
    const char* pos_values_arr[]     = {"Noun", "Verb", "Adj", "Adv"};
    for(int i = 0; i<4; i++)
    {
        std::string wordnet_stdout =
                xl::system::shell_capture("wn \"" + word + "\" -faml" + wordnet_faml_types[i]);
        if(wordnet_stdout.size())
        {
            pos_values->push_back(pos_values_arr[i]);
            found_match = true;
        }
    }
    return found_match;
}

bool get_pos_values(
        std::string               word,
        std::vector<std::string>* pos_values)
{
    if(word.empty() || !pos_values)
        return false;
    if(word == "and")
    {
        pos_values->push_back("Conj");
        pos_values->push_back("Conj_2");
        pos_values->push_back("Conj_3");
        return true;
    }
    std::set<std::string> unique_pos_values;
    bool found_in_lexer = false;
    {
        std::vector<std::string> pos_values_from_lexer;
        found_in_lexer = get_pos_values_from_lexer(word, &pos_values_from_lexer);
        if(found_in_lexer)
        {
            for(auto p = pos_values_from_lexer.begin(); p != pos_values_from_lexer.end(); p++)
                unique_pos_values.insert(*p);
        }
    }
    bool found_in_wordnet = false;
    {
        std::vector<std::string> pos_values_from_wordnet;
        found_in_wordnet = get_pos_values_from_wordnet(word, &pos_values_from_wordnet);
        if(found_in_wordnet)
        {
            for(auto q = pos_values_from_wordnet.begin(); q != pos_values_from_wordnet.end(); q++)
                unique_pos_values.insert(*q);
        }
    }
    if(found_in_lexer || found_in_wordnet)
    {
        for(auto r = unique_pos_values.begin(); r != unique_pos_values.end(); r++)
            pos_values->push_back(*r);
    }
    return pos_values->size();
}

void build_pos_paths_from_pos_options(
        std::list<std::vector<int>>*                 pos_paths,                  // OUT
        const std::vector<std::vector<std::string>> &sentence_pos_options_table, // IN
        std::stack<int>*                             pos_path,                   // TEMP
        int                                          word_index)                 // TEMP
{
    if(!pos_paths || !pos_path)
        return;
    if(static_cast<size_t>(word_index) >= sentence_pos_options_table.size())
    {
        size_t n = pos_path->size();
        std::vector<int> pos_path_vec(n);
        for(int i = 0; i < static_cast<int>(n); i++)
        {
            pos_path_vec[n-i-1] = pos_path->top();
            pos_path->pop();
        }
        pos_paths->push_back(pos_path_vec);
        for(auto q = pos_path_vec.begin(); q != pos_path_vec.end(); q++)
            pos_path->push(*q);
        return;
    }
    const std::vector<std::string> &word_pos_options = sentence_pos_options_table[word_index];
    int pos_index = 0;
    for(auto p = word_pos_options.begin(); p != word_pos_options.end(); p++)
    {
        pos_path->push(pos_index);
        build_pos_paths_from_pos_options(
                pos_paths,
                sentence_pos_options_table,
                pos_path,
                word_index+1);
        pos_path->pop();
        pos_index++;
    }
}

void build_pos_paths_from_pos_options(
        std::list<std::vector<int>>*                 pos_paths,                  // OUT
        const std::vector<std::vector<std::string>> &sentence_pos_options_table) // IN
{
    if(!pos_paths)
        return;
    std::stack<int> pos_path;
    int word_index = 0;
    build_pos_paths_from_pos_options(
            pos_paths,
            sentence_pos_options_table,
            &pos_path,
            word_index);
}

void build_pos_value_paths_from_sentence(
        std::list<std::vector<std::string>>* pos_value_paths, // OUT
        std::string                          sentence)        // IN
{
    if(!pos_value_paths)
        return;
    std::vector<std::vector<std::string>> sentence_pos_options_table;
    std::vector<std::string> words = xl::tokenize(sentence);
    sentence_pos_options_table.resize(words.size());
    int word_index = 0;
    for(auto t = words.begin(); t != words.end(); t++)
    {
        std::cout << *t << "<";
        std::vector<std::string> pos_values;
        get_pos_values(*t, &pos_values);
        for(auto r = pos_values.begin(); r != pos_values.end(); r++)
        {
            sentence_pos_options_table[word_index].push_back(*r);
            std::cout << *r << " ";
        }
        std::cout << ">" << std::endl;
        word_index++;
    }
    std::list<std::vector<int>> pos_paths;
    build_pos_paths_from_pos_options(&pos_paths, sentence_pos_options_table);
    int path_index = 0;
    for(auto p = pos_paths.begin(); p != pos_paths.end(); p++)
    {
        std::cout << "path #" << path_index << ": ";
        std::vector<std::string> pos_value_path;
        int word_index = 0;
        auto pos_indices = *p;
        for(auto q = pos_indices.begin(); q != pos_indices.end(); q++)
        {
            std::string pos_value = sentence_pos_options_table[word_index][*q];
            pos_value_path.push_back(pos_value);
            std::cout << pos_value << " ";
            word_index++;
        }
        std::cout << std::endl;
        pos_value_paths->push_back(pos_value_path);
        path_index++;
    }
}

void test_build_pos_value_paths()
{
    std::list<std::vector<std::string>> pos_value_paths;
    build_pos_value_paths_from_sentence(&pos_value_paths, "eats shoots and leaves");
    //test_build_pos_paths("flying saucers are dangerous", pos_paths);
}
