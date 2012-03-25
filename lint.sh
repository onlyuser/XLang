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
    echo "SYNTAX: lint <LINT_TOOL> <INPUT_FILE> <OUTPUT_FILE> <LINT_FLAGS..>"
}

if [ $# -lt 3 ];
then
    echo "fail! -- requires at least 3 arguments! ==> $@"
    show_help
    exit 1
fi

TEMP_FILE=`mktemp`
trap "rm $TEMP_FILE" EXIT

LINT_TOOL=$1
INPUT_FILE=$2
OUTPUT_FILE=$3
PASS_FILE=${OUTPUT_FILE}.pass
FAIL_FILE=${OUTPUT_FILE}.fail

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

echo "$LINT_TOOL $INPUT_FILE $LINT_FLAGS >& $TEMP_FILE"
FILE_DATA=`$LINT_TOOL $INPUT_FILE $LINT_FLAGS |& grep -v "not found\|Checking" |& sed "/^$/d"`
if [ -n "$FILE_DATA" ];
then
    echo "fail!"
    echo -e "$FILE_DATA" > $FAIL_FILE
    exit 1
fi

echo "success!" | tee $PASS_FILE
exit 0
