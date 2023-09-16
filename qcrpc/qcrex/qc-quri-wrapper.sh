#!/bin/sh

if test $# -ne 3; then
    exit 1
fi

echo "'$0' '${1}' '${2}' '${3}'" 1>&2

echo 4 > ${3}
echo "0 12.0" >> ${3}
echo "1 0.9" >> ${3}
echo "2 8" >> ${3}
echo "3 0" >> ${3}

exit 0

