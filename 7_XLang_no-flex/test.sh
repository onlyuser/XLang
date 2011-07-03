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
    echo "SYNTAX: test <EXEC> <INPUT_FILE> <GOLD_KEYWORD> <GOLD_FILE> <RESULT_FILE>"
}

if [ $# -ne 5 ];
then
    echo "fail! -- requires 5 arguments! ==> $@"
    show_help
    exit 1
fi

TEMP_FILE=`mktemp`
trap "rm $TEMP_FILE" EXIT

EXEC=$1
INPUT_FILE=$2
GOLD_KEYWORD=$3
GOLD_FILE=$4
RESULT_FILE=$5
PASS_FILE=${RESULT_FILE}.pass
FAIL_FILE=${RESULT_FILE}.fail

if [ ! -f $INPUT_FILE ];
then
    echo "fail! -- <INPUT_FILE> not found! ==> $INPUT_FILE"
    exit 1
fi
if [ ! -f $GOLD_FILE ];
then
    echo "fail! -- <GOLD_FILE> not found! ==> $GOLD_FILE"
    exit 1
fi

$EXEC $INPUT_FILE | grep $GOLD_KEYWORD > $TEMP_FILE
diff $TEMP_FILE $GOLD_FILE | tee $PASS_FILE

if [ ${PIPESTATUS[0]} -ne 0 ]; # $? captures the last pipe
then
    echo "fail!"
    mv $PASS_FILE $FAIL_FILE
    exit 1
fi

if [ -f $FAIL_FILE ];
then
    rm $FAIL_FILE
fi
echo "success!" | tee $PASS_FILE
exit 0
