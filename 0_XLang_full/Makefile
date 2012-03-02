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

CXX = g++
LINT = cppcheck

#==================
# compile flags
#==================

TOP = .
INCLUDE_PATH = $(TOP)/include
LIB_PATH = $(TOP)/lib
SRC_PATH = $(TOP)/src
BUILD_PATH = $(TOP)/build
BIN_PATH = $(TOP)/bin
BINARY = $(BIN_PATH)/XLang.exe

COMMON = ../lib-common
COMMON_INCLUDE_PATH = $(COMMON)/include
COMMON_SRC_PATH = $(COMMON)/src
COMMON_BUILD_PATH = $(COMMON)/build

INCLUDE_PATHS = $(INCLUDE_PATH) $(COMMON_INCLUDE_PATH)
INCLUDE_PATH_FLAGS = $(patsubst %, -I%, $(INCLUDE_PATHS))

LIB_PATHS = $(LIB_PATH)
LIB_PATH_FLAGS = $(patsubst %, -L%, $(LIB_PATHS))

# NOTE: this project does not use OpenGL, but just for example, additional libs
#       can be added here.

#LIB_STEMS = glut32 glu32 opengl32
#LIBS = $(patsubst %, $(LIB_PATH)/lib%.a, $(LIB_STEMS))
LIB_FLAGS = $(patsubst %, -l%, $(LIBS))

#DEBUG = -g
CXXFLAGS = -Wall $(DEBUG) $(INCLUDE_PATH_FLAGS)
LDFLAGS = -Wall $(DEBUG) $(LIB_PATH_FLAGS) $(LIB_FLAGS)
LINTFLAGS = $(INCLUDE_PATH_FLAGS) -v --enable=all

TEST_PATH = $(TOP)/../test
TEST_SH = $(TOP)/test.sh
TEST_FILE_STEMS = AAA BBB CCC DDD EEE
TEST_KEYWORD = 'LISP'
TEST_PASS_FILES = $(patsubst %, $(BUILD_PATH)/%.pass, $(TEST_FILE_STEMS))
TEST_FAIL_FILES = $(patsubst %, $(BUILD_PATH)/%.fail, $(TEST_FILE_STEMS))

LINT_ENABLE = 0
LINT_SH = $(TOP)/../lint.sh

.DEFAULT_GOAL : all
all : $(BINARY)

#==================
# yacc
#==================

YACC_STEMS = XLang.tab
YACC_GEN = $(patsubst %, $(SRC_PATH)/%.cpp, $(YACC_STEMS)) \
		$(patsubst %, $(INCLUDE_PATH)/%.h, $(YACC_STEMS))
.SECONDARY : $(YACC_GEN)

$(SRC_PATH)/%.tab.cpp $(INCLUDE_PATH)/%.tab.h : $(SRC_PATH)/%.y
	bison -d $<
	mv $*.tab.c $(SRC_PATH)/$*.tab.cpp
	mv $*.tab.h $(INCLUDE_PATH)/$*.tab.h

#==================
# lex
#==================

LEX_STEMS = lex.XLang
LEX_GEN = $(patsubst %, $(SRC_PATH)/%.cpp, $(LEX_STEMS))
.SECONDARY : $(LEX_GEN)

$(SRC_PATH)/lex.%.cpp : $(SRC_PATH)/%.l
	flex $<
	mv lex.$*.c $(SRC_PATH)/lex.$*.cpp

#==================
# object (common)
#==================

COMMON_CPP_STEMS = XLangAlloc XLangNode XLangNodePrinterVisitor XLangMVCView
COMMON_OBJECTS = $(patsubst %, $(COMMON_BUILD_PATH)/%.o, $(COMMON_CPP_STEMS))
COMMON_LINT_FILES = $(patsubst %, $(COMMON_BUILD_PATH)/%.lint, $(COMMON_CPP_STEMS))

$(COMMON_BUILD_PATH)/%.o : $(COMMON_SRC_PATH)/%.cpp
	mkdir -p $(COMMON_BUILD_PATH)
	$(CXX) -c -o $@ $< $(CXXFLAGS)
	-$(LINT_SH) $(LINT_ENABLE) $(LINT) $< $(COMMON_BUILD_PATH)/$*.lint $(LINTFLAGS)

#==================
# object
#==================

$(BUILD_PATH)/%.o : $(SRC_PATH)/%.cpp
	mkdir -p $(BUILD_PATH)
	$(CXX) -c -o $@ $< $(CXXFLAGS)
	-$(LINT_SH) $(LINT_ENABLE) $(LINT) $< $(BUILD_PATH)/$*.lint $(LINTFLAGS)

#==================
# binary
#==================

CPP_STEMS = $(YACC_STEMS) $(LEX_STEMS) XLangMVCModel
OBJECTS = $(patsubst %, $(BUILD_PATH)/%.o, $(CPP_STEMS))
LINT_FILES = $(patsubst %, $(BUILD_PATH)/%.lint, $(CPP_STEMS))

$(BINARY) : $(OBJECTS) $(COMMON_OBJECTS)
	mkdir -p $(BIN_PATH)
	$(CXX) -o $@ $^ $(LDFLAGS)

#==================
# general
#==================

$(BUILD_PATH)/%.pass : $(BINARY) $(TEST_PATH)/%.test
	-$(TEST_SH) $(BINARY) $(TEST_PATH)/$*.test $(TEST_KEYWORD) $(TEST_PATH)/$*.gold $(BUILD_PATH)/$*

.PHONY : test
test : $(TEST_PASS_FILES)

.PHONY : clean
clean :
	-rm $(BINARY) $(OBJECTS) $(LINT_FILES) $(YACC_GEN) $(LEX_GEN) \
			$(TEST_PASS_FILES) $(TEST_FAIL_FILES)
	-rmdir $(BUILD_PATH) $(BIN_PATH)
	-rm $(COMMON_OBJECTS) $(COMMON_LINT_FILES)
	-rmdir $(COMMON_BUILD_PATH) $(COMMON_BIN_PATH)
