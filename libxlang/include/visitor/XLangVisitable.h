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

#ifndef XLANG_VISITABLE_H_
#define XLANG_VISITABLE_H_

#include "visitor/XLangVisitor.h" // visitor::Visitor

namespace xlang { namespace visitor {

template<class T>
class Visitable
{
public:
    Visitable(T* instance) : m_instance(*instance)
    {}
    virtual ~Visitable()
    {}
    virtual void accept(Visitor<T>* v)
    {
        v->visit_any(&m_instance);
    }

private:
    T &m_instance;
};

} }

#endif
