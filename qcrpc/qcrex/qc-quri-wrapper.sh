#!/bin/sh

if test $# -ne 2; then
    exit 1
fi

echo "'$0' '${1}' '${2}'" 1>&2
exit 0

