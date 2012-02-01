#!/bin/bash

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

show_help()
{
    echo "SYNTAX: lint <LINT_EXEC> <INPUT_FILE> <OUTPUT_FILE> <LINT_FLAGS..>"
}

if [ $# -lt 3 ];
then
    echo "fail! -- requires at least 3 arguments! ==> $@"
    show_help
    exit 1
fi

LINT_EXEC=$1
INPUT_FILE=$2
OUTPUT_FILE=$3

if [ ! -f $INPUT_FILE ];
then
    echo "fail! -- <INPUT_FILE> not found! ==> $INPUT_FILE"
    exit 1
fi

COUNT=0
for ARG in "$@"
do
    ((COUNT+=1))
    if [ $COUNT -le 3 ];
    then
        continue
    fi
    LINT_FLAGS="$LINT_FLAGS $ARG"
done

echo "$LINT_EXEC $INPUT_FILE $LINT_FLAGS >& $OUTPUT_FILE"
$LINT_EXEC $INPUT_FILE $LINT_FLAGS |& grep -v "not found" >& $OUTPUT_FILE

echo "success!"
exit 0
