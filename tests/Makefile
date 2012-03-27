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
# TEST_SH

#==================
# compile flags
#==================

TOP = .

#==================
# all
#==================

.DEFAULT_GOAL : all
all : test

#==================
# test
#==================

ifneq ($(strip $(BASIC_SUITE)),)
    TEST_PATH = $(TOP)/basic_suite
    TEST_FILE_STEMS = 0_simple 1_op_prec 2_grouping 3_identifier
endif
ifneq ($(strip $(ADVANCED_SUITE)),)
    TEST_PATH = $(TOP)/advanced_suite
    TEST_FILE_STEMS = 4_comments 5_strings
endif
TEST_FILES = $(patsubst %, $(BUILD_PATH)/%.test, $(TEST_FILE_STEMS))
TEST_PASS_FILES = $(patsubst %, %.pass, $(TEST_FILES))
TEST_FAIL_FILES = $(patsubst %, %.fail, $(TEST_FILES))
TEST_FILT_KEYWORD = 'LISP'

$(BUILD_PATH)/%.test.pass : $(BINARY) $(TEST_PATH)/%.test
	-$(TEST_SH) $(BINARY) $(TEST_PATH)/$*.test $(TEST_FILT_KEYWORD) $(TEST_PATH)/$*.gold \
			$(BUILD_PATH)/$*.test

.PHONY : test
test : $(TEST_PASS_FILES)

.PHONY : clean_tests
clean_tests :
	-rm $(TEST_PASS_FILES) $(TEST_FAIL_FILES)

#==================
# clean
#==================

.PHONY : clean
clean : clean_tests
