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

#==================
# external defines
#==================

# BUILD_PATH
# BINARY
# INPUT_MODE

#==================
# compile flags
#==================

PARENT = ..
SCRIPT_PATH = $(PARENT)/scripts
TEST_SH = $(SCRIPT_PATH)/test.sh
PURE_SH = $(SCRIPT_PATH)/pure.sh
EXPORT_SH = $(SCRIPT_PATH)/export.sh

#==================
# all
#==================

.DEFAULT_GOAL : all
all : test

#==================
# test
#==================

ifneq ($(strip $(BASIC_SUITE)),)
    TEST_PATH = basic_suite
    TEST_FILE_STEMS = \
            0_simple \
            1_op_prec \
            2_grouping \
            3_identifier
endif
ifneq ($(strip $(ADVANCED_SUITE_COMMENTS)),)
    TEST_PATH = advanced_suite
    TEST_FILE_STEMS = \
            4_comments
endif
ifneq ($(strip $(ADVANCED_SUITE_STRINGS)),)
    TEST_PATH = advanced_suite
    TEST_FILE_STEMS = \
            5_strings
endif
TEST_FILES = $(patsubst %, $(BUILD_PATH)/%.test, $(TEST_FILE_STEMS))
TEST_PASS_FILES = $(patsubst %, %.pass, $(TEST_FILES))
TEST_FAIL_FILES = $(patsubst %, %.fail, $(TEST_FILES))
PURE_FILES = $(patsubst %, $(BUILD_PATH)/%.test.pure, $(TEST_FILE_STEMS))
PURE_PASS_FILES = $(patsubst %, %.pass, $(PURE_FILES))
PURE_FAIL_FILES = $(patsubst %, %.fail, $(PURE_FILES))
DOT_FILES = $(patsubst %, $(BUILD_PATH)/%.test.dot, $(TEST_FILE_STEMS))
DOT_PNG_FILES = $(patsubst %, %.png, $(DOT_FILES))
XML_FILES = $(patsubst %, $(BUILD_PATH)/%.test.xml, $(TEST_FILE_STEMS))

$(BUILD_PATH)/%.test.pass : $(BINARY) $(TEST_PATH)/%.test
	-$(TEST_SH) $(BINARY) --lisp $(INPUT_MODE) $(TEST_PATH)/$*.test $(TEST_PATH)/$*.gold \
			$(BUILD_PATH)/$*.test

.PHONY : test
test : $(TEST_PASS_FILES)

.PHONY : clean_test
clean_test :
	-rm $(TEST_PASS_FILES) $(TEST_FAIL_FILES)

#==================
# pure
#==================

GOLD_KEYWORD = "All heap blocks were freed -- no leaks are possible"

$(BUILD_PATH)/%.test.pure.pass : $(BINARY) $(TEST_PATH)/%.test
	-$(PURE_SH) $(BINARY) $(INPUT_MODE) $(TEST_PATH)/$*.test $(GOLD_KEYWORD) \
			$(BUILD_PATH)/$*.test.pure

.PHONY : pure
pure : $(PURE_PASS_FILES)

.PHONY : clean_pure
clean_pure :
	-rm $(PURE_PASS_FILES) $(PURE_FAIL_FILES)

#==================
# dot
#==================

$(BUILD_PATH)/%.test.dot.png : $(BINARY) $(TEST_PATH)/%.test
	-$(EXPORT_SH) $(BINARY) $(INPUT_MODE) $(TEST_PATH)/$*.test \
			png \
			$(BUILD_PATH)/$*.test.dot

.PHONY : dot
dot : $(DOT_PNG_FILES)

.PHONY : clean_dot
clean_dot :
	-rm $(DOT_FILES) $(DOT_PNG_FILES)

#==================
# xml
#==================

$(BUILD_PATH)/%.test.xml : $(BINARY) $(TEST_PATH)/%.test
	-$(EXPORT_SH) $(BINARY) $(INPUT_MODE) $(TEST_PATH)/$*.test \
			xml \
			$(BUILD_PATH)/$*.test

.PHONY : xml
xml : $(XML_FILES)

.PHONY : clean_xml
clean_xml :
	-rm $(XML_FILES)

#==================
# clean
#==================

.PHONY : clean
clean : clean_test clean_pure clean_dot clean_xml