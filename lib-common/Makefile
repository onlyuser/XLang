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
BINARY = $(BIN_PATH)/libcommon.a

INCLUDE_PATHS = $(INCLUDE_PATH) $(COMMON_INCLUDE_PATH)
INCLUDE_PATH_FLAGS = $(patsubst %, -I%, $(INCLUDE_PATHS))

LIB_PATHS = $(LIB_PATH) $(PARENT_LIB_PATH)
LIB_PATH_FLAGS = $(patsubst %, -L%, $(LIB_PATHS))

# NOTE: this project does not use OpenGL, but just for example, additional libs
#       can be added here.

#LIB_STEMS = glut32 glu32 opengl32
#LIBS = $(patsubst %, $(LIB_PATH)/lib%.a, $(LIB_STEMS))
LIB_FLAGS = $(patsubst %, -l%, $(LIB_STEMS))

CXXFLAGS = -Wall $(DEBUG) $(INCLUDE_PATH_FLAGS)
LDFLAGS = -Wall $(DEBUG) $(LIB_PATH_FLAGS) $(LIB_FLAGS)

.DEFAULT_GOAL : all
all : $(BINARY)

#==================
# object
#==================

$(BUILD_PATH)/%.o : $(SRC_PATH)/%.cpp
	mkdir -p $(BUILD_PATH)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

#==================
# binary
#==================

CPP_STEMS = XLangAlloc XLangNode XLangNodePrinterVisitor XLangMVCModel XLangMVCView
OBJECTS = $(patsubst %, $(BUILD_PATH)/%.o, $(CPP_STEMS))

$(BINARY) : $(LIBS) $(OBJECTS)
	mkdir -p $(BIN_PATH)
	$(AR) -cvq $@ $^
	mkdir -p $(PARENT_LIB_PATH)
	cp $@ $(PARENT_LIB_PATH)

#==================
# general
#==================

.PHONY : clean
clean :
	-rm $(BINARY) $(OBJECTS)
	-rmdir $(BIN_PATH) $(BUILD_PATH)
