#include <SymbolTable.h>
#include <string>
#include <sstream>
#include <iostream>

Symbol::Symbol(type_t type, std::string name)
    : m_type(type),
      m_name(name)
{
}

void Symbol::print() const
{
    std::stringstream ss;
    ss << "\"" << m_name << "\"\t: ";
    switch(m_type) {
        case TYPE: ss << "type"; break;
        case FUNC: ss << "func"; break;
        case VAR:  ss << "var"; break;
        default:
            break;
    }
    ss << std::endl;
    std::cout << ss.str();
}

SymbolTable::SymbolTable()
{
}

SymbolTable::~SymbolTable()
{
    reset();
}

SymbolTable* SymbolTable::instance()
{
    static SymbolTable g_symbol_table;
    return &g_symbol_table;
}

bool SymbolTable::_add_symbol(Symbol::type_t type, std::string name)
{
    symbols_t::iterator p = m_symbols.find(name);
    if(p != m_symbols.end()) {
        std::cout << "Error: Name \"" << name << "\" already exists in symbol table!" << std::endl;
        return false;
    }
    m_symbols.insert(p, symbols_t::value_type(name, new Symbol(type, name)));
    return true;
}

bool SymbolTable::add_type(std::string name)
{
    return _add_symbol(Symbol::TYPE, name);
}

bool SymbolTable::add_func(std::string name)
{
    return _add_symbol(Symbol::FUNC, name);
}

bool SymbolTable::add_var(std::string name)
{
    return _add_symbol(Symbol::VAR, name);
}

bool SymbolTable::lookup(Symbol::type_t *type, std::string name)
{
    if(!type) {
        return false;
    }
    for(symbols_t::iterator p = m_symbols.begin(); p != m_symbols.end(); p++) {
        if((*p).first == name && (*p).second) {
            *type = (*p).second->type();
            return true;
        }
    }
    return false;
}

void SymbolTable::print() const
{
    std::cout << "Symbol table:" << std::endl;
    std::cout << "[name]\t: [meta-type]" << std::endl;
    for(symbols_t::const_iterator p = m_symbols.begin(); p != m_symbols.end(); p++) {
        if((*p).second) {
            (*p).second->print();
        }
    }
}

void SymbolTable::reset()
{
    for(symbols_t::iterator p = m_symbols.begin(); p != m_symbols.end(); p++) {
        if((*p).second) {
            delete (*p).second;
        }
    }
    m_symbols.clear();
}
