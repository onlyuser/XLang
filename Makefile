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

SUBPATHS = \
    0_XLang_full \
    1_XLang_no-strings \
    2_XLang_no-comments \
    3_XLang_no-locations \
    4_XLang_no-reentrant \
    5_XLang_stdio \
    6_XLang_file \
    7_XLang_no-flex

.PHONY : all
all :
	@for i in $(SUBPATHS); do \
	echo "make all in $$i..."; \
	(cd $$i; $(MAKE)); done

.PHONY : test
test :
	@for i in $(SUBPATHS); do \
	echo "make test in $$i..."; \
	(cd $$i; $(MAKE) test); done

.PHONY : clean
clean :
	@for i in $(SUBPATHS); do \
	echo "make clean in $$i..."; \
	(cd $$i; $(MAKE) clean); done
