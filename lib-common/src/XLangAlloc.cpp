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

#include "XLangAlloc.h" // Allocator
#include <string> // std::string
#include <iostream> // std::cout
#include <stdlib.h> // malloc
#include <stddef.h> // size_t

MemChunk::MemChunk(size_t _size_bytes, std::string _filename, size_t _line_number)
    : m_size_bytes(_size_bytes), m_filename(_filename), m_lineber(_line_number)
{
    m_ptr = malloc(_size_bytes);
}

MemChunk::~MemChunk()
{
    if(!!m_ptr)
        free(m_ptr);
}

void MemChunk::dump() const
{
    std::cout << m_filename << ":" << m_lineber << " .. " << m_size_bytes << " bytes";
}

Allocator::Allocator(std::string name)
    : m_name(name), m_size_bytes(0)
{
}
Allocator::~Allocator()
{
    _free();
}

void* Allocator::_malloc(size_t size_bytes, std::string filename, size_t line_number)
{
    MemChunk* chunk = new MemChunk(size_bytes, filename, line_number);
    m_size_bytes += size_bytes;
    m_chunk_map.insert(internal_type::value_type(chunk->ptr(), chunk));
    return chunk->ptr();
}

void Allocator::_free(void* ptr)
{
    internal_type::iterator p = m_chunk_map.find(ptr);
    if(p != m_chunk_map.end())
    {
        MemChunk* chunk = (*p).second;
        m_size_bytes -= chunk->size();
        delete chunk;
    }
    m_chunk_map.erase(p);
}

void Allocator::_free()
{
    for(internal_type::iterator p = m_chunk_map.begin(); p != m_chunk_map.end(); p++)
        delete (*p).second;
    m_chunk_map.clear();
}

void Allocator::dump() const
{
    std::cout << "dumping allocator: " << m_name << std::endl;
    for(internal_type::const_iterator p = m_chunk_map.begin(); p != m_chunk_map.end(); p++)
    {
        (*p).second->dump();
        std::cout << std::endl;
    }
}

void* operator new(size_t size_bytes, Allocator &alloc, std::string filename, size_t line_number)
{
    return alloc._malloc(size_bytes, filename, line_number);
}
