#!/bin/bash
#
# @(#)$Id: scc.test-06.sh,v 8.1 2022/04/07 00:40:18 jleffler Exp $
#
# Test driver for SCC: using -c, -n in combination
#
# NB: This is a Bash script because it uses command substitution

T_SCC=./scc             # Version of SCC under test
RCSKWCMP="${RCSKWCMP:-./rcskwcmp}"
RCSKWREDUCE="${RCSKWREDUCE:-./rcskwreduce}"
export RCSKWCMP RCSKWREDUCE

[ -x "$T_SCC" ] || ${MAKE:-make} "$T_SCC" || exit 1

usage()
{
    echo "Usage: $(basename $0 .sh) [-gq]" >&2
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

tmp="${TMPDIR:-/tmp}/scc-test.$$"
trap "rm -f $tmp.?; exit 1" 0 1 2 3 13 15

OUTPUT_DIR=Output
EXAMPLE_C=scc-test.example2

{
fail=0
pass=0
# Don't quote file name or options - spaces need trimming
for standard in c90 c99 c++98
do
    base=$EXAMPLE_C
    for args in "" "-c" "-n" "-c -n"
    do
        test=0
        "$T_SCC" $args -S "$standard" "${base}.c" > "$tmp.1" 2> "$tmp.2"
        EXPECT="$base-$standard${args/ /}"
        EXPOUT="$OUTPUT_DIR/$EXPECT.1"
        EXPERR="$OUTPUT_DIR/$EXPECT.2"
        if [ "$gflag" = yes ]
        then cp "$tmp.1" "$EXPOUT"
        elif $RCSKWCMP "$tmp.1" "$EXPOUT" 
        then : OK
        else
            echo "Differences: $EXPECT - standard output (wanted vs actual)"
            diff "$EXPOUT" "$tmp.1"
            test=1
        fi
        if [ "$gflag" = yes ]
        then cp "$tmp.2" "$EXPERR"
        elif $RCSKWCMP "$tmp.2" "$EXPERR" 
        then : OK
        else
            echo "Differences: $EXPECT - standard error (wanted vs actual)"
            diff "$EXPERR" "$tmp.2"
            test=1
        fi
        if [ "$gflag" = yes ]
        then
            echo "== GENERATED == ($standard: $base.c)"
            echo "                ($EXPOUT $EXPERR)"
            : $((pass++))
        elif [ $test = 0 ]
        then
            echo "== PASS == ($standard: ${args:+$args }$base.c)"
            : $((pass++))
        else
            echo "!! FAIL !! ($standard: ${args:+$args }$base.c)"
            : $((fail++))
        fi
        rm -f "$tmp".?
    done
done
if [ $fail = 0 ]
then echo "== PASS == ($pass tests OK)"
else echo "!! FAIL !! ($pass tests OK, $fail tests failed)"
fi
}

rm -f $tmp.?
trap 0
