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

#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <map>
#include <string>

class Symbol
{
public:
    typedef enum { TYPE, FUNC, VAR } type_t;

    Symbol(type_t type, std::string name);
    type_t type() const      { return m_type; }
    std::string name() const { return m_name; }
    void print() const;

private:
    type_t      m_type;
    std::string m_name;
};

class SymbolTable
{
public:
    ~SymbolTable();
    static SymbolTable* instance();
    bool add_type(std::string name);
    bool add_func(std::string name);
    bool add_var(std::string name);
    bool lookup(Symbol::type_t *type, std::string name);
    void print() const;
    void reset();

private:
    typedef std::map<std::string, Symbol*> symbols_t;
    symbols_t m_symbols;

    SymbolTable();
    bool _add_name(Symbol::type_t type, std::string name);
};

#endif
