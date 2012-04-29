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
    echo "Usage: `basename $0` <EXEC={xlang}> <EXEC_FLAGS> <INPUT_MODE={file|stin|arg}> <INPUT_FILE> <OUTPUT_FILE>"
}

if [ $# -ne 5 ]; then
    echo "fail! -- expect 5 arguments! ==> $@"
    show_help
    exit 1
fi

EXEC=$1
EXEC_FLAGS=$2
INPUT_MODE=$3
INPUT_FILE=$4
OUTPUT_FILE=$5

if [ ! -f $INPUT_FILE ]; then
    echo "fail! -- INPUT_FILE not found! ==> $INPUT_FILE"
    exit 1
fi

case $INPUT_MODE in
    "file")
        $EXEC $EXEC_FLAGS --in-file $INPUT_FILE > $OUTPUT_FILE
        ;;
    "stdin")
        cat $INPUT_FILE | $EXEC $EXEC_FLAGS > $OUTPUT_FILE
        ;;
    "arg")
        $EXEC $EXEC_FLAGS --expr `cat $INPUT_FILE` > $OUTPUT_FILE
        ;;
    *)
        echo "fail! -- invalid input mode"
        exit 1
        ;;
esac

#echo "success!"
