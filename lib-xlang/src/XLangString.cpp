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

#include "XLangString.h" // Allocator
#include <string> // std::string
#include <sstream> // std::stringstream
#include <string.h> // strcpy

namespace xl {

std::string escape(std::string s)
{
    std::stringstream ss;
    for(size_t i = 0; i<s.length(); i++)
        ss << escape(s[i]);
    return ss.str();
}

std::string escape(char c)
{
    switch(c)
    {
        case '\r': return "\\r";
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\"': return "\\\"";
        case '\'': return "\\\'";
        case '\\': return "\\\\";
    }
    char buf[] = " \0";
    buf[0] = c;
    std::string s(buf);
    return s;
}

std::string unescape(std::string s)
{
    char* buf = new char[s.length()+1]; // can't use allocator for arrays
    strcpy(buf, s.c_str());
    int n = 0;
    bool unescape_next_char = false;
    for(int i = 0; buf[i]; i++)
    {
        if(unescape_next_char)
        {
            buf[i] = unescape(buf[i]);
            unescape_next_char = false;
        }
        else if('\\' == buf[i])
        {
            unescape_next_char = true;
            continue;
        }
        buf[n++] = buf[i];
    }
    buf[n] = '\0';
    std::string s2(buf);
    delete []buf;
    return s2;
}

char unescape(char c)
{
    switch(c)
    {
        case 'r': return '\r';
        case 'n': return '\n';
        case 't': return '\t';
    }
    return c;
}

}
