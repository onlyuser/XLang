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
    echo "Usage: `basename $0` EXEC INPUT_MODE INPUT_FILE OUTPUT_FILE_STEM"
}

if [ $# -ne 4 ]; then
    echo "fail! -- expect 4 arguments! ==> $@"
    show_help
    exit 1
fi

TEMP_FILE=`mktemp`
trap "rm $TEMP_FILE" EXIT

EXEC=$1
INPUT_MODE=$2
INPUT_FILE=$3
OUTPUT_FILE_STEM=$4
PNG_FILE=${OUTPUT_FILE_STEM}.png

if [ ! -f $INPUT_FILE ]; then
    echo "fail! -- INPUT_FILE not found! ==> $INPUT_FILE"
    exit 1
fi

case $INPUT_MODE in
    "file")
        $EXEC --dot --file $INPUT_FILE > $TEMP_FILE
        ;;
    "stdin")
        echo `cat $INPUT_FILE` | $EXEC --dot > $TEMP_FILE
        ;;
    "arg")
        $EXEC --dot --input `cat $INPUT_FILE` > $TEMP_FILE
        ;;
    *)
        echo "fail! -- invalid input mode"
        exit 1
        ;;
esac

DOT_TOOL="dot"
DOT_FLAGS="-Tpng"
$DOT_TOOL $DOT_FLAGS -o $PNG_FILE $TEMP_FILE

echo "success!"
