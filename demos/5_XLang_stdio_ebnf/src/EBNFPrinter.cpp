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

// string to be inserted to front of proto_block_node's string value
static std::string gen_shared_include_headers()
{
    return "#include <vector>\n#include <tuple>";
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_typedef(std::string _type, std::string _typename)
{
    return std::string("typedef ") + _type + " " + _typename + ";";
}

// string to be inserted to front of proto_block_node's string value
static std::string gen_tuple_typedef(std::vector<std::string> &type_vec, std::string _typename)
{
    std::string exploded_types;
    for(auto p = type_vec.begin(); p != type_vec.end(); p++)
    {
        exploded_types.append(*p);
        if((p+1) != type_vec.end())
            exploded_types.append(", ");
    }
    return gen_typedef("std::tuple<" + exploded_types + ">", _typename);
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
    return stem + "_type";
}

static std::string gen_type(std::string stem)
{
    return stem + "_type_t";
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
static const xl::node::NodeIdentIFace* get_alts_node_from_paren_node(
        const xl::node::NodeIdentIFace* paren_node)
{
    // XML:
    //<symbol type="(">        // <-- paren_node
    //    <symbol type="alts"> // <-- alts_node
    //        <!-- ... -->
    //    </symbol>
    //</symbol>

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

static std::string gen_delete_rule_rvalue_term(int position);
static xl::node::NodeIdentIFace* make_stem_rule(
        std::string name,
        const xl::node::NodeIdentIFace* rule_node,
        char                            kleene_op,
        const xl::node::NodeIdentIFace* outermost_paren_node,
        xl::TreeContext* tc)
{
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

    if(!rule_node || !outermost_paren_node)
        return NULL;
    xl::node::NodeIdentIFace* rule_node_copy = rule_node->clone(tc);
    const xl::node::NodeIdentIFace* outer_node =
            (kleene_op == '(') ? outermost_paren_node : outermost_paren_node->parent();
    xl::node::NodeIdentIFace* outer_node_copy =
            find_clone_of_original_recursive(rule_node_copy, outer_node);
    if(kleene_op != '(')
    {
        std::string* action_string_ptr = get_action_string_from_kleene_node(outer_node_copy);
        if(action_string_ptr)
        {
            int position = 1;
            xl::node::NodeIdentIFace* outer_parent_node = outer_node->parent();
            if(outer_parent_node)
            {
                xl::node::SymbolNodeIFace* outer_parent_symbol =
                        dynamic_cast<xl::node::SymbolNodeIFace*>(outer_parent_node);
                if(outer_parent_symbol)
                {
                    for(size_t i = 0; i<outer_parent_symbol->size(); i++)
                    {
                        if((*outer_parent_symbol)[i] == outer_node)
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
    replace_node(outer_node_copy, replacement_node);
    return rule_node_copy;
}

static xl::node::NodeIdentIFace* make_recursive_rule_plus(std::string name1, std::string name2,
        xl::TreeContext* tc)
{
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

    xl::node::NodeIdentIFace* node =
            MAKE_SYMBOL(tc, ID_RULE, 2,
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
    return node;
}

static xl::node::NodeIdentIFace* make_recursive_rule_star(std::string name1, std::string name2,
        xl::TreeContext* tc)
{
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

    xl::node::NodeIdentIFace* node =
            MAKE_SYMBOL(tc, ID_RULE, 2,
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
    return node;
}

static xl::node::NodeIdentIFace* make_recursive_rule_optional(std::string name1, std::string name2,
        xl::TreeContext* tc)
{
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
        const xl::node::NodeIdentIFace* alts_node,
        xl::TreeContext* tc)
{
    if(!alts_node)
        return NULL;
    return MAKE_SYMBOL(tc, ID_RULE, 2,
            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name)),
            alts_node->clone(tc));
}

// node to be appended to back of union_block_node
static xl::node::NodeIdentIFace* make_union_member_node(
        std::string _type, std::string _typename, xl::TreeContext* tc)
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
                                    MAKE_TERM(ID_STRING, tc->alloc_string(_type + "*"))
                                    ),
                            MAKE_SYMBOL(tc, ID_UNION_TERM, 1,
                                    MAKE_TERM(ID_STRING, tc->alloc_string(_typename))
                                    )
                            )
                    );
    return node;
}

// node to be appended to back of definitions block
static xl::node::NodeIdentIFace* make_def_brace_node(
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
        exploded_tokens.append(*p);
        if((p+1) != token_vec.end())
            exploded_tokens.append(", ");
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

static void add_shared_typedefs_and_headers(
        std::map<const xl::node::NodeIdentIFace*, std::list<std::string>>* string_insertions_to_front,
        std::string                                                        name1,
        std::string                                                        name2,
        char                                                               kleene_op,
        const xl::node::NodeIdentIFace*                                    paren_node,
        const xl::node::NodeIdentIFace*                                    proto_block_node,
        std::map<std::string, std::string>*                                union_typename_to_type,
        std::map<std::string, std::string>*                                def_symbol_name_to_union_typename)
{
    if(!string_insertions_to_front || !paren_node)
        return;
    const xl::node::NodeIdentIFace* alts_node = get_alts_node_from_paren_node(paren_node);
    if(!alts_node)
        return;
    auto alts_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(alts_node);
    if(!alts_symbol)
        return;
    xl::node::NodeIdentIFace* alt_node = CHILD_OF(alts_symbol); // only consider first alt_node
    auto alt_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(alt_node);
    if(!alt_symbol)
        return;
    xl::node::NodeIdentIFace* term_node = (*alt_symbol)[0]; // left child is symbol, right child is action
    auto term_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(term_node);
    if(!term_symbol)
        return;
    std::vector<std::string> type_vec;
    for(size_t i = 0; i < term_symbol->size(); i++)
    {
        xl::node::NodeIdentIFace* child = (*term_symbol)[i];
        switch(child->type())
        {
            case xl::node::NodeIdentIFace::INT:    type_vec.push_back("int"); break;
            case xl::node::NodeIdentIFace::FLOAT:  type_vec.push_back("float"); break;
            case xl::node::NodeIdentIFace::STRING: type_vec.push_back("std::string"); break;
            case xl::node::NodeIdentIFace::CHAR:   type_vec.push_back("char"); break;
            case xl::node::NodeIdentIFace::IDENT:
                {
                    std::string def_symbol_name =
                            *dynamic_cast<
                                    xl::node::TermNode<xl::node::NodeIdentIFace::IDENT>*
                                    >(child)->value();
                    std::string union_typename = (*def_symbol_name_to_union_typename)[def_symbol_name];
                    std::string union_type = (*union_typename_to_type)[union_typename];
                    type_vec.push_back(union_type);
                }
                break;
            default:
                break;
        }
    }
    auto proto_block_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(proto_block_node);
    if(!proto_block_symbol)
        return;
    xl::node::NodeIdentIFace* proto_block_term_node = CHILD_OF(proto_block_symbol);
    if(!proto_block_term_node)
        return;
    std::string proto_block_string = get_string_from_term_node(proto_block_term_node);
    std::string shared_typedefs_and_headers;
    std::string shared_include_headers = gen_shared_include_headers();
    if(kleene_op == '(')
    {
        shared_typedefs_and_headers =
                std::string("\n") + shared_include_headers + "\n" +
                gen_tuple_typedef(type_vec, gen_type(name1));
    }
    else
    {
        std::string vector_typedef;
        if(kleene_op == '?')
            vector_typedef = gen_typedef(gen_type(name2), gen_type(name1));
        else
            vector_typedef = gen_vector_typedef(gen_type(name2), gen_type(name1));
        shared_typedefs_and_headers =
                std::string("\n") + shared_include_headers + "\n" +
                gen_tuple_typedef(type_vec, gen_type(name2)) + "\n" +
                vector_typedef;
    }
    if(proto_block_string.find(shared_typedefs_and_headers) == std::string::npos)
        (*string_insertions_to_front)[proto_block_term_node].push_back(shared_typedefs_and_headers);
}

static void add_union_member(
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_appends_to_back,
        std::string                                                                      rule_name,
        const xl::node::NodeIdentIFace*                                                  union_block_node,
        xl::TreeContext*                                                                 tc)
{
    if(!node_appends_to_back)
        return;
    auto union_block_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(union_block_node);
    if(!union_block_symbol)
        return;
    xl::node::NodeIdentIFace* union_members_node = CHILD_OF(union_block_symbol);
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
        (*node_appends_to_back)[union_members_symbol].push_back(union_member_node);
}

static void add_def_brace(
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_appends_to_back,
        std::string                                                                      name1,
        const xl::node::NodeIdentIFace*                                                  definitions_node,
        xl::TreeContext*                                                                 tc)
{
    if(!node_appends_to_back)
        return;
    auto definitions_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(definitions_node);
    if(!definitions_symbol)
        return;
    std::vector<std::string> token_vec;
    token_vec.push_back(name1);
    xl::node::NodeIdentIFace* def_brace_node =
            make_def_brace_node(gen_typename(name1), token_vec, tc);
    if(!definitions_symbol->find(def_brace_node))
        (*node_appends_to_back)[definitions_symbol].push_back(def_brace_node);
}

static void add_term_rule(
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_insertions_after,
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_appends_to_back,
        std::string                                                                      name2,
        const xl::node::NodeIdentIFace*                                                  paren_node,
        const xl::node::NodeIdentIFace*                                                  rule_node,
        const xl::node::NodeIdentIFace*                                                  rule_def_symbol_node,
        const xl::node::NodeIdentIFace*                                                  definitions_node,
        const xl::node::NodeIdentIFace*                                                  union_block_node,
        xl::TreeContext*                                                                 tc)
{
    if(!node_insertions_after)
        return;
    const xl::node::NodeIdentIFace* alts_node = get_alts_node_from_paren_node(paren_node);
    if(!alts_node)
        return;
    xl::node::NodeIdentIFace* term_rule = make_term_rule(name2, alts_node, tc);
    if(!term_rule)
        return;
#ifdef DEBUG_EBNF
    std::cout << ">>> (term_rule)" << std::endl;
    EBNFPrinter v(tc); v.visit_any(term_rule); std::cout << std::endl;
    std::cout << "<<< (term_rule)" << std::endl;
#endif
    (*node_insertions_after)[rule_node].push_back(term_rule);
    add_union_member(
            node_appends_to_back,
            name2,
            union_block_node,
            tc);
    add_def_brace(
            node_appends_to_back,
            name2,
            definitions_node,
            tc);
}

static void add_recursive_rule(
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_insertions_after,
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_appends_to_back,
        std::string                                                                      name1,
        std::string                                                                      name2,
        char                                                                             kleene_op,
        const xl::node::NodeIdentIFace*                                                  rule_node,
        const xl::node::NodeIdentIFace*                                                  definitions_node,
        const xl::node::NodeIdentIFace*                                                  proto_block_node,
        const xl::node::NodeIdentIFace*                                                  union_block_node,
        xl::TreeContext*                                                                 tc)
{
    if(!node_insertions_after || !node_appends_to_back)
        return;
    xl::node::NodeIdentIFace* recursive_rule = NULL;
    switch(kleene_op)
    {
        case '+': recursive_rule = make_recursive_rule_plus(name1, name2, tc); break;
        case '*': recursive_rule = make_recursive_rule_star(name1, name2, tc); break;
        case '?': recursive_rule = make_recursive_rule_optional(name1, name2, tc); break;
    }
    if(!recursive_rule)
        return;
#ifdef DEBUG_EBNF
    std::cout << ">>> (recursive_rule)" << std::endl;
    EBNFPrinter v(tc); v.visit_any(recursive_rule); std::cout << std::endl;
    std::cout << "<<< (recursive_rule)" << std::endl;
#endif
    (*node_insertions_after)[rule_node].push_back(recursive_rule);
    add_union_member(
            node_appends_to_back,
            name1,
            union_block_node,
            tc);
    add_def_brace(
            node_appends_to_back,
            name1,
            definitions_node,
            tc);
}

static void add_stem_rule(
        std::map<const xl::node::NodeIdentIFace*, xl::node::NodeIdentIFace*>* node_replacements,
        std::string                                                           name1,
        char                                                                  kleene_op,
        const xl::node::NodeIdentIFace*                                       outermost_paren_node,
        const xl::node::NodeIdentIFace*                                       rule_node,
        const xl::node::NodeIdentIFace*                                       proto_block_node,
        xl::TreeContext*                                                      tc)
{
    if(!node_replacements)
        return;
    xl::node::NodeIdentIFace* stem_rule =
            make_stem_rule(name1, rule_node, kleene_op, outermost_paren_node, tc);
    if(!stem_rule)
        return;
#ifdef DEBUG_EBNF
    std::cout << ">>> (stem_rule)" << std::endl;
    EBNFPrinter v(tc); v.visit_any(stem_rule); std::cout << std::endl;
    std::cout << "<<< (stem_rule)" << std::endl;
#endif
    (*node_replacements)[rule_node] = stem_rule;
}

static void enqueue_changes_for_kleene_closure(
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_insertions_after,
        std::map<const xl::node::NodeIdentIFace*, std::list<std::string>>*               string_appends_to_back,
        std::map<const xl::node::NodeIdentIFace*, std::list<std::string>>*               string_insertions_to_front,
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* node_appends_to_back,
        std::map<const xl::node::NodeIdentIFace*, xl::node::NodeIdentIFace*>*            node_replacements,
        char                                                                             kleene_op,
        const xl::node::NodeIdentIFace*                                                  paren_node,
        const xl::node::NodeIdentIFace*                                                  definitions_node,
        const xl::node::NodeIdentIFace*                                                  proto_block_node,
        const xl::node::NodeIdentIFace*                                                  union_block_node,
        std::map<std::string, const xl::node::NodeIdentIFace*>*                          def_symbol_name_to_node,
        std::map<std::string, std::string>*                                              union_typename_to_type,
        std::map<std::string, std::string>*                                              def_symbol_name_to_union_typename,
        xl::TreeContext*                                                                 tc)
{
    if(!paren_node)
        return;
    const xl::node::NodeIdentIFace* rule_node            = get_ancestor_node(ID_RULE, paren_node);
    std::string                     rule_name            = get_rule_name_from_rule_node(rule_node);
    const xl::node::NodeIdentIFace* rule_def_symbol_node = (*def_symbol_name_to_node)[rule_name];
    std::string                     name1                = gen_name(rule_name);
    std::string                     name2                = gen_name(rule_name);
    const xl::node::NodeIdentIFace* outermost_paren_node = paren_node;
    const xl::node::NodeIdentIFace* innermost_paren_node = paren_node;
    while(innermost_paren_node->sym_id() == '(')
    {
        auto paren_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(innermost_paren_node);
        if(!paren_symbol || paren_symbol->size() > 1)
            break;
        const xl::node::NodeIdentIFace* alts_node = CHILD_OF(paren_symbol);
        if(alts_node->sym_id() != ID_RULE_ALTS)
            break;
        auto alts_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(alts_node);
        if(!alts_symbol || alts_symbol->size() > 1)
            break;
        const xl::node::NodeIdentIFace* alt_node = CHILD_OF(alts_symbol);
        if(alt_node->sym_id() != ID_RULE_ALT)
            break;
        auto alt_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(alt_node);
        if(!alt_symbol || alt_symbol->size() > 1)
            break;
        const xl::node::NodeIdentIFace* terms_node = CHILD_OF(alt_symbol);
        if(terms_node->sym_id() != ID_RULE_TERMS)
            break;
        auto terms_symbol = dynamic_cast<const xl::node::SymbolNodeIFace*>(terms_node);
        if(!terms_symbol || terms_symbol->size() > 1)
            break;
        const xl::node::NodeIdentIFace* paren_child_node = CHILD_OF(terms_symbol);
        if(paren_child_node->sym_id() != '(')
            break;
        innermost_paren_node = paren_child_node;
    }
    add_term_rule(
            node_insertions_after,
            node_appends_to_back,
            (kleene_op == '(') ? name1 : name2,
            innermost_paren_node,
            rule_node,
            rule_def_symbol_node,
            definitions_node,
            union_block_node,
            tc);
    if(kleene_op != '(')
        add_recursive_rule(
                node_insertions_after,
                node_appends_to_back,
                name1,
                name2,
                kleene_op,
                rule_node,
                definitions_node,
                proto_block_node,
                union_block_node,
                tc);
    add_stem_rule(
            node_replacements,
            name1,
            kleene_op,
            outermost_paren_node,
            rule_node,
            proto_block_node,
            tc);
    add_shared_typedefs_and_headers(
            string_insertions_to_front,
            name1,
            name2,
            kleene_op,
            innermost_paren_node,
            proto_block_node,
            union_typename_to_type,
            def_symbol_name_to_union_typename);
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
    static std::map<std::string, const xl::node::NodeIdentIFace*> def_symbol_name_to_node;
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
                def_symbol_name_to_node.clear();
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
                for(size_t i = 0; i<union_term_names.size()-1; i++) // exclude last
                    union_type.append(union_term_names[i]);
                std::string union_typename =
                        union_term_names[union_term_names.size()-1]; // last term is typename
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
                    def_symbol_name_to_node[symbol_name] = _node;
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
        case '(':
            {
                char kleene_op = _node->sym_id();
                if(!entered_kleene_closure)
                {
                    if(m_changes)
                        enqueue_changes_for_kleene_closure(
                                &m_changes->m_node_insertions_after,
                                &m_changes->m_string_appends_to_back,
                                &m_changes->m_string_insertions_to_front,
                                &m_changes->m_node_appends_to_back,
                                &m_changes->m_node_replacements,
                                kleene_op,
                                (kleene_op == '(') ? _node : CHILD_OF(_node),
                                definitions_node,
                                proto_block_node,
                                union_block_node,
                                &def_symbol_name_to_node,
                                &union_typename_to_type,
                                &def_symbol_name_to_union_typename,
                                m_tc);
                    entered_kleene_closure = true;
                }
                if(kleene_op == '(')
                    std::cout << '(';
                xl::visitor::TraverseDFS::visit(_node);
                if(kleene_op == '(')
                    std::cout << ')';
                else
                    std::cout << kleene_op;
            }
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
