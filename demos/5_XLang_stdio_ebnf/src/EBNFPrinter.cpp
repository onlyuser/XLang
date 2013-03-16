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
#include "TreeChanges.h" // TreeChanges
#include "XLangString.h" // xl::escape
#include <iostream> // std::cout
#include <string> // std::string
#include <vector> // std::vector
#include <map> // std::map
#include <stdarg.h> // va_list
#include <assert.h> // assert

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
        assert(_node);

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

#define RECURSIVE_NAME_SUFFIX "_recursive" // +, *
#define OPTIONAL_NAME_SUFFIX  "_optional"  // ?
#define PAREN_NAME_SUFFIX     "_paren"     // ()
#define TERM_NAME_SUFFIX      "_term"
#define INTERNAL_NAME_SUFFIX  "_internal"
#define TYPENAME_SUFFIX       "_type"
#define TYPE_SUFFIX           "_type_t"

static const xl::node::NodeIdentIFace* get_child(const xl::node::NodeIdentIFace* _node)
{
    assert(_node);

    auto symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(_node);
    if(!symbol)
        return NULL;
    assert(symbol->size() == 1);
    return (*symbol)[0];
}

static const xl::node::NodeIdentIFace* get_left_child(const xl::node::NodeIdentIFace* _node)
{
    assert(_node);

    auto symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(_node);
    if(!symbol)
        return NULL;
    assert(symbol->size() == 2);
    return (*symbol)[0];
}

static const xl::node::NodeIdentIFace* get_right_child(const xl::node::NodeIdentIFace* _node)
{
    assert(_node);

    auto symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(_node);
    if(!symbol)
        return NULL;
    assert(symbol->size() == 2);
    return (*symbol)[1];
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_tuple_include_headers()
{
    return "#include <tuple>";
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_variant_include_headers()
{
    return "#include <boost/variant.hpp>";
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_vector_include_headers()
{
    return "#include <vector>";
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_typedef(
        std::string _type,
        std::string _typename)
{
    return std::string("typedef ") + _type + " " + _typename + ";";
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_template_type(
        std::string               template_type,
        std::vector<std::string>* type_vec)
{
    assert(type_vec);

    std::string exploded_types;
    for(auto p = type_vec->begin(); p != type_vec->end(); p++)
    {
        exploded_types.append(*p);
        if((p+1) != type_vec->end())
            exploded_types.append(", ");
    }
    return std::string(template_type) + "<" + exploded_types + ">";
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_tuple_type(std::vector<std::string>* type_vec)
{
    assert(type_vec);

    return gen_template_type("std::tuple", type_vec);
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_variant_type(std::vector<std::string>* type_vec)
{
    assert(type_vec);

    return gen_template_type("boost::variant", type_vec);
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_vector_typedef(std::string _type, std::string _typename)
{
    return gen_typedef("std::vector<" + _type + ">", _typename);
}

// string to be appended to back of kleene closure action_block's string value
static std::string gen_delete_rule_rvalue_term(int position)
{
    std::stringstream ss;
    ss << "delete $" << position << ";";
    return ss.str();
}

static std::string gen_name(std::string stem)
{
    static std::map<std::string, int> tally_map;
    std::stringstream ss;
    ss << stem << '_' << tally_map[stem]++;
    return ss.str();
}

static std::string gen_typename(std::string stem)
{
    return stem + TYPENAME_SUFFIX;
}

static std::string gen_type(std::string stem)
{
    return stem + TYPE_SUFFIX;
}

static xl::node::NodeIdentIFace* find_node_recursive(
        const xl::node::NodeIdentIFace* root_node,
        const xl::node::NodeIdentIFace* find_node)
{
    assert(root_node);
    assert(find_node);

    auto root_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(root_node);
    if(!root_symbol)
        return NULL;
    static const xl::node::NodeIdentIFace* temp_node; // NOTE: must be static for compile-time closure!
    temp_node = find_node; // NOTE: do not combine with previous line! -- must assign every time
    xl::node::NodeIdentIFace* clone_node = root_symbol->find_if([](const xl::node::NodeIdentIFace* _node) {
            return _node ? (_node->original() == temp_node) : false;
            });
    if(clone_node)
        return clone_node;
    for(size_t i = 0; i<root_symbol->size(); i++)
    {
        clone_node = find_node_recursive((*root_symbol)[i], find_node);
        if(clone_node)
            return clone_node;
    }
    return NULL;
}

static void replace_node(
        xl::node::NodeIdentIFace* find_node,
        xl::node::NodeIdentIFace* replacement_node)
{
    assert(find_node);
    assert(replacement_node);

    auto parent_symbol = dynamic_cast<xl::node::SymbolNodeIFace*>(find_node->parent());
    if(parent_symbol)
        parent_symbol->replace_first(find_node, replacement_node);
}

static const xl::node::NodeIdentIFace* get_ancestor_node(
        uint32_t sym_id,
        const xl::node::NodeIdentIFace* _node)
{
    assert(_node);

    for(auto p = _node; p; p = p->parent())
    {
        if(p->sym_id() == sym_id)
            return p;
    }
    return NULL;
}

static std::string* get_string_ptr_from_term_node(const xl::node::NodeIdentIFace* term_node)
{
    assert(term_node);

    auto string_term =
            dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(term_node);
    if(!string_term)
        return NULL;
    return string_term->value();
}

static const std::string* get_ident_string_ptr_from_term_node(const xl::node::NodeIdentIFace* term_node)
{
    assert(term_node);

    auto ident_term =
            dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::IDENT>*>(term_node);
    if(!ident_term)
        return NULL;
    return ident_term->value();
}

static std::string get_string_value_from_term_node(const xl::node::NodeIdentIFace* term_node)
{
    assert(term_node);

    switch(term_node->type())
    {
        case xl::node::NodeIdentIFace::CHAR:
            {
                auto char_term =
                        dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::CHAR>*>(term_node);
                std::stringstream ss;
                ss << '\'' << xl::escape(char_term->value()) << '\'';
                return ss.str();
            }
        case xl::node::NodeIdentIFace::STRING:
            {
                std::string* string_ptr = get_string_ptr_from_term_node(term_node);
                if(!string_ptr)
                    return "";
                return *string_ptr;
            }
        case xl::node::NodeIdentIFace::IDENT:
            {
                const std::string* ident_string_ptr = get_ident_string_ptr_from_term_node(term_node);
                if(!ident_string_ptr)
                    return "";
                return *ident_string_ptr;
            }
        default:
            return "";
    }
}

// kleene_node --> alt_node (up) --> action_node
static std::string* get_action_string_ptr_from_kleene_node(
        const xl::node::NodeIdentIFace* kleene_node)
{
    assert(kleene_node);

    // XML:
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

    const xl::node::NodeIdentIFace* alt_node = get_ancestor_node(ID_RULE_ALT, kleene_node);
    if(!alt_node)
        return NULL;
    const xl::node::NodeIdentIFace* action_node = get_right_child(alt_node);
    if(!action_node)
        return NULL;
    const xl::node::NodeIdentIFace* action_string_node = get_child(action_node);
    if(!action_string_node)
        return NULL;
    return get_string_ptr_from_term_node(action_string_node);
}

static std::string get_rule_name_from_rule_node(const xl::node::NodeIdentIFace* rule_node)
{
    assert(rule_node);

    const xl::node::NodeIdentIFace* lhs_node = get_left_child(rule_node);
    if(!lhs_node)
        return "";
    const std::string* ident_string_ptr = get_ident_string_ptr_from_term_node(lhs_node);
    if(!ident_string_ptr)
        return "";
    return *ident_string_ptr;
}

static xl::node::NodeIdentIFace* make_stem_rule(
        std::string name,
        const xl::node::NodeIdentIFace* rule_node,
        char                            kleene_op,
        const xl::node::NodeIdentIFace* outermost_paren_node,
        xl::TreeContext*                tc)
{
    assert(rule_node);
    assert(outermost_paren_node);
    assert(tc);

    // STRING:
    //program:
    //      (
    //            statement ',' { /* AAA */ ... }
    //      )* statement        { /* BBB */ ... }
    //    ;
    //
    // XML:
    //<symbol type="rule">
    //    <term type="ident" value=program/>
    //    <symbol type="alts">
    //        <symbol type="alt">
    //            <symbol type="terms">
    //                <symbol type="*">     // <-- kleene_node
    //                    <symbol type="("> // <-- paren_node
    //                        <symbol type="alts">
    //                            <symbol type="alt">
    //                                <symbol type="terms">
    //                                    <term type="ident" value=statement/>
    //                                    <term type="char" value=','/>
    //                                </symbol>
    //                                <symbol type="action_block">
    //                                    <term type="string" value=" /* AAA */ ... "/>
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

    xl::node::NodeIdentIFace* rule_node_clone = rule_node->clone(tc);
    const xl::node::NodeIdentIFace* find_node =
            (kleene_op == '(') ? outermost_paren_node : outermost_paren_node->parent();
    xl::node::NodeIdentIFace* find_node_clone =
            find_node_recursive(rule_node_clone, find_node);
    if(kleene_op != '(')
    {
        std::string* action_string_ptr = get_action_string_ptr_from_kleene_node(find_node_clone);
        if(action_string_ptr)
        {
            int position = 1;
            xl::node::NodeIdentIFace* outer_parent_node = find_node->parent();
            if(outer_parent_node)
            {
                xl::node::SymbolNodeIFace* outer_parent_symbol =
                        dynamic_cast<xl::node::SymbolNodeIFace*>(outer_parent_node);
                if(outer_parent_symbol)
                {
                    for(size_t i = 0; i<outer_parent_symbol->size(); i++)
                    {
                        if((*outer_parent_symbol)[i] == find_node)
                        {
                            position = i+1;
                            break;
                        }
                    }
                }
            }
            if(kleene_op == '?')
            {
                std::stringstream ss;
                ss << "if($" << position << ") ";
                action_string_ptr->append(ss.str());
            }
            action_string_ptr->append(gen_delete_rule_rvalue_term(position));
        }
    }
    xl::node::NodeIdentIFace* replacement_node =
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name));
    replace_node(find_node_clone, replacement_node);
    return rule_node_clone;
}

static xl::node::NodeIdentIFace* make_recursive_rule_plus(
        std::string      name1,
        std::string      name2,
        xl::TreeContext* tc)
{
    assert(tc);

    // STRING:
    //plus_0:
    //      plus_1        { /* AAA */ ... }
    //    | plus_0 plus_1 { /* BBB */ ... }
    //    ;
    //
    // XML:
    //<symbol type="rule">
    //    <term type="ident" value=plus_0/>
    //    <symbol type="alts">
    //        <symbol type="alt">
    //            <symbol type="terms">
    //                <term type="ident" value=plus_1/>
    //            </symbol>
    //            <symbol type="action_block">
    //                <term type="string" value=" /* AAA */ ... "/>
    //            </symbol>
    //        </symbol>
    //        <symbol type="alt">
    //            <symbol type="terms">
    //                <term type="ident" value=plus_0/>
    //                <term type="ident" value=plus_1/>
    //            </symbol>
    //            <symbol type="action_block">
    //                <term type="string" value=" /* BBB */ ... "/>
    //            </symbol>
    //        </symbol>
    //    </symbol>
    //</symbol>

    return MAKE_SYMBOL(tc, ID_RULE, 2,
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name1)),
            MAKE_SYMBOL(tc, ID_RULE_ALTS, 2,
                    MAKE_SYMBOL(tc, ID_RULE_ALT, 2,
                            MAKE_SYMBOL(tc, ID_RULE_TERMS, 1,
                                    MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name2))
                                    ),
                            MAKE_SYMBOL(tc, ID_RULE_ACTION_BLOCK, 1,
                                    MAKE_TERM(ID_STRING,
                                            tc->alloc_string(" $$ = new " + gen_type(name1) +
                                                    "; $$->push_back(*$1); delete $1; ")
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
}

static xl::node::NodeIdentIFace* make_recursive_rule_star(
        std::string      name1,
        std::string      name2,
        xl::TreeContext* tc)
{
    assert(tc);

    // STRING:
    //program_0:
    //      /* empty */         { /* AAA */ ... }
    //    | program_0 program_1 { /* BBB */ ... }
    //    ;
    //
    // XML:
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

    return MAKE_SYMBOL(tc, ID_RULE, 2,
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name1)),
            MAKE_SYMBOL(tc, ID_RULE_ALTS, 2,
                    MAKE_SYMBOL(tc, ID_RULE_ALT, 1,
                            MAKE_SYMBOL(tc, ID_RULE_ACTION_BLOCK, 1,
                                    MAKE_TERM(ID_STRING,
                                            tc->alloc_string(" $$ = new " + gen_type(name1) + "; ")
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
}

static xl::node::NodeIdentIFace* make_recursive_rule_optional(
        std::string      name1,
        std::string      name2,
        xl::TreeContext* tc)
{
    assert(tc);

    // STRING:
    //statement_0:
    //      /* empty */ { /* AAA */ $$ = NULL; }
    //    | statement_1 { /* BBB */ $$ = $1; }
    //    ;
    //
    // XML:
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

    return MAKE_SYMBOL(tc, ID_RULE, 2,
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
}

static xl::node::NodeIdentIFace* make_term_rule(
        std::string                     name,
        const xl::node::NodeIdentIFace* alts_node,
        xl::TreeContext*                tc)
{
    assert(alts_node);
    assert(tc);

    return MAKE_SYMBOL(tc, ID_RULE, 2,
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name)),
            alts_node->clone(tc));
}

// node to be appended to back of union_block_node
static xl::node::NodeIdentIFace* make_union_member_node(
        std::string      _type,
        std::string      _typename,
        xl::TreeContext* tc)
{
    assert(tc);

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

    return MAKE_SYMBOL(tc, ID_UNION_MEMBER, 1,
            MAKE_SYMBOL(tc, ID_UNION_TERMS, 2,
                    MAKE_SYMBOL(tc, ID_UNION_TERM, 1,
                            MAKE_TERM(ID_STRING, tc->alloc_string(_type + "*"))
                            ),
                    MAKE_SYMBOL(tc, ID_UNION_TERM, 1,
                            MAKE_TERM(ID_STRING, tc->alloc_string(_typename))
                            )
                    )
            );
}

// node to be appended to back of definitions block
static xl::node::NodeIdentIFace* make_def_brace_node(
        std::string               _type_name,
        std::vector<std::string>* token_vec,
        xl::TreeContext*          tc)
{
    assert(token_vec);
    assert(tc);

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
    for(auto p = token_vec->begin(); p != token_vec->end(); p++)
    {
        exploded_tokens.append(*p);
        if((p+1) != token_vec->end())
            exploded_tokens.append(", ");
    }
    return MAKE_SYMBOL(tc, ID_DEF_BRACE, 3,
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string("type")),
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(_type_name)),
            MAKE_SYMBOL(tc, ID_DEF_SYMBOLS, 1,
                    MAKE_SYMBOL(tc, ID_DEF_SYMBOL, 1,
                            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(exploded_tokens))
                            )
                    )
            );
}

static void add_union_member(
        TreeChanges*                    tree_changes,
        std::string                     rule_name,
        const xl::node::NodeIdentIFace* union_block_node,
        xl::TreeContext*                tc)
{
    assert(tree_changes);
    assert(union_block_node);
    assert(tc);

    const xl::node::NodeIdentIFace* union_members_node = get_child(union_block_node);
    if(!union_members_node)
        return;
    auto union_members_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(union_members_node);
    if(!union_members_symbol)
        return;
    xl::node::NodeIdentIFace* union_member_node =
            make_union_member_node(gen_type(rule_name), gen_typename(rule_name), tc);
    if(!union_member_node)
        return;
    if(!union_members_symbol->find(union_member_node))
        tree_changes->add_change(
                TreeChange::NODE_APPENDS_TO_BACK,
                union_members_symbol,
                union_member_node);
}

static void add_def_brace(
        TreeChanges*                    tree_changes,
        std::string                     name1,
        const xl::node::NodeIdentIFace* definitions_node,
        xl::TreeContext*                tc)
{
    assert(tree_changes);
    assert(definitions_node);
    assert(tc);

    auto definitions_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(definitions_node);
    if(!definitions_symbol)
        return;
    std::vector<std::string> token_vec;
    token_vec.push_back(name1);
    xl::node::NodeIdentIFace* def_brace_node =
            make_def_brace_node(gen_typename(name1), &token_vec, tc);
    if(!definitions_symbol->find(def_brace_node))
        tree_changes->add_change(
                TreeChange::NODE_APPENDS_TO_BACK,
                definitions_symbol,
                def_brace_node);
}

static void add_term_rule(
        TreeChanges*     tree_changes,
        std::string      name2,
        KleeneContext*   kleene_context,
        EBNFContext*     ebnf_context,
        xl::TreeContext* tc)
{
    assert(tree_changes);
    assert(kleene_context);
    assert(kleene_context->innermost_paren_node);
    assert(kleene_context->rule_node);
    assert(kleene_context->rule_def_symbol_node);
    assert(ebnf_context);
    assert(ebnf_context->union_block_node);
    assert(ebnf_context->definitions_node);
    assert(tc);

    const xl::node::NodeIdentIFace* alts_node = get_child(kleene_context->innermost_paren_node);
    if(!alts_node)
        return;
    xl::node::NodeIdentIFace* term_rule = make_term_rule(name2, alts_node, tc);
    if(!term_rule)
        return;
#ifdef DEBUG_EBNF
    std::cout << ">>> (term_rule)" << std::endl;
    EBNFPrinter v(tc); v.dispatch_visit(term_rule); std::cout << std::endl;
    std::cout << "<<< (term_rule)" << std::endl;
#endif
    tree_changes->add_change(
            TreeChange::NODE_INSERTIONS_AFTER,
            kleene_context->rule_node,
            term_rule);
    add_union_member(
            tree_changes,
            name2,
            ebnf_context->union_block_node,
            tc);
    add_def_brace(
            tree_changes,
            name2,
            ebnf_context->definitions_node,
            tc);
}

static void add_recursive_rule(
        TreeChanges*     tree_changes,
        KleeneContext*   kleene_context,
        EBNFContext*     ebnf_context,
        xl::TreeContext* tc)
{
    assert(tree_changes);
    assert(kleene_context);
    assert(kleene_context->rule_node);
    assert(ebnf_context);
    assert(ebnf_context->union_block_node);
    assert(ebnf_context->definitions_node);
    assert(tc);

    xl::node::NodeIdentIFace* recursive_rule = NULL;
    switch(kleene_context->kleene_op)
    {
        case '+':
            recursive_rule = make_recursive_rule_plus(kleene_context->name1, kleene_context->name2, tc);
            break;
        case '*':
            recursive_rule = make_recursive_rule_star(kleene_context->name1, kleene_context->name2, tc);
            break;
        case '?':
            recursive_rule = make_recursive_rule_optional(kleene_context->name1, kleene_context->name2, tc);
            break;
    }
    if(!recursive_rule)
        return;
#ifdef DEBUG_EBNF
    std::cout << ">>> (recursive_rule)" << std::endl;
    EBNFPrinter v(tc); v.dispatch_visit(recursive_rule); std::cout << std::endl;
    std::cout << "<<< (recursive_rule)" << std::endl;
#endif
    tree_changes->add_change(
            TreeChange::NODE_INSERTIONS_AFTER,
            kleene_context->rule_node,
            recursive_rule);
    add_union_member(
            tree_changes,
            kleene_context->name1,
            ebnf_context->union_block_node,
            tc);
    add_def_brace(
            tree_changes,
            kleene_context->name1,
            ebnf_context->definitions_node,
            tc);
}

static void add_stem_rule(
        TreeChanges*     tree_changes,
        KleeneContext*   kleene_context,
        xl::TreeContext* tc)
{
    assert(tree_changes);
    assert(kleene_context);
    assert(kleene_context->outermost_paren_node);
    assert(kleene_context->rule_node);
    assert(tc);

    xl::node::NodeIdentIFace* stem_rule =
            make_stem_rule(
                    kleene_context->name1,
                    kleene_context->rule_node,
                    kleene_context->kleene_op,
                    kleene_context->outermost_paren_node,
                    tc);
    if(!stem_rule)
        return;
#ifdef DEBUG_EBNF
    std::cout << ">>> (stem_rule)" << std::endl;
    EBNFPrinter v(tc); v.dispatch_visit(stem_rule); std::cout << std::endl;
    std::cout << "<<< (stem_rule)" << std::endl;
#endif
    tree_changes->add_change(
            TreeChange::NODE_REPLACEMENTS,
            kleene_context->rule_node,
            stem_rule);
}

static xl::node::NodeIdentIFace* find_unique_child_by_sym_id(
        const xl::node::SymbolNodeIFace* symbol,
        uint32_t                         sym_id)
{
    assert(symbol);

    xl::node::NodeIdentIFace* unique_child_node = NULL;
    for(size_t i = 0; i<symbol->size(); i++)
    {
        xl::node::NodeIdentIFace* child_node = (*symbol)[i];
        if(child_node && child_node->sym_id() == sym_id)
        {
            if(unique_child_node)
                return NULL;
            unique_child_node = child_node;
        }
    }
    return unique_child_node;
}

static const xl::node::NodeIdentIFace* enter_cyclic_sequence(
        const xl::node::NodeIdentIFace* _node,
        bool                            cyclic,
        ...)
{
    assert(_node);

    std::vector<uint32_t> cyclic_sequence;
    uint32_t sym_id = 0;
    va_list ap;
    va_start(ap, cyclic);
    do
    {
        sym_id = va_arg(ap, uint32_t);
        cyclic_sequence.push_back(sym_id);
    } while(sym_id);
    va_end(ap);
    if(cyclic_sequence.empty())
        return _node;
    const xl::node::NodeIdentIFace* next_node = _node;
    if(next_node->sym_id() == cyclic_sequence[0])
    {
        bool done = false;
        do
        {
            const xl::node::NodeIdentIFace* next_node_in_sequence = next_node;
            for(size_t i = 1; i<cyclic_sequence.size(); i++)
            {
                auto next_symbol =
                        dynamic_cast<const xl::node::SymbolNodeIFace*>(next_node_in_sequence);
                if(!next_symbol)
                {
                    done = true;
                    break;
                }
                size_t mapped_index = i%(cyclic_sequence.size()-1);
                const xl::node::NodeIdentIFace* unique_child_node =
                        find_unique_child_by_sym_id(next_symbol, cyclic_sequence[mapped_index]);
                if(!unique_child_node || unique_child_node->sym_id() != cyclic_sequence[mapped_index])
                {
                    done = true;
                    break;
                }
                next_node_in_sequence = unique_child_node;
                if(mapped_index == 0)
                    next_node = next_node_in_sequence;
            }
        } while(cyclic && next_node->sym_id() == cyclic_sequence[0] && !done);
    }
    return next_node;
}

static const xl::node::NodeIdentIFace* get_innermost_paren_node(const xl::node::NodeIdentIFace* paren_node)
{
    assert(paren_node);

    return enter_cyclic_sequence(paren_node, true, '(', ID_RULE_ALTS, ID_RULE_ALT, ID_RULE_TERMS, 0);
}

static void add_shared_typedefs_and_headers(
        TreeChanges*                    tree_changes,
        std::string                     name1,
        std::string                     name2,
        const xl::node::NodeIdentIFace* innermost_paren_node,
        KleeneContext*                  kleene_context,
        EBNFContext*                    ebnf_context)
{
    assert(tree_changes);
    assert(innermost_paren_node);
    assert(kleene_context);
    assert(ebnf_context);
    assert(ebnf_context->def_symbol_name_to_union_typename.size());
    assert(ebnf_context->union_typename_to_type.size());
    assert(ebnf_context->proto_block_node);

    const xl::node::NodeIdentIFace* alts_node = get_child(innermost_paren_node);
    if(!alts_node)
        return;
    auto alts_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(alts_node);
    if(!alts_symbol)
        return;
    typedef std::vector<std::pair<std::string, const xl::node::NodeIdentIFace*>> delayed_recursion_args_t;
    delayed_recursion_args_t delayed_recursion_args;
    std::vector<std::string> variant_type_vec;
    for(size_t j = 0; j<alts_symbol->size(); j++)
    {
        const xl::node::NodeIdentIFace* alt_node = (*alts_symbol)[j];
        if(!alt_node)
            return;
        const xl::node::NodeIdentIFace* term_node = get_left_child(alt_node);
        if(!term_node)
            return;
        auto term_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(term_node);
        if(!term_symbol)
            return;
        std::vector<std::string> tuple_type_vec;
        for(size_t i = 0; i<term_symbol->size(); i++)
        {
            xl::node::NodeIdentIFace* child_node = (*term_symbol)[i];
            switch(child_node->type())
            {
                case xl::node::NodeIdentIFace::INT:    tuple_type_vec.push_back("int"); break;
                case xl::node::NodeIdentIFace::FLOAT:  tuple_type_vec.push_back("float"); break;
                case xl::node::NodeIdentIFace::STRING: tuple_type_vec.push_back("std::string"); break;
                case xl::node::NodeIdentIFace::CHAR:   tuple_type_vec.push_back("char"); break;
                case xl::node::NodeIdentIFace::IDENT:
                    {
                        std::string def_symbol_name =
                                *dynamic_cast<
                                        xl::node::TermNode<xl::node::NodeIdentIFace::IDENT>*
                                        >(child_node)->value();
                        std::string union_typename =
                                ebnf_context->def_symbol_name_to_union_typename[def_symbol_name];
                        std::string union_type = ebnf_context->union_typename_to_type[union_typename];
                        tuple_type_vec.push_back(union_type);
                    }
                    break;
                case xl::node::NodeIdentIFace::SYMBOL:
                    {
                        const xl::node::NodeIdentIFace* kleene_node = child_node;
                        uint32_t kleene_op = kleene_node->sym_id();
                        switch(kleene_op)
                        {
                            case '+':
                            case '*':
                            case '?':
                            case '(':
                                {
                                    std::string next_name1      = gen_name(name1 + INTERNAL_NAME_SUFFIX);
                                    std::string next_name1_type = gen_type(next_name1);
                                    tuple_type_vec.push_back(next_name1_type);
                                    const xl::node::NodeIdentIFace* next_outermost_paren_node =
                                            (kleene_op == '(') ? kleene_node : get_child(kleene_node);
                                    const xl::node::NodeIdentIFace* next_innermost_paren_node =
                                            get_innermost_paren_node(next_outermost_paren_node);
                                    delayed_recursion_args.push_back(delayed_recursion_args_t::value_type(
                                            next_name1,
                                            next_innermost_paren_node));
                                }
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        variant_type_vec.push_back(gen_tuple_type(&tuple_type_vec));
    }
    std::string include_headers = gen_tuple_include_headers();
    std::string kleene_type;
    if(alts_symbol->size() == 1)
        kleene_type = variant_type_vec[0];
    else
    {
        kleene_type = gen_variant_type(&variant_type_vec);
        include_headers.insert(0, gen_variant_include_headers() + "\n");
    }
    std::string kleene_typedef;
    std::string kleene_depends_typedef;
    if(kleene_context->kleene_op == '(')
        kleene_typedef = gen_typedef(kleene_type, gen_type(name1));
    else
    {
        switch(kleene_context->kleene_op)
        {
            case '?':
                kleene_typedef = gen_typedef(gen_type(name2), gen_type(name1));
                break;
            case '*':
            case '+':
                kleene_typedef = gen_vector_typedef(gen_type(name2), gen_type(name1));
                include_headers.insert(0, gen_vector_include_headers() + "\n");
                break;
        }
        kleene_depends_typedef = gen_typedef(kleene_type, gen_type(name2));
    }
    std::string shared_typedefs_and_headers =
            std::string("\n") + include_headers + "\n" +
            (kleene_depends_typedef.empty() ? "" : kleene_depends_typedef + "\n") +
            kleene_typedef;
    const xl::node::NodeIdentIFace* proto_block_term_node = get_child(ebnf_context->proto_block_node);
    if(!proto_block_term_node)
        return;
    std::string proto_block_string = get_string_value_from_term_node(proto_block_term_node);
    if(proto_block_string.find(shared_typedefs_and_headers) == std::string::npos)
        tree_changes->add_change(
                TreeChange::STRING_INSERTIONS_TO_FRONT,
                proto_block_term_node,
                shared_typedefs_and_headers);
    for(auto p = delayed_recursion_args.begin(); p != delayed_recursion_args.end(); p++)
    {
        add_shared_typedefs_and_headers(
                tree_changes,
                (*p).first,
                name2,
                (*p).second,
                kleene_context,
                ebnf_context);
    }
}

KleeneContext::KleeneContext(
        const xl::node::NodeIdentIFace* kleene_node,
        EBNFContext*                    ebnf_context)
{
    kleene_op            = kleene_node->sym_id();
    outermost_paren_node = (kleene_node->sym_id() == '(') ? kleene_node : get_child(kleene_node);
    innermost_paren_node = get_innermost_paren_node(outermost_paren_node);
    rule_node            = get_ancestor_node(ID_RULE, outermost_paren_node);
    rule_name            = get_rule_name_from_rule_node(rule_node);
    rule_def_symbol_node = ebnf_context->def_symbol_name_to_node[rule_name];
    switch(kleene_op)
    {
        case '+':
        case '*':
            name1 = gen_name(rule_name + RECURSIVE_NAME_SUFFIX);
            break;
        case '?':
            name1 = gen_name(rule_name + OPTIONAL_NAME_SUFFIX);
            break;
        case '(':
            name1 = gen_name(rule_name + PAREN_NAME_SUFFIX);
            break;
    }
    name2 = gen_name(rule_name + TERM_NAME_SUFFIX);
}

static void add_changes_for_kleene_closure(
        TreeChanges*                    tree_changes,
        const xl::node::NodeIdentIFace* kleene_node,
        EBNFContext*                    ebnf_context,
        xl::TreeContext*                tc)
{
    assert(tree_changes);
    assert(kleene_node);
    assert(ebnf_context);
    assert(ebnf_context->def_symbol_name_to_node.size());
    assert(tc);

    KleeneContext kleene_context(kleene_node, ebnf_context);
    add_term_rule(
            tree_changes,
            (kleene_context.kleene_op == '(') ? kleene_context.name1 : kleene_context.name2,
            &kleene_context,
            ebnf_context,
            tc);
    if(kleene_context.kleene_op != '(')
        add_recursive_rule(
                tree_changes,
                &kleene_context,
                ebnf_context,
                tc);
    add_stem_rule(
            tree_changes,
            &kleene_context,
            tc);
    add_shared_typedefs_and_headers(
            tree_changes,
            kleene_context.name1,
            kleene_context.name2,
            kleene_context.innermost_paren_node,
            &kleene_context,
            ebnf_context);
}

void EBNFContext::reset()
{
    definitions_node = NULL;
    proto_block_node = NULL;
    union_block_node = NULL;
    def_symbol_name_to_node.clear();
    union_typename_to_type.clear();
    def_symbol_name_to_union_typename.clear();
}

void EBNFPrinter::visit(const xl::node::SymbolNodeIFace* _node)
{
    assert(_node);

#ifdef DEBUG_EBNF
    if(_node->sym_id() == ID_RULE || is_kleene_node(_node))
        std::cout << '[';
#endif
    static bool entered_kleene_closure = false;
    static EBNFContext ebnf_context;
    static std::vector<std::string> union_term_names, def_symbol_names;
    bool more = false;
    uint32_t kleene_op = _node->sym_id();
    switch(kleene_op)
    {
        case ID_GRAMMAR:
            {
                entered_kleene_closure = false;
                ebnf_context.reset();
            }
            visit_next_child(_node);
            std::cout << std::endl << std::endl << "%%" << std::endl << std::endl;
            visit_next_child(_node);
            std::cout << std::endl << std::endl << "%%";
            visit_next_child(_node);
            std::cout << std::endl;
            break;
        case ID_DEFINITIONS:
            ebnf_context.definitions_node = _node;
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
                    xl::node::NodeIdentIFace* child_node = NULL;
                    visit_next_child(_node, &child_node);
                    if(child_node)
                        union_typename = get_string_value_from_term_node(child_node);
                    std::cout << "> ";
                }
                visit_next_child(_node);
                for(auto p = def_symbol_names.begin(); p != def_symbol_names.end(); p++)
                    ebnf_context.def_symbol_name_to_union_typename[*p] = union_typename;
            }
            break;
        case ID_DEF_PROTO_BLOCK:
            ebnf_context.proto_block_node = _node;
            std::cout << "%{";
            std::cout << get_string_value_from_term_node(get_child(_node));
            std::cout << "%}";
            break;
        case ID_UNION_BLOCK:
            ebnf_context.union_block_node = _node;
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
                for(size_t i = 0; i<union_term_names.size()-1; i++) // exclude last
                    union_type.append(union_term_names[i]);
                std::string union_typename =
                        union_term_names[union_term_names.size()-1]; // last term is typename
                ebnf_context.union_typename_to_type[union_typename] = union_type;
            }
            break;
        case ID_UNION_TERM:
            {
                std::string name = get_string_value_from_term_node(get_child(_node));
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
                std::string symbol_name = get_string_value_from_term_node(get_child(_node));
                std::cout << symbol_name;
                if(!symbol_name.empty())
                {
                    ebnf_context.def_symbol_name_to_node[symbol_name] = _node;
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
            {
                set_allow_visit_null(false);
                visit_next_child(_node);
                set_allow_visit_null(true);
            }
            break;
        case ID_RULE_ACTION_BLOCK:
            std::cout << " {";
            std::cout << get_string_value_from_term_node(get_child(_node));
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
        case '(':
            {
                if(!entered_kleene_closure)
                {
                    if(m_tree_changes)
                        add_changes_for_kleene_closure(
                                m_tree_changes,
                                _node,
                                &ebnf_context,
                                m_tc);
                    entered_kleene_closure = true; // only enter once
                }
                if(kleene_op == '(')
                    std::cout << '(';
                xl::visitor::VisitorDFS::visit(_node); // continue visitation/pretty-print
                if(kleene_op == '(')
                    std::cout << ')';
                else
                    std::cout << static_cast<char>(kleene_op);
            }
            break;
        case ID_CODE:
            std::cout << get_string_value_from_term_node(get_child(_node));
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
