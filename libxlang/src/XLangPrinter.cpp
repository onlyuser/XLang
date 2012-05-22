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
    if(_node->is_root())
        std::cout << std::endl;
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::INT>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << _node->uid() << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << _node->value() << "/>" << std::endl;
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::FLOAT>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << _node->uid() << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << _node->value() << "/>" << std::endl;
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::STRING>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << _node->uid() << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << _node->value() << "/>" << std::endl;
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::CHAR>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << _node->uid() << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << _node->value() << "/>" << std::endl;
}

void XMLPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::IDENT>* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<leaf ";
    if(include_node_uid)
        std::cout << "id=" << _node->uid() << " ";
    std::cout << "type=\"" << _node->name() << "\" value=" << *_node->value() << "/>" << std::endl;
}

void XMLPrinter::visit(const node::InnerNodeIFace* _node)
{
    std::cout << std::string(depth*4, ' ');
    std::cout << "<inner ";
    if(include_node_uid)
        std::cout << "id=" << _node->uid() << " ";
    std::cout << "type=\"" << _node->name() << "\">" << std::endl;
    depth++;
    DefaultTour::visit(_node);
    depth--;
    std::cout << std::string(depth*4, ' ') << "</inner>" << std::endl;
}

void DotPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::INT>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << _node->value() << "\"," << std::endl <<
            "\t\tshape=\"ellipse\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::FLOAT>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << _node->value() << "\"," << std::endl <<
            "\t\tshape=\"ellipse\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::STRING>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << _node->value() << "\"," << std::endl <<
            "\t\tshape=\"ellipse\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::CHAR>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << _node->value() << "\"," << std::endl <<
            "\t\tshape=\"ellipse\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::LeafNodeIFace<node::NodeIdentIFace::IDENT>* _node)
{
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << *_node->value() << "\"," << std::endl <<
            "\t\tshape=\"ellipse\"" << std::endl <<
            "\t];" << std::endl;
    std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
}

void DotPrinter::visit(const node::InnerNodeIFace* _node)
{
    if(_node->is_root())
        std::cout << "digraph g {" << std::endl;
    std::cout << "\t" << _node->uid() << " [" << std::endl <<
            "\t\tlabel=\"" << _node->name() << "\"," << std::endl <<
            "\t\tshape=\"ellipse\"" << std::endl <<
            "\t];" << std::endl;
    DefaultTour::visit(_node);
    if(!_node->is_root())
        std::cout << '\t' << _node->parent()->uid() << "->" << _node->uid() << ";" << std::endl;
    if(_node->is_root())
        std::cout << "}" << std::endl;
}

} }