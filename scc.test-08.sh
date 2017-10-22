#!/bin/ksh
#
# @(#)$Id: scc.test-08.sh,v 1.1 2017/10/18 05:49:53 jleffler Exp $
#
# Test driver for SCC: Printing empty comments

T_SCC=./scc             # Version of SCC under test
RCSKW="${RCSKWREDUCE:-rcskwreduce}"

[ -x "$T_SCC" ] || ${MAKE:-make} "$T_SCC" || exit 1

arg0=$(basename "$0" .sh)

usage()
{
    echo "Usage: $arg0 [-gq]" >&2
    exit 1
}

# -g  Generate result files
# -q  Quiet mode

qflag=no
gflag=no
while getopts gq opt
do
    case "$opt" in
    (q) qflag=yes;;
    (g) gflag=yes;;
    (*) usage;;
    esac
done
shift $((OPTIND - 1))
[ "$#" = 0 ] || usage

tmp=${TMPDIR:-/tmp}/scc-test
trap "rm -f $tmp.?; exit 1" 0 1 2 3 13 15

OUTPUT_DIR=Output
SOURCE=scc-test.example2.c
base="$arg0.example2"

{
suffix=".c"
fail=0
pass=0
# Don't quote file name or options - spaces need trimming
for options in "-e" "-en" "-ec"
do
    test=0
    "$T_SCC" "$options" "${SOURCE}" > "$tmp.1" 2> "$tmp.2"
    EXPOUT="$OUTPUT_DIR/$base$options.1"
    EXPERR="$OUTPUT_DIR/$base$options.2"
    if [ "$gflag" = yes ]
    then cp "$tmp.1" "$EXPOUT"
    elif cmp -s <($RCSKW "$tmp.1") <($RCSKW "$EXPOUT")
    then : OK
    else
        echo "Differences: $SOURCE - standard output"
        diff "$tmp.1" "$EXPOUT"
        test=1
    fi
    if [ "$gflag" = yes ]
    then cp "$tmp.2" "$EXPERR"
    elif cmp -s <($RCSKW "$tmp.2") <($RCSKW "$EXPERR")
    then : OK
    else
        echo "Differences: $SOURCE - standard error"
        diff "$tmp.2" "$EXPERR"
        test=1
    fi
    if [ "$gflag" = yes ]
    then
        echo "== GENERATED == ($options: $SOURCE)"
        echo "                ($EXPOUT $EXPERR)"
        : $((pass++))
    elif [ $test = 0 ]
    then
        echo "== PASS == ($options: $SOURCE)"
        : $((pass++))
    else
        echo "!! FAIL !! ($options: $SOURCE)"
        : $((fail++))
    fi
    rm -f "$tmp".?
done
if [ $fail = 0 ]
then echo "== PASS == ($pass tests OK)"
else echo "!! FAIL !! ($pass tests OK, $fail tests failed)"
fi
}

rm -f $tmp.?
trap 0
