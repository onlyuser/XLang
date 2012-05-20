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

#include "visitor/XLangPrinter.h" // visitor::LispPrinter
#include <iostream> // std::cout
#include <sstream> // std::stringstream

static std::string ptr_to_string(const void* x)
{
    std::stringstream ss;
    ss << '_' << x;
    std::string s = ss.str();
    return s;
}

static bool include_node_uid;
static size_t depth;

namespace xl { namespace visitor {

void LispPrinter::visit(const node::InnerNodeIFace* _node)
{
    std::cout << '(' << _node->name() << ' ';
    bool more;
    do
    {
        more = visit_next_child(_node);
        if(more)
            std::cout << ' ';
    } while(more);
    std::cout << ')';
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::INT>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << ptr_to_string(_node) << " ";
    std::cout << "type=\"" << _node->name() << "\" value=";
    std::cout << _node->value();
    std::cout << "/>";
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::FLOAT>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << ptr_to_string(_node) << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << _node->value() << "/>";
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::STRING>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << ptr_to_string(_node) << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << _node->value() << "/>";
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::CHAR>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << ptr_to_string(_node) << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << _node->value() << "/>";
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::IDENT>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << ptr_to_string(_node) << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << _node->value() << "/>";
}

void XMLPrinter::visit(const node::InnerNodeIFace* _node)
{
    if(NULL == _node)
        return;
    std::cout << std::string(depth*4, ' ');
    std::cout << "<inner ";
    if(include_node_uid)
        std::cout << "id=" << ptr_to_string(_node) << " ";
    std::cout << "type=\"" << _node->name() << "\">" << std::endl;
    depth++;
    bool more;
    do
    {
        more = visit_next_child(_node);
        std::cout << std::endl;
    } while(more);
    depth--;
    std::cout << std::string(depth*4, ' ') << "</inner>";
    if(depth == 0)
        std::cout << std::endl;
}

} }
