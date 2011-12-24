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

#ifndef XLANG_ALLOC_H_
#define XLANG_ALLOC_H_

#include <map> // std::map
#include <string> // std::string
#include <stddef.h> // size_t

class MemChunk
{
    void* m_ptr;
    size_t m_size_bytes;
    std::string m_filename;
    size_t m_line_number;

public:
    MemChunk(size_t _size_bytes, std::string _filename, size_t _line_number);
    ~MemChunk();
    void* ptr() const { return m_ptr; }
    size_t size() const { return m_size_bytes; }
    std::string filename() const { return m_filename; }
    size_t line_number() const { return m_line_number; }
    void dump() const;
};

class Allocator
{
    typedef std::map<void*, MemChunk*> internal_type;
    std::string m_name;
    internal_type m_chunk_map;
    size_t m_size_bytes;

public:
    Allocator(std::string _filename);
    ~Allocator();
    std::string name() const { return m_name; }
    size_t size() const { return m_size_bytes; }
    void* _malloc(size_t size_bytes, std::string filename, size_t line_number);
    void _free(void* ptr);
    void _free();
    void dump() const;
};

// NOTE: doesn't work forarrays
void* operator new(size_t size_bytes, Allocator &alloc, std::string filename, size_t line_number);

#endif
