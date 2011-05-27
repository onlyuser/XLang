# Variations of a Flex-Bison parser
# -- based on "A COMPACT GUIDE TO LEX & YACC" by Tom Niemann
# Copyright (C) 2011 Jerry Chen <mailto:onlyuser@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

CXX=g++

#==================
# compile flags
#==================

TOP = .
INCLUDE_PATH = $(TOP)/include
SRC_PATH = $(TOP)/src
BUILD_PATH = $(TOP)/build
BIN_PATH = $(TOP)/bin
TARGET = $(BIN_PATH)/XLang.exe

COMMON = ../lib-common
COMMON_INCLUDE_PATH = $(COMMON)/include
COMMONRC_PATH = $(COMMON)/src
COMMON_BUILD_PATH = $(COMMON)/build

INCLUDE_PATHS = $(INCLUDE_PATH) $(COMMON_INCLUDE_PATH)
#LIB_PATHS = $(TOP)/lib
#LIBS = glut32 glu32 opengl32

INCLUDE_PATH_FLAGS = $(patsubst %, -I%, $(INCLUDE_PATHS))
LIB_PATH_FLAGS = $(patsubst %, -L%, $(LIB_PATHS))
LIB_FLAGS = $(patsubst %, -l%, $(LIBS))

CXXFLAGS = -Wall $(DEBUG) $(INCLUDE_PATH_FLAGS)
LDFLAGS = -Wall $(DEBUG) $(LIB_PATH_FLAGS) $(LIB_FLAGS)

#==================
# yacc-related
#==================

YACCTEMS = XLang.tab
YACCODEGEN_FILES = $(patsubst %, $(SRC_PATH)/%.cpp, $(YACCTEMS)) \
    $(patsubst %, $(INCLUDE_PATH)/%.h, $(YACCTEMS))
.SECONDARY : $(YACCODEGEN_FILES)

$(SRC_PATH)/%.tab.cpp $(INCLUDE_PATH)/%.tab.h : $(SRC_PATH)/%.y
	bison -d $<
	mv $*.tab.c $(SRC_PATH)/$*.tab.cpp
	mv $*.tab.h $(INCLUDE_PATH)/$*.tab.h

#==================
# lex-related
#==================

LEXTEMS = lex.XLang
LEXODEGEN_FILES = $(patsubst %, $(SRC_PATH)/%.cpp, $(LEXTEMS))
.SECONDARY : $(LEXODEGEN_FILES)

$(SRC_PATH)/lex.%.cpp : $(SRC_PATH)/%.l
	flex $<
	mv lex.$*.c $(SRC_PATH)/lex.$*.cpp

#==================
# cpp-related (common)
#==================

COMMONPPTEMS = XLangAlloc XLangView XLangViewG
COMMON_OBJECTS = $(patsubst %, $(COMMON_BUILD_PATH)/%.o, $(COMMONPPTEMS))

$(COMMON_BUILD_PATH)/%.o : $(COMMONRC_PATH)/%.cpp
	mkdir -p $(COMMON_BUILD_PATH)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

#==================
# cpp-related
#==================

CPPTEMS = $(YACCTEMS) $(LEXTEMS) XLangModel
OBJECTS = $(patsubst %, $(BUILD_PATH)/%.o, $(CPPTEMS))

$(BUILD_PATH)/%.o : $(SRC_PATH)/%.cpp
	mkdir -p $(BUILD_PATH)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET) : $(OBJECTS) $(COMMON_OBJECTS)
	mkdir -p $(BIN_PATH)
	$(CXX) -o $@ $^ $(LDFLAGS)

#==================
# general
#==================

.PHONY : clean
clean :
	rm $(TARGET) $(OBJECTS) $(YACCODEGEN_FILES) $(LEXODEGEN_FILES)
	rmdir $(BUILD_PATH) $(BIN_PATH)
	-rm $(COMMON_OBJECTS)
	-rmdir $(COMMON_BUILD_PATH) $(COMMON_BIN_PATH)
