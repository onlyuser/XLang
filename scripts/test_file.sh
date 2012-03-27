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
    echo "SYNTAX: test <EXEC> <INPUT_FILE> <GOLD_KEYWORD> <GOLD_FILE> <OUTPUT_FILE>"
}

if [ $# -ne 5 ];
then
    echo "fail! -- requires 5 arguments! ==> $@"
    show_help
    exit 1
fi

TEMP_FILE_0=`mktemp`
trap "rm $TEMP_FILE_0" EXIT
TEMP_FILE_1=`mktemp`
trap "rm $TEMP_FILE_1" EXIT

EXEC=$1
INPUT_FILE=$2
GOLD_KEYWORD=$3
GOLD_FILE=$4
OUTPUT_FILE=$5
PASS_FILE=${OUTPUT_FILE}.pass
FAIL_FILE=${OUTPUT_FILE}.fail

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

$EXEC $INPUT_FILE | grep $GOLD_KEYWORD > $TEMP_FILE_0
diff $TEMP_FILE_0 $GOLD_FILE | tee $TEMP_FILE_1

if [ ${PIPESTATUS[0]} -ne 0 ]; # $? captures the last pipe
then
    echo "fail!"
    cp $TEMP_FILE_1 $FAIL_FILE # already trapped exit for TEMP_FILE_1
    exit 1
fi

echo "success!" | tee $PASS_FILE
exit 0
