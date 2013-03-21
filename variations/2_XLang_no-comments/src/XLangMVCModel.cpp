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
#include "XLangTreeContext.h" // TreeContext
#include "node/XLangNode.h" // node::NodeIdentIFace
#include "XLangType.h" // uint32_t
#include <stdarg.h> // va_list
#include <string.h> // memset
#include <string> // std::string
#include <vector> // std::vector

#ifdef EXTERN_INCLUDE_PATH
    #define TIXML_USE_TICPP
#endif
#ifdef TIXML_USE_TICPP
    #include "ticpp/ticpp.h"
    #include <map> // std::map

    // prototype
    extern uint32_t name_to_id(std::string name);
#endif

namespace xl { namespace mvc {

node::SymbolNode* MVCModel::make_symbol(TreeContext* tc, uint32_t sym_id, YYLTYPE loc, size_t size, ...)
{
    va_list ap;
    va_start(ap, size);
    node::SymbolNode* node = new (PNEW(tc->alloc(), node::, NodeIdentIFace))
            node::SymbolNode(sym_id, loc, size, ap);
    va_end(ap);
    return node;
}

template<>
node::NodeIdentIFace* MVCModel::make_term<
        node::TermInternalType<node::NodeIdentIFace::IDENT>::type
        >(TreeContext* tc, uint32_t sym_id, YYLTYPE loc,
                node::TermInternalType<node::NodeIdentIFace::IDENT>::type value)
{
    return new (PNEW(tc->alloc(), node::, NodeIdentIFace))
            node::TermNode<node::NodeIdentIFace::IDENT>(sym_id, loc, value); // supports non-trivial dtor
}

#ifdef TIXML_USE_TICPP
static node::NodeIdentIFace* make_term(
        TreeContext* tc, std::string node_type, uint32_t sym_id, std::string node_value)
{
    static YYLTYPE dummy_loc;
    memset(&dummy_loc, 0, sizeof(dummy_loc));
    if(node_type == "int")
        return mvc::MVCModel::make_term(tc, sym_id, dummy_loc,
                static_cast<node::TermInternalType<node::NodeIdentIFace::INT>::type>(
                        atoi(node_value.c_str())
                        ));
    if(node_type == "float")
        return mvc::MVCModel::make_term(tc, sym_id, dummy_loc,
                static_cast<node::TermInternalType<node::NodeIdentIFace::FLOAT>::type>(
                        atof(node_value.c_str())
                        ));
    if(node_type == "ident")
        return mvc::MVCModel::make_term(tc, sym_id, dummy_loc,
                static_cast<node::TermInternalType<node::NodeIdentIFace::IDENT>::type>(
                        tc->alloc_unique_string(node_value)
                        ));
    return NULL;
}

static node::NodeIdentIFace* visit(TreeContext* tc, ticpp::Node* node)
{
    static YYLTYPE dummy_loc;
    memset(&dummy_loc, 0, sizeof(dummy_loc));
    if(dynamic_cast<ticpp::Document*>(node))
    {
        uint32_t sym_id = 0;
        node::SymbolNode* dest_node = mvc::MVCModel::make_symbol(tc, sym_id, dummy_loc, 0);
        if(!node->NoChildren())
        {
            ticpp::Iterator<ticpp::Node> child;
            for(child = child.begin(node); child != child.end(); child++)
                dest_node->push_back(visit(tc, child.Get()));
            if(dest_node->size() == 1)
            {
                node::NodeIdentIFace* dest_child = (*dest_node)[0];
                tc->alloc()._free(dest_node);
                return dest_child;
            }
        }
        return dest_node;
    }
    if(dynamic_cast<ticpp::Declaration*>(node))
        return NULL;
    std::string node_type, node_value;
    uint32_t sym_id;
    ticpp::Element* elem = dynamic_cast<ticpp::Element*>(node);
    if(elem)
    {
        std::map<std::string, std::string> attrib_map; // in case you need it
        ticpp::Iterator<ticpp::Attribute> attribute;
        for(attribute = attribute.begin(elem); attribute != attribute.end(); attribute++)
        {
            std::string attrib_name, attrib_value;
            attribute->GetName(&attrib_name);
            attribute->GetValue(&attrib_value);
            attrib_map[attrib_name] = attrib_value;
        }
        node_type = attrib_map["type"];
        node_value = attrib_map["value"];
        sym_id = name_to_id(node_type);
    }
    if(node->NoChildren())
        return make_term(tc, node_type, sym_id, node_value);
    else
    {
        node::SymbolNode* dest_node = mvc::MVCModel::make_symbol(tc, sym_id, dummy_loc, 0);
        ticpp::Iterator<ticpp::Node> child;
        for(child = child.begin(node); child != child.end(); child++)
            dest_node->push_back(visit(tc, child.Get()));
        return dest_node;
    }
}
#endif

node::NodeIdentIFace* MVCModel::make_ast(TreeContext* tc, std::string filename)
{
#ifdef TIXML_USE_TICPP
    ticpp::Document doc(filename.c_str());
    doc.LoadFile();
    return visit(tc, &doc);
#else
    return NULL;
#endif
}

} }
