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
#include <iostream> // std::cout
#include <string> // std::string
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

static std::string gen_name(std::string stem)
{
    static std::map<std::string, int> tally_map;
    std::stringstream ss;
    ss << stem << '_' << tally_map[stem]++;
    return ss.str();
}

static xl::node::NodeIdentIFace* find_clone_of_original_recursive(
        const xl::node::NodeIdentIFace* root,
        const xl::node::NodeIdentIFace* original)
{
    if(!root || !original)
        return NULL;
    const xl::node::SymbolNodeIFace* root_sym = dynamic_cast<const xl::node::SymbolNodeIFace*>(root);
    if(!root_sym)
        return NULL;
    static const xl::node::NodeIdentIFace* temp; // NOTE: must be static for compile-time closure!
    temp = original; // NOTE: do not combine with previous line! -- must assign every time
    xl::node::NodeIdentIFace* result = root_sym->find_if([](const xl::node::NodeIdentIFace* _node) {
            return _node->original() == temp;
            });
    if(result)
        return result;
    for(size_t i = 0; i < root_sym->size(); i++)
    {
        result = find_clone_of_original_recursive((*root_sym)[i], original);
        if(result)
            return result;
    }
    return NULL;
}

static void replace_node(
        const xl::node::NodeIdentIFace* find_node,
        const xl::node::NodeIdentIFace* replace_node)
{
    if(!find_node)
        return;
    xl::node::SymbolNodeIFace* parent =
            dynamic_cast<xl::node::SymbolNodeIFace*>(find_node->parent());
    if(parent)
    {
        parent->replace(
                const_cast<xl::node::NodeIdentIFace*>(find_node),
                const_cast<xl::node::NodeIdentIFace*>(replace_node));
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

static std::string get_string_from_term_node(const xl::node::NodeIdentIFace* ident_node)
{
    switch(ident_node->type())
    {
        case xl::node::NodeIdentIFace::IDENT:
            {
                const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::IDENT>* ident_term =
                        dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::IDENT>*>(ident_node);
                if(!ident_term)
                    return "";
                const std::string* value_ptr = ident_term->value();
                if(!value_ptr)
                    return "";
                return *value_ptr;
            }
        case xl::node::NodeIdentIFace::STRING:
            {
                const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>* string_term =
                        dynamic_cast<const xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(ident_node);
                if(!string_term)
                    return "";
                std::string* value_ptr = string_term->value();
                if(!value_ptr)
                    return "";
                return *value_ptr;
            }
        default:
            return "";
    }
}

// kleene_node --> paren_node --> alt_node
static const xl::node::NodeIdentIFace* get_alt_node_from_kleene_node(
        const xl::node::NodeIdentIFace* kleene_node)
{
    if(!kleene_node)
        return NULL;
    xl::node::NodeIdentIFace* paren_node =
            (*dynamic_cast<const xl::node::SymbolNodeIFace*>(kleene_node))[0];
    if(!paren_node)
        return NULL;
    return (*dynamic_cast<xl::node::SymbolNodeIFace*>(paren_node))[0];
}

// kleene_node --> alt_node (up) --> action_node
static std::string get_action_string_from_kleene_node(
        const xl::node::NodeIdentIFace* kleene_node)
{
    if(!kleene_node)
        return NULL;
    const xl::node::NodeIdentIFace* alt_node = get_ancestor_node(ID_ALT, kleene_node);
    if(!alt_node)
        return NULL;
    xl::node::NodeIdentIFace* action_node = (*dynamic_cast<const xl::node::SymbolNodeIFace*>(alt_node))[1];
    if(!action_node)
        return NULL;
    xl::node::NodeIdentIFace* action_string_node =
            (*dynamic_cast<const xl::node::SymbolNodeIFace*>(action_node))[0];
    return get_string_from_term_node(action_string_node);
}

static std::string get_lhs_value_from_rule_node(const xl::node::NodeIdentIFace* rule_node)
{
    if(!rule_node)
        return "";
    xl::node::NodeIdentIFace* lhs_node =
            (*dynamic_cast<const xl::node::SymbolNodeIFace*>(rule_node))[0];
    if(!lhs_node)
        return "";
    return get_string_from_term_node(lhs_node);
}

static xl::node::NodeIdentIFace* make_stem_rule(
        std::string name,
        const xl::node::NodeIdentIFace* rule_node,
        const xl::node::NodeIdentIFace* kleene_node,
        xl::TreeContext* tc)
{
    if(!rule_node || !kleene_node)
        return NULL;
    xl::node::NodeIdentIFace* rule_node_copy = rule_node->clone(tc);
    xl::node::NodeIdentIFace* kleene_node_copy =
            find_clone_of_original_recursive(rule_node_copy, kleene_node);
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
        std::string action_string, xl::TreeContext* tc)
{
	//program_0:
	//      /* empty */ {
	//                /* AAA */
	//                $$ = new std::vector<
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

    std::string empty_case_action = " $$ = new std::vector<"
    		"xl::node::TermInternalType<xl::node::NodeIdentIFace::SYMBOL>::type"
    		">; ";
    std::string recurse_case_action = " $1->push_back($2); $$ = $1; ";

    xl::node::NodeIdentIFace* node =
            MAKE_SYMBOL(tc, ID_RULE, 2,
                    MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name1)),
                    MAKE_SYMBOL(tc, ID_ALTS, 2,
                            MAKE_SYMBOL(tc, ID_ALT, 1,
                                    MAKE_SYMBOL(tc, ID_ACTION_BLOCK, 1,
                                            MAKE_TERM(ID_STRING, tc->alloc_string(empty_case_action))
                                            )
                                    ),
                            MAKE_SYMBOL(tc, ID_ALT, 2,
                                    MAKE_SYMBOL(tc, ID_TERMS, 2,
                                            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name1)),
                                            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name2))
                                            ),
                                    MAKE_SYMBOL(tc, ID_ACTION_BLOCK, 1,
                                            MAKE_TERM(ID_STRING, tc->alloc_string(recurse_case_action))
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

    std::string empty_case_action    = " $$ = NULL; ";
    std::string optional_case_action = " $$ = $1; ";

    xl::node::NodeIdentIFace* node =
            MAKE_SYMBOL(tc, ID_RULE, 2,
                    MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name1)),
                    MAKE_SYMBOL(tc, ID_ALTS, 2,
                            MAKE_SYMBOL(tc, ID_ALT, 1,
                                    MAKE_SYMBOL(tc, ID_ACTION_BLOCK, 1,
                                            MAKE_TERM(ID_STRING, tc->alloc_string(empty_case_action))
                                            )
                                    ),
                            MAKE_SYMBOL(tc, ID_ALT, 2,
                                    MAKE_SYMBOL(tc, ID_TERMS, 1,
                                            MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name2))
                                            ),
                                    MAKE_SYMBOL(tc, ID_ACTION_BLOCK, 1,
                                            MAKE_TERM(ID_STRING, tc->alloc_string(optional_case_action))
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

static void enqueue_add_to_symbols(
        const xl::node::NodeIdentIFace* symbols_attach_loc,
        std::string after_name,
        std::string name,
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* insertions_after,
        xl::TreeContext* tc)
{
    if(!symbols_attach_loc)
        return;
    xl::node::SymbolNodeIFace* symbols_attach_loc_symbol =
            const_cast<xl::node::SymbolNodeIFace*>(
                    dynamic_cast<const xl::node::SymbolNodeIFace*>(symbols_attach_loc));
    if(!symbols_attach_loc_symbol)
        return;
    static std::string temp; // NOTE: must be static for compile-time closure!
    temp = after_name; // NOTE: do not combine with previous line! -- must assign every time
    xl::node::NodeIdentIFace* result =
            symbols_attach_loc_symbol->find_if([](const xl::node::NodeIdentIFace* _node) {
                    return get_string_from_term_node(_node) == temp;
                    });
    if(result)
        (*insertions_after)[result].push_back(MAKE_TERM(ID_IDENT, tc->alloc_unique_string(name)));
}

static void enqueue_changes_for_kleene_closure(
        std::map<std::string, const xl::node::NodeIdentIFace*>*                          symbols_attach_loc_map,
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* insertions_after,
        std::map<const xl::node::NodeIdentIFace*, std::list<xl::node::NodeIdentIFace*>>* append_to,
        std::map<const xl::node::NodeIdentIFace*, xl::node::NodeIdentIFace*>*            replacements,
        const xl::node::NodeIdentIFace*                                                  kleene_node,
        xl::TreeContext* tc)
{
    const xl::node::NodeIdentIFace* rule_node = get_ancestor_node(ID_RULE, kleene_node);
    std::string lhs_value = get_lhs_value_from_rule_node(rule_node);
    std::string name1 = gen_name(lhs_value);
    std::string name2 = gen_name(lhs_value);
    const xl::node::NodeIdentIFace* alt_node = get_alt_node_from_kleene_node(kleene_node);
    xl::node::NodeIdentIFace* term_rule = make_term_rule(name2, alt_node, tc);
    if(term_rule)
    {
#ifdef DEBUG_EBNF
        std::cout << ">>> (term_rule)" << std::endl;
        EBNFPrinter v(tc); v.visit_any(term_rule); std::cout << std::endl;
        std::cout << "<<< (term_rule)" << std::endl;
#endif
        if(symbols_attach_loc_map)
            enqueue_add_to_symbols(
                    (*symbols_attach_loc_map)[lhs_value],
                    lhs_value,
                    name2,
                    insertions_after,
                    tc);
        if(insertions_after)
            (*insertions_after)[rule_node].push_back(term_rule);
    }
    xl::node::NodeIdentIFace* recursive_rule = NULL;
    switch(kleene_node->sym_id())
    {
        case '+': recursive_rule = make_recursive_rule_plus(name1, name2, tc); break;
        case '*':
            {
                std::string action_string = get_action_string_from_kleene_node(kleene_node);
                recursive_rule = make_recursive_rule_star(name1, name2, action_string, tc);
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
        if(symbols_attach_loc_map)
            enqueue_add_to_symbols(
                    (*symbols_attach_loc_map)[lhs_value],
                    lhs_value,
                    name1,
                    insertions_after,
                    tc);
        if(insertions_after)
            (*insertions_after)[rule_node].push_back(recursive_rule);
    }
    xl::node::NodeIdentIFace* stem_rule = make_stem_rule(name1, rule_node, kleene_node, tc);
    if(stem_rule)
    {
#ifdef DEBUG_EBNF
        std::cout << ">>> (stem_rule)" << std::endl;
        EBNFPrinter v(tc); v.visit_any(stem_rule); std::cout << std::endl;
        std::cout << "<<< (stem_rule)" << std::endl;
#endif
        if(replacements)
            (*replacements)[rule_node] = stem_rule;
    }
}

void EBNFPrinter::visit(const xl::node::SymbolNodeIFace* _node)
{
#ifdef DEBUG_EBNF
    if(_node->sym_id() == ID_RULE || is_kleene_node(_node))
        std::cout << '[';
#endif
    static bool entered_kleen_closure = false;
    static const xl::node::NodeIdentIFace *proto_block = NULL, *union_block = NULL;
    bool more;
    switch(_node->sym_id())
    {
        case ID_GRAMMAR:
            entered_kleen_closure = false;
            visit_next_child(_node);
            std::cout << std::endl << std::endl << "%%" << std::endl << std::endl;
            visit_next_child(_node);
            std::cout << std::endl << std::endl << "%%";
            visit_next_child(_node);
            std::cout << std::endl;
            break;
        case ID_DEFINITIONS:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl;
            } while(more);
            break;
        case ID_DECL:
            std::cout << '%';
            more = visit_next_child(_node);
            if(more)
            {
                std::cout << ' ';
                visit_next_child(_node);
            }
            break;
        case ID_DECL_EQ:
            std::cout << '%';
            visit_next_child(_node);
            std::cout << '=';
            visit_next_child(_node);
            break;
        case ID_DECL_BRACE:
            std::cout << '%';
            visit_next_child(_node);
            std::cout << '<';
            visit_next_child(_node);
            std::cout << "> ";
            visit_next_child(_node);
            break;
        case ID_PROTO_BLOCK:
            std::cout << "%{";
            std::cout << *dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
                    (*_node)[0])->value();
            std::cout << "%}";
            proto_block = _node;
            break;
        case ID_UNION_BLOCK:
            std::cout << std::endl << '{' << std::endl;
            visit_next_child(_node);
            std::cout << std::endl << '}';
            union_block = _node;
            break;
        case ID_DECL_STMTS:
            do
            {
                std::cout << '\t';
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl;
            } while(more);
            break;
        case ID_DECL_STMT:
            visit_next_child(_node);
            std::cout << ';';
            break;
        case ID_DECL_CHUNKS:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << ' ';
            } while(more);
            break;
        case ID_DECL_CHUNK:
            std::cout << *dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
                    (*_node)[0])->value();
            break;
        case ID_SYMBOLS:
            do
            {
                xl::node::NodeIdentIFace* child = NULL;
                more = visit_next_child(_node, &child);
                if(child)
                {
                    std::string s = get_string_from_term_node(child);
                    if(!s.empty() && m_changes)
                        m_changes->m_symbols_attach_loc_map[s] = _node;
                }
                if(more)
                    std::cout << ' ';
            } while(more);
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
        case ID_ALTS:
            do
            {
                more = visit_next_child(_node);
                if(more)
                    std::cout << std::endl << "\t| ";
            } while(more);
            break;
        case ID_ALT:
            if(visit_next_child(_node))
                visit_next_child(_node);
            break;
        case ID_ACTION_BLOCK:
            std::cout << " {";
            std::cout << *dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
                    (*_node)[0])->value();
            std::cout << '}';
            break;
        case ID_TERMS:
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
            if(!entered_kleen_closure)
            {
                if(m_changes)
                    enqueue_changes_for_kleene_closure(
                            &m_changes->m_symbols_attach_loc_map,
                            &m_changes->m_insertions_after,
                            &m_changes->m_append_to,
                            &m_changes->m_replacements,
                            _node,
                            m_tc);
                entered_kleen_closure = true;
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
            std::cout << *dynamic_cast<xl::node::TermNodeIFace<xl::node::NodeIdentIFace::STRING>*>(
                    (*_node)[0])->value();
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
