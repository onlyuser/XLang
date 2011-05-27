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
LIB_PATH = $(TOP)/lib
PARENT_LIB_PATH = $(TOP)/../lib
SRC_PATH = $(TOP)/src
BUILD_PATH = $(TOP)/build
BIN_PATH = $(TOP)/bin
BINARY = $(BIN_PATH)/XLang.exe
COMMON_PATH = ../lib-common
COMMON_INCLUDE_PATH = $(COMMON_PATH)/include

INCLUDE_PATHS = $(INCLUDE_PATH) $(COMMON_INCLUDE_PATH)
INCLUDE_PATH_FLAGS = $(patsubst %, -I%, $(INCLUDE_PATHS))

LIB_PATHS = $(LIB_PATH) $(PARENT_LIB_PATH)
LIB_PATH_FLAGS = $(patsubst %, -L%, $(LIB_PATHS))

#LIBTEMS = glut32 glu32 opengl32
#LIBS = $(patsubst %, $(LIB_PATH)/lib%.a, $(LIBTEMS))
PARENT_LIBTEMS = common
PARENT_LIBS = $(patsubst %, $(PARENT_LIB_PATH)/lib%.a, $(PARENT_LIBTEMS))

LIB_FLAGS = $(patsubst %, -l%, $(LIBTEMS)) \
        $(patsubst %, -l%, $(PARENT_LIBTEMS))
CXXFLAGS = -Wall $(DEBUG) $(INCLUDE_PATH_FLAGS)
LDFLAGS = -Wall $(DEBUG) $(LIB_PATH_FLAGS) $(LIB_FLAGS)

.DEFAULT_GOAL : all
all : $(BINARY)

#==================
# lib
#==================

$(PARENT_LIBS) :
	cd $(COMMON_PATH); $(MAKE) -f Makefile all

#==================
# yacc
#==================

YACCTEMS = XLang.tab
YACC_GEN = $(patsubst %, $(SRC_PATH)/%.cpp, $(YACCTEMS)) \
        $(patsubst %, $(INCLUDE_PATH)/%.h, $(YACCTEMS))
.SECONDARY : $(YACC_GEN)

$(SRC_PATH)/%.tab.cpp $(INCLUDE_PATH)/%.tab.h : $(SRC_PATH)/%.y
	bison -d $<
	mv $*.tab.c $(SRC_PATH)/$*.tab.cpp
	mv $*.tab.h $(INCLUDE_PATH)/$*.tab.h

#==================
# lex
#==================

#LEXTEMS = lex.XLang
LEX_GEN = $(patsubst %, $(SRC_PATH)/%.cpp, $(LEXTEMS))
.SECONDARY : $(LEX_GEN)

$(SRC_PATH)/lex.%.cpp : $(SRC_PATH)/%.l
	flex $<
	mv lex.$*.c $(SRC_PATH)/lex.$*.cpp

#==================
# object
#==================

$(BUILD_PATH)/%.o : $(SRC_PATH)/%.cpp
	mkdir -p $(BUILD_PATH)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

#==================
# binary
#==================

CPPTEMS = $(YACCTEMS) $(LEXTEMS)
OBJECTS = $(patsubst %, $(BUILD_PATH)/%.o, $(CPPTEMS))

$(BINARY) : $(PARENT_LIBS) $(LIBS) $(OBJECTS)
	mkdir -p $(BIN_PATH)
	$(CXX) -o $@ $^ $(LDFLAGS)

#==================
# general
#==================

.PHONY : clean
clean :
	-rm $(BINARY) $(OBJECTS) $(YACC_GEN) $(LEX_GEN) $(PARENT_LIBS)
	-rmdir $(BIN_PATH) $(BUILD_PATH) $(PARENT_LIB_PATH)
	cd $(COMMON_PATH); $(MAKE) -f Makefile clean
