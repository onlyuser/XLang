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

OUTTER_PATH = ..
INCLUDE_PATH = include
LIB_PATH = lib
PARENT_LIB_PATH = $(OUTTER_PATH)/lib
SRC_PATH = src
BUILD_PATH = build
BIN_PATH = bin
BINARY = $(BIN_PATH)/libcommon.a
INSTALL_BINARY = $(PARENT_LIB_PATH)/libcommon.a

INCLUDE_PATHS = $(INCLUDE_PATH) $(COMMON_INCLUDE_PATH)
INCLUDE_PATH_FLAGS = $(patsubst %, -I%, $(INCLUDE_PATHS))

LIB_PATHS = $(LIB_PATH) $(PARENT_LIB_PATH)
LIB_PATH_FLAGS = $(patsubst %, -L%, $(LIB_PATHS))

# NOTE: this project does not use OpenGL, but just for example, additional libs
#       can be added here.

#LIB_STEMS = glut32 glu32 opengl32
#LIBS = $(patsubst %, $(LIB_PATH)/lib%.a, $(LIB_STEMS))
LIB_FLAGS = $(patsubst %, -l%, $(LIB_STEMS))

#DEBUG = -g
CXXFLAGS = -Wall $(DEBUG) $(INCLUDE_PATH_FLAGS)
LDFLAGS = -Wall $(DEBUG) $(LIB_PATH_FLAGS) $(LIB_FLAGS)
LINTFLAGS = $(INCLUDE_PATH_FLAGS) -v --enable=all

LINT_SH = $(OUTTER_PATH)/lint.sh

DOXY_CONFIG_PATH = $(OUTTER_PATH)/doc
DOXY_CONFIG_FILE = XLang.config

#==================
# all
#==================

.DEFAULT_GOAL : all
all : $(BINARY)

#==================
# objects
#==================

$(BUILD_PATH)/%.o : $(SRC_PATH)/%.cpp
	mkdir -p $(BUILD_PATH)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

.PHONY : clean_objects
clean_objects :
	-rm $(OBJECTS)

#==================
# binary
#==================

CPP_STEMS = \
        XLangAlloc \
        XLangNode \
        XLangNodePrinterVisitor \
        XLangMVCView \
        XLangMVCModel
OBJECTS = $(patsubst %, $(BUILD_PATH)/%.o, $(CPP_STEMS))
LINT_FILES = $(patsubst %, $(BUILD_PATH)/%.lint, $(CPP_STEMS))

$(BINARY) : $(LIBS) $(OBJECTS)
	mkdir -p $(BIN_PATH)
	$(AR) -cvq $@ $^
	mkdir -p $(PARENT_LIB_PATH)
	cp $@ $(INSTALL_BINARY)

.PHONY : clean_binary
clean_binary :
	-rm $(BINARY)
	-rm $(INSTALL_BINARY)

#==================
# lint
#==================

LINT_PASS_FILES = $(patsubst %, %.pass, $(LINT_FILES))
LINT_FAIL_FILES = $(patsubst %, %.fail, $(LINT_FILES))

$(BUILD_PATH)/%.lint.pass : $(SRC_PATH)/%.cpp
	mkdir -p $(BUILD_PATH)
	-$(LINT_SH) $(LINT) $< $(BUILD_PATH)/$*.lint.pass $(LINTFLAGS)

.PHONY : lint
lint : $(LINT_PASS_FILES)

.PHONY : clean_lint
clean_lint :
	-rm $(LINT_PASS_FILES) $(LINT_FAIL_FILES)

#==================
# doc
#==================

.PHONY : doc
doc :
	mkdir -p $(BUILD_PATH)
	-ln -s ../$(DOXY_CONFIG_PATH)/$(DOXY_CONFIG_FILE) $(BUILD_PATH)/$(DOXY_CONFIG_FILE)
	cd $(BUILD_PATH); doxygen $(DOXY_CONFIG_FILE)

.PHONY : clean_doc
clean_doc :
	rm -rf $(BUILD_PATH)/html
	rm -rf $(BUILD_PATH)/$(DOXY_CONFIG_FILE)

#==================
# clean
#==================

.PHONY : clean
clean : clean_binary clean_objects clean_lint clean_doc
	-rmdir $(BUILD_PATH) $(BIN_PATH) $(PARENT_LIB_PATH)
