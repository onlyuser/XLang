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

#include "EBNFPrinter.h" // EBNFPrinter
#include "XLang.tab.h" // ID_XXX (yacc generated)
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "node/XLangNode.h" // node::Node
#include "XLangTreeContext.h" // TreeContext
#include "XLangString.h" // xl::escape
#include <iostream> // std::cout
#include <string> // std::string
#include <vector> // std::vector
#include <map> // std::map

//#define DEBUG_EBNF
#ifdef DEBUG_EBNF
    static std::string ptr_to_string(const void* x)
    {
        std::stringstream ss;
        ss << '_' << x;
        std::string s = ss.str();
        return s;
    }

    static bool is_kleene_node(const xl::node::NodeIdentIFace* _node)
    {
        switch(_node->sym_id())
        {
            case '+':
            case '*':
            case '?':
                return true;
        }
        return false;
    }
#endif

#define MAKE_TERM(sym_id, ...) xl::mvc::MVCModel::make_term(tc, sym_id, ##__VA_ARGS__)
#if 0 // NOTE: macro recursion not allowed
    #define MAKE_SYMBOL(...) xl::mvc::MVCModel::make_symbol(tc, ##__VA_ARGS__)
#endif
#define MAKE_SYMBOL xl::mvc::MVCModel::make_symbol

#define CHILD_OF(x) ((*x)[0])

static std::string gen_name(std::string stem)
{
    static std::map<std::string, int> tally_map;
    std::stringstream ss;
    ss << stem << '_' << tally_map[stem]++;
    return ss.str();
}

static std::string gen_vec_name(std::string stem)
{
    return stem + "_vec";
}

static xl::node::NodeIdentIFace* find_clone_of_original_recursive(
        const xl::node::NodeIdentIFace* root,
        const xl::node::NodeIdentIFace* original)
{
    if(!root || !original)
        return NULL;
    auto root_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(root);
    if(!root_symbol)
        return NULL;
    static const xl::node::NodeIdentIFace* temp; // NOTE: must be static for compile-time closure!
    temp = original; // NOTE: do not combine with previous line! -- must assign every time
    xl::node::NodeIdentIFace* result = root_symbol->find_if([](const xl::node::NodeIdentIFace* _node) {
            return _node->original() == temp;
            });
    if(result)
        return result;
    for(size_t i = 0; i < root_symbol->size(); i++)
    {
        result = find_clone_of_original_recursive((*root_symbol)[i], original);
        if(result)
            return result;
    }
    return NULL;
}

static void replace_node(
        const xl::node::NodeIdentIFace* find_node,
        const xl::node::NodeIdentIFace* replacement_node)
{
    if(!find_node)
        return;
    auto parent_symbol =
            dynamic_cast<xl::node::SymbolNodeIFace*>(find_node->parent());
    if(parent_symbol)
    {
        parent_symbol->replace_first(
                const_cast<xl::node::NodeIdentIFace*>(find_node),
                const_cast<xl::node::NodeIdentIFace*>(replacement_node));
    }
}

static const xl::node::NodeIdentIFace* get_ancestor_node(
        uint32_t sym_id,
        const xl::node::NodeIdentIFace* node)
{
    for(auto p = node; p; p = p->parent())
    {
        if(p->sym_id() == sym_id)
            return p;
    }
    return NULL;
}

static std::string* get_string_ptr_from_term_node(const xl::node::NodeIdentIFace* term_node)
{
    switch(term_node->type())
    {
        case xl::node::NodeIdentIFace::IDENT:
            {
                auto ident_term =
                        dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::IDENT>*>(term_node);
                if(!ident_term)
                    return NULL;
                return const_cast<std::string*>(ident_term->value()); // TODO: fix-me!
            }
        case xl::node::NodeIdentIFace::STRING:
            {
                auto string_term =
                        dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(term_node);
                if(!string_term)
                    return NULL;
                return string_term->value();
            }
        default:
            return NULL;
    }
}

static std::string get_string_from_term_node(const xl::node::NodeIdentIFace* term_node)
{
    if(term_node->type() == xl::node::NodeIdentIFace::CHAR)
    {
        auto char_term =
                dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::CHAR>*>(term_node);
        std::stringstream ss;
        ss << '\'' << xl::escape(char_term->value()) << '\'';
        return ss.str();
    }
    std::string* string_ptr = get_string_ptr_from_term_node(term_node);
    if(!string_ptr)
        return "";
    return *string_ptr;
}

// kleene_node --> paren_node --> alts_node
static const xl::node::NodeIdentIFace* get_alts_node_from_kleene_node(
        const xl::node::NodeIdentIFace* kleene_node)
{
    //<symbol type="*">            // <-- kleene_node
    //    <symbol type="(">        // <-- paren_node
    //        <symbol type="alts"> // <-- alts_node
    //            <!-- ... -->
    //        </symbol>
    //    </symbol>
    //</symbol>

    if(!kleene_node)
        return NULL;
    auto kleene_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(kleene_node);
    if(!kleene_symbol)
        return NULL;
    xl::node::NodeIdentIFace* paren_node = CHILD_OF(kleene_symbol);
    if(!paren_node)
        return NULL;
    auto paren_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(paren_node);
    if(!paren_symbol)
        return NULL;
    return CHILD_OF(paren_symbol);
}

// kleene_node --> alt_node (up) --> action_node
static std::string* get_action_string_from_kleene_node(
        const xl::node::NodeIdentIFace* kleene_node)
{
    //<symbol type="alt"> // <-- alt_node
    //    <symbol type="terms">
    //        <symbol type="*"> // <-- kleene_node
    //            <!-- ... -->
    //        </symbol>
    //        <term type="ident" value=statement/>
    //    </symbol>
    //    <symbol type="action_block"> // <-- action_node
    //        <term type="string" value=" /* BBB */ ... "/>
    //    </symbol>
    //</symbol>

    if(!kleene_node)
        return NULL;
    const xl::node::NodeIdentIFace* alt_node = get_ancestor_node(ID_RULE_ALT, kleene_node);
    if(!alt_node)
        return NULL;
    auto alt_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(alt_node);
    if(!alt_symbol)
        return NULL;
    xl::node::NodeIdentIFace* action_node = (*alt_symbol)[1];
    if(!action_node)
        return NULL;
    auto action_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(action_node);
    if(!action_symbol)
        return NULL;
    xl::node::NodeIdentIFace* action_string_node = CHILD_OF(action_symbol);
    return get_string_ptr_from_term_node(action_string_node);
}

static std::string get_rule_name_from_rule_node(const xl::node::NodeIdentIFace* rule_node)
{
    if(!rule_node)
        return "";
    auto rule_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(rule_node);
    if(!rule_symbol)
        return "";
    xl::node::NodeIdentIFace* lhs_node = CHILD_OF(rule_symbol);
    if(!lhs_node)
        return "";
    std::string* lhs_value_ptr = get_string_ptr_from_term_node(lhs_node);
    if(!lhs_value_ptr)
        return "";
    return *lhs_value_ptr;
}

static std::string gen_new_delete_vector_stmt(int position);
static xl::node::NodeIdentIFace* make_stem_rule(
        std::string name,
        const xl::node::NodeIdentIFace* rule_node,
        const xl::node::NodeIdentIFace* kleene_node,
        xl::TreeContext* tc)
{
    //program:
    //      (
    //            statement ',' { /* AAA */ $$ = $1; }
    //      )* statement { /* BBB */ ... }
    //    ;
    //
    //<symbol type="rule">
    //    <term type="ident" value=program/>
    //    <symbol type="alts">
    //        <symbol type="alt">
    //            <symbol type="terms">
    //                <symbol type="*"> // <-- kleene_node
    //                    <symbol type="(">
    //                        <symbol type="alts">
    //                            <symbol type="alt">
    //                                <symbol type="terms">
    //                                    <term type="ident" value=statement/>
    //                                    <term type="char" value=','/>
    //                                </symbol>
    //                                <symbol type="action_block">
    //                                    <term type="string" value=" /* AAA */ $$ = $1; "/>
    //                                </symbol>
    //                            </symbol>
    //                        </symbol>
    //                    </symbol>
    //                </symbol>
    //                <term type="ident" value=statement/>
    //            </symbol>
    //            <symbol type="action_block">
    //                <term type="string" value=" /* BBB */ ... "/>
    //            </symbol>
    //        </symbol>
    //    </symbol>
    //</symbol>

    if(!rule_node || !kleene_node)
        return NULL;
    xl::node::NodeIdentIFace* rule_node_copy = rule_node->clone(tc);
    xl::node::NodeIdentIFace* kleene_node_copy =
            find_clone_of_original_recursive(rule_node_copy, kleene_node);
    std::string* action_string_ptr = get_action_string_from_kleene_node(kleene_node_copy);
    if(action_string_ptr)
    {
        int position = 1;
        xl::node::NodeIdentIFace* kleene_parent_node = kleene_node->parent();
        if(kleene_parent_node)
        {
            xl::node::SymbolNodeIFace* kleene_parent_symbol =
                    dynamic_cast<xl::node::SymbolNodeIFace*>(kleene_parent_node);
            if(kleene_parent_symbol)
            {
                for(size_t i = 0; i<kleene_parent_symbol->size(); i++)
                {
                    if((*kleene_parent_symbol)[i] == kleene_node)
                    {
                        position = i+1;
                        break;
                    }
                }
            }
        }
        action_string_ptr->append(gen_new_delete_vector_stmt(position));
    }
    xl::node::NodeIdentIFace* replacement_node =
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name));
    replace_node(kleene_node_copy, replacement_node);
    return rule_node_copy;
}

static xl::node::NodeIdentIFace* make_recursive_rule_plus(std::string name1, std::string name2,
        xl::TreeContext* tc)
{
    return NULL;
}

static xl::node::NodeIdentIFace* make_recursive_rule_star(std::string name1, std::string name2,
        std::string vector_inner_type, xl::TreeContext* tc)
{
    //program_0:
    //      /* empty */ {
    //                /* AAA */
    //                $$ = new std::vector<
    //                        /* vector_inner_type */
    //                        xl::node::TermInternalType<xl::node::NodeIdentIFace::SYMBOL>::type
    //                        >;
    //            }
    //    | program_0 program_1 { /* BBB */ $1->push_back($2); $$ = $1; }
    //    ;
    //
    //<symbol type="rule">
    //    <term type="ident" value=program_0/>
    //    <symbol type="alts">
    //        <symbol type="alt">
    //            <symbol type="action_block">
    //                <term type="string" value=" /* AAA */ ... "/>
    //            </symbol>
    //        </symbol>
    //        <symbol type="alt">
    //            <symbol type="terms">
    //                <term type="ident" value=program_0/>
    //                <term type="ident" value=program_1/>
    //            </symbol>
    //            <symbol type="action_block">
    //                <term type="string" value=" /* BBB */ ... "/>
    //            </symbol>
    //        </symbol>
    //    </symbol>
    //</symbol>

    xl::node::NodeIdentIFace* node =
            MAKE_SYMBOL(tc, ID_RULE, 2,
                    MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name1)),
                    MAKE_SYMBOL(tc, ID_RULE_ALTS, 2,
                            MAKE_SYMBOL(tc, ID_RULE_ALT, 1,
                                    MAKE_SYMBOL(tc, ID_RULE_ACTION_BLOCK, 1,
                                            MAKE_TERM(ID_STRING,
                                                    tc->alloc_string(" $$ = new std::vector<"
                                                            + vector_inner_type + ">; ")
                                                    )
                                            )
                                    ),
                            MAKE_SYMBOL(tc, ID_RULE_ALT, 2,
                                    MAKE_SYMBOL(tc, ID_RULE_TERMS, 2,
                                            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name1)),
                                            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name2))
                                            ),
                                    MAKE_SYMBOL(tc, ID_RULE_ACTION_BLOCK, 1,
                                            MAKE_TERM(ID_STRING,
                                                    tc->alloc_string(" $1->push_back(*$2); delete $2; $$ = $1; ")
                                                    )
                                            )
                                    )
                            )
                    );
    return node;
}

static xl::node::NodeIdentIFace* make_recursive_rule_optional(std::string name1, std::string name2,
        xl::TreeContext* tc)
{
    //statement_0:
    //      /* empty */ { /* AAA */ $$ = NULL; }
    //    | statement_1 { /* BBB */ $$ = $1; }
    //    ;
    //
    //<symbol type="rule">
    //    <term type="ident" value=statement_0/>
    //    <symbol type="alts">
    //        <symbol type="alt">
    //            <symbol type="action_block">
    //                <term type="string" value=" /* AAA */ ... "/>
    //            </symbol>
    //        </symbol>
    //        <symbol type="alt">
    //            <symbol type="terms">
    //                <term type="ident" value=statement_1/>
    //            </symbol>
    //            <symbol type="action_block">
    //                <term type="string" value=" /* BBB */ ... "/>
    //            </symbol>
    //        </symbol>
    //    </symbol>
    //</symbol>

    xl::node::NodeIdentIFace* node =
            MAKE_SYMBOL(tc, ID_RULE, 2,
                    MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name1)),
                    MAKE_SYMBOL(tc, ID_RULE_ALTS, 2,
                            MAKE_SYMBOL(tc, ID_RULE_ALT, 1,
                                    MAKE_SYMBOL(tc, ID_RULE_ACTION_BLOCK, 1,
                                            MAKE_TERM(ID_STRING, tc->alloc_string(" $$ = NULL; "))
                                            )
                                    ),
                            MAKE_SYMBOL(tc, ID_RULE_ALT, 2,
                                    MAKE_SYMBOL(tc, ID_RULE_TERMS, 1,
                                            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name2))
                                            ),
                                    MAKE_SYMBOL(tc, ID_RULE_ACTION_BLOCK, 1,
                                            MAKE_TERM(ID_STRING, tc->alloc_string(" $$ = $1; "))
                                            )
                                    )
                            )
                    );
    return node;
}

static xl::node::NodeIdentIFace* make_term_rule(
        std::string name,
        const xl::node::NodeIdentIFace* alt_node,
        xl::TreeContext* tc)
{
    if(!alt_node)
        return NULL;
    return MAKE_SYMBOL(tc, ID_RULE, 2,
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name)),
            alt_node->clone(tc));
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_new_include_header_stmt()
{
    return "#include <vector>";
}

// node to be appended to back of union_block_node
static xl::node::NodeIdentIFace* make_new_union_member_node(
        std::string type, std::string _type_name, xl::TreeContext* tc)
{
    // STRING:
    //std::vector< /* AAA */ type >* /* BBB */ _type_name;
    //
    // XML:
    //<symbol type="decl_stmt">
    //    <symbol type="decl_chunks">
    //        <symbol type="decl_chunk">
    //            <term type="string" value="std::vector< /* AAA */ type >*"/>
    //        </symbol>
    //        <symbol type="decl_chunk">
    //            <term type="string" value=" /* BBB */ _type_name "/>
    //        </symbol>
    //    </symbol>
    //</symbol>

    xl::node::NodeIdentIFace* node =
            MAKE_SYMBOL(tc, ID_UNION_MEMBER, 1,
                    MAKE_SYMBOL(tc, ID_UNION_TERMS, 2,
                            MAKE_SYMBOL(tc, ID_UNION_TERM, 1,
                                    MAKE_TERM(ID_STRING, tc->alloc_string("std::vector<" + type + ">"))
                                    ),
                            MAKE_SYMBOL(tc, ID_UNION_TERM, 1,
                                    MAKE_TERM(ID_STRING, tc->alloc_string(_type_name))
                                    )
                            )
                    );
    return node;
}

// node to be appended to back of definitions block
static xl::node::NodeIdentIFace* make_new_def_brace_node(
        std::string _type_name, std::vector<std::string> &token_vec, xl::TreeContext* tc)
{
    // STRING:
    //%type< /* AAA */ _type_name > /* BBB */ token_vec
    //
    // XML:
    //<symbol type="decl_brace">
    //    <term type="ident" value=type/>
    //    <term type="ident" value= /* AAA */ _type_name />
    //    <symbol type="symbols">
    //        <symbol type="symbol">
    //            <term type="ident" value= /* BBB */ token_vec />
    //        </symbol>
    //    </symbol>
    //</symbol>

    std::string exploded_tokens;
    for(auto p = token_vec.begin(); p != token_vec.end(); p++)
    {
        if((p+1) != token_vec.end())
            exploded_tokens.append(*p + ", ");
        else
            exploded_tokens.append(*p);
    }

    xl::node::NodeIdentIFace* node =
            MAKE_SYMBOL(tc, ID_DEF_BRACE, 3,
                    MAKE_TERM(ID_IDENT, tc->alloc_unique_string("type")),
                    MAKE_TERM(ID_IDENT, tc->alloc_unique_string(_type_name)),
                    MAKE_SYMBOL(tc, ID_DEF_SYMBOLS, 1,
                            MAKE_SYMBOL(tc, ID_DEF_SYMBOL, 1,
                                    MAKE_TERM(ID_IDENT, tc->alloc_unique_string(exploded_tokens))
                                    )
                            )
                    );
    return node;
}

// string to be appended to back of kleene closure action_block's string value
static std::string gen_new_delete_vector_stmt(int position)
{
    std::stringstream ss;
    ss << "delete $" << position << ";";
    return ss.str();
}

static void add_term_rule(
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_insertions_after,
        std::string                                                                      name2,
        const xl::node::NodeIdentIFace*                                                  kleene_node,
        const xl::node::NodeIdentIFace*                                                  rule_node,
        const xl::node::NodeIdentIFace*                                                  rule_definition_node,
        xl::TreeContext*                                                                 tc)
{
    const xl::node::NodeIdentIFace* alts_node = get_alts_node_from_kleene_node(kleene_node);
    xl::node::NodeIdentIFace* term_rule = make_term_rule(name2, alts_node, tc);
    if(term_rule)
    {
#ifdef DEBUG_EBNF
        std::cout << ">>> (term_rule)" << std::endl;
        EBNFPrinter v(tc); v.visit_any(term_rule); std::cout << std::endl;
        std::cout << "<<< (term_rule)" << std::endl;
#endif
        if(rule_definition_node)
            (*node_insertions_after)[rule_definition_node].push_back(
                    MAKE_SYMBOL(tc, ID_DEF_SYMBOL, 1,
                            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name2))
                            )
                    );
        if(node_insertions_after)
            (*node_insertions_after)[rule_node].push_back(term_rule);
    }
}

static void add_recursive_rule(
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_insertions_after,
        std::string                                                                      name1,
        std::string                                                                      name2,
        const xl::node::NodeIdentIFace*                                                  kleene_node,
        const xl::node::NodeIdentIFace*                                                  rule_node,
        std::string                                                                      rule_type,
        xl::TreeContext*                                                                 tc)
{
    xl::node::NodeIdentIFace* recursive_rule = NULL;
    switch(kleene_node->sym_id())
    {
        case '+': recursive_rule = make_recursive_rule_plus(name1, name2, tc); break;
        case '*':
            {
                std::string* action_string_ptr = get_action_string_from_kleene_node(kleene_node);
                if(!action_string_ptr)
                    break;
                std::string action_string = *action_string_ptr;
                recursive_rule = make_recursive_rule_star(name1, name2, rule_type, tc);
                break;
            }
        case '?': recursive_rule = make_recursive_rule_optional(name1, name2, tc); break;
    }
    if(recursive_rule)
    {
#ifdef DEBUG_EBNF
        std::cout << ">>> (recursive_rule)" << std::endl;
        EBNFPrinter v(tc); v.visit_any(recursive_rule); std::cout << std::endl;
        std::cout << "<<< (recursive_rule)" << std::endl;
#endif
        if(node_insertions_after)
            (*node_insertions_after)[rule_node].push_back(recursive_rule);
    }
}

static void add_stem_rule(
        std::map<const xl::node::NodeIdentIFace*, xl::node::NodeIdentIFace*>* node_replacements,
        std::string                                                           name1,
        const xl::node::NodeIdentIFace*                                       kleene_node,
        const xl::node::NodeIdentIFace*                                       rule_node,
        xl::TreeContext*                                                      tc)
{
    xl::node::NodeIdentIFace* stem_rule = make_stem_rule(name1, rule_node, kleene_node, tc);
    if(stem_rule)
    {
#ifdef DEBUG_EBNF
        std::cout << ">>> (stem_rule)" << std::endl;
        EBNFPrinter v(tc); v.visit_any(stem_rule); std::cout << std::endl;
        std::cout << "<<< (stem_rule)" << std::endl;
#endif
        if(node_replacements)
            (*node_replacements)[rule_node] = stem_rule;
    }
}

static void enqueue_changes_for_kleene_closure(
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_insertions_after,
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_appends_to_back,
        std::map<const xl::node::NodeIdentIFace*, std::list<std::string>>*               string_appends_to_back,
        std::map<const xl::node::NodeIdentIFace*, std::list<std::string>>*               string_insertions_to_front,
        std::map<const xl::node::NodeIdentIFace*, xl::node::NodeIdentIFace*>*            node_replacements,
        const xl::node::NodeIdentIFace*                                                  kleene_node,
        const xl::node::NodeIdentIFace*                                                  definitions_node,
        const xl::node::NodeIdentIFace*                                                  proto_block_node,
        const xl::node::NodeIdentIFace*                                                  union_block_node,
        std::map<std::string, const xl::node::NodeIdentIFace*>*                          def_symbol_name_to_symbol,
        std::map<std::string, std::string>*                                              union_typename_to_type,
        std::map<std::string, std::string>*                                              def_symbol_name_to_union_typename,
        xl::TreeContext*                                                                 tc)
{
    if(string_insertions_to_front)
    {
        auto proto_block_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(proto_block_node);
        if(proto_block_symbol)
        {
            const xl::node::NodeIdentIFace* term_node = CHILD_OF(proto_block_symbol);
            if(term_node)
            {
                std::string proto_block_string = get_string_from_term_node(term_node);
                std::string include_header_stmt = gen_new_include_header_stmt();
                if(proto_block_string.find(include_header_stmt) == std::string::npos)
                    (*string_insertions_to_front)[term_node].push_back(include_header_stmt);
            }
        }
    }
    const xl::node::NodeIdentIFace* rule_node = get_ancestor_node(ID_RULE, kleene_node);
    std::string rule_name = get_rule_name_from_rule_node(rule_node);
    std::string rule_typename = (*def_symbol_name_to_union_typename)[rule_name];
    std::string rule_type = (*union_typename_to_type)[rule_typename];
    std::string name1 = gen_name(rule_name);
    std::string name2 = gen_name(rule_name);
    if(node_appends_to_back)
    {
        auto union_block_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(union_block_node);
        if(union_block_symbol)
        {
            const xl::node::NodeIdentIFace* union_members_node = CHILD_OF(union_block_symbol);
            if(union_members_node)
            {
                auto union_members_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(union_members_node);
                if(union_members_symbol)
                {
                    xl::node::NodeIdentIFace* union_member_node =
                            make_new_union_member_node(rule_type, gen_vec_name(rule_typename), tc);
                    if(union_member_node)
                    {
                        if(!union_members_symbol->find(union_member_node))
                            (*node_appends_to_back)[union_members_symbol].push_back(union_member_node);
                    }
                }
            }
        }
        auto definitions_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(definitions_node);
        if(definitions_symbol)
        {
            std::vector<std::string> token_vec;
            token_vec.push_back(name1);
            xl::node::NodeIdentIFace* def_brace_node =
                    make_new_def_brace_node(gen_vec_name(rule_typename), token_vec, tc);
            if(!definitions_symbol->find(def_brace_node))
                (*node_appends_to_back)[definitions_symbol].push_back(def_brace_node);
        }
    }
    const xl::node::NodeIdentIFace* rule_definition_node = (*def_symbol_name_to_symbol)[rule_name];
    add_term_rule(
            node_insertions_after,
            name2,
            kleene_node,
            rule_node,
            rule_definition_node,
            tc);
    add_recursive_rule(
            node_insertions_after,
            name1, name2,
            kleene_node,
            rule_node,
            rule_type,
            tc);
    add_stem_rule(
            node_replacements,
            name1,
            kleene_node,
            rule_node,
            tc);
}

void EBNFPrinter::visit(const xl::node::SymbolNodeIFace* _node)
{
#ifdef DEBUG_EBNF
    if(_node->sym_id() == ID_RULE || is_kleene_node(_node))
        std::cout << '[';
#endif
    static bool entered_kleene_closure = false;
    static const xl::node::NodeIdentIFace *definitions_node = NULL, *proto_block_node = NULL,
            *union_block_node = NULL;
    static std::map<std::string, const xl::node::NodeIdentIFace*> def_symbol_name_to_symbol;
    static std::map<std::string, std::string> union_typename_to_type, def_symbol_name_to_union_typename;
    static std::vector<std::string> union_term_names, def_symbol_names;
    bool more;
    switch(_node->sym_id())
    {
        case ID_GRAMMAR:
            {
                entered_kleene_closure = false;
                definitions_node = NULL;
                proto_block_node = NULL;
                union_block_node = NULL;
                def_symbol_name_to_symbol.clear();
                union_typename_to_type.clear();
                def_symbol_name_to_union_typename.clear();
            }
            visit_next_child(_node);
            std::cout << std::endl << std::endl << "%%" << std::endl << std::endl;
            visit_next_child(_node);
            std::cout << std::endl << std::endl << "%%";
            visit_next_child(_node);
            std::cout << std::endl;
            break;
        case ID_DEFINITIONS:
            definitions_node = _node;
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl;
            } while(more);
            break;
        case ID_DEFINITION:
            std::cout << '%';
            more = visit_next_child(_node);
            if(more)
            {
                std::cout << ' ';
                visit_next_child(_node);
            }
            break;
        case ID_DEF_EQ:
            std::cout << '%';
            visit_next_child(_node);
            std::cout << '=';
            visit_next_child(_node);
            break;
        case ID_DEF_BRACE:
            {
                std::string union_typename;
                std::cout << '%';
                visit_next_child(_node);
                {
                    std::cout << '<';
                    xl::node::NodeIdentIFace* child = NULL;
                    visit_next_child(_node, &child);
                    if(child)
                        union_typename = get_string_from_term_node(child);
                    std::cout << "> ";
                }
                visit_next_child(_node);
                for(auto p = def_symbol_names.begin(); p != def_symbol_names.end(); p++)
                    def_symbol_name_to_union_typename[*p] = union_typename;
            }
            break;
        case ID_DEF_PROTO_BLOCK:
            proto_block_node = _node;
            std::cout << "%{";
            std::cout << get_string_from_term_node(CHILD_OF(_node));
            std::cout << "%}";
            break;
        case ID_UNION_BLOCK:
            union_block_node = _node;
            std::cout << std::endl << '{' << std::endl;
            visit_next_child(_node);
            std::cout << std::endl << '}';
            break;
        case ID_UNION_MEMBERS:
            do
            {
                std::cout << '\t';
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl;
            } while(more);
            break;
        case ID_UNION_MEMBER:
            visit_next_child(_node);
            std::cout << ';';
            break;
        case ID_UNION_TERMS:
            union_term_names.clear();
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << ' ';
            } while(more);
            if(!union_term_names.empty())
            {
                std::string union_type;
                for(size_t i = 0; i<union_term_names.size()-1; i++)
                    union_type.append(union_term_names[i]);
                std::string union_typename = union_term_names[union_term_names.size()-1];
                union_typename_to_type[union_typename] = union_type;
            }
            break;
        case ID_UNION_TERM:
            {
                std::string name = get_string_from_term_node(CHILD_OF(_node));
                std::cout << name;
                union_term_names.push_back(name);
            }
            break;
        case ID_DEF_SYMBOLS:
            def_symbol_names.clear();
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << ' ';
            } while(more);
            break;
        case ID_DEF_SYMBOL:
            {
                std::string symbol_name = get_string_from_term_node(CHILD_OF(_node));
                std::cout << symbol_name;
                if(!symbol_name.empty())
                {
                    def_symbol_name_to_symbol[symbol_name] = _node;
                    def_symbol_names.push_back(symbol_name);
                }
            }
            break;
        case ID_RULES:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl << std::endl;
            } while(more);
            break;
        case ID_RULE:
            visit_next_child(_node);
            std::cout << ':' << std::endl << "\t  ";
            visit_next_child(_node);
            std::cout << ';';
            break;
        case ID_RULE_ALTS:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl << "\t| ";
            } while(more);
            break;
        case ID_RULE_ALT:
            if(visit_next_child(_node))
                visit_next_child(_node);
            break;
        case ID_RULE_ACTION_BLOCK:
            std::cout << " {";
            std::cout << get_string_from_term_node(CHILD_OF(_node));
            std::cout << '}';
            break;
        case ID_RULE_TERMS:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << ' ';
            } while(more);
            break;
        case '+':
        case '*':
        case '?':
            if(!entered_kleene_closure)
            {
                if(m_changes)
                    enqueue_changes_for_kleene_closure(
                            &m_changes->m_node_insertions_after,
                            &m_changes->m_node_appends_to_back,
                            &m_changes->m_string_appends_to_back,
                            &m_changes->m_string_insertions_to_front,
                            &m_changes->m_node_replacements,
                            _node,
                            definitions_node,
                            proto_block_node,
                            union_block_node,
                            &def_symbol_name_to_symbol,
                            &union_typename_to_type,
                            &def_symbol_name_to_union_typename,
                            m_tc);
                entered_kleene_closure = true;
            }
            xl::visitor::DefaultTour::visit(_node);
            std::cout << static_cast<char>(_node->sym_id());
            break;
        case '(':
            std::cout << '(';
            xl::visitor::DefaultTour::visit(_node);
            std::cout << ')';
            break;
        case ID_CODE:
            std::cout << get_string_from_term_node(CHILD_OF(_node));
            break;
    }
#ifdef DEBUG_EBNF
    if(_node->sym_id() == ID_RULE || is_kleene_node(_node))
    {
        std::cout << '<'
                << _node->name() << "::"
                << ptr_to_string(dynamic_cast<const xl::node::NodeIdentIFace*>(_node)) << '>';
        std::cout << ']';
    }
#endif
}
