#!/bin/sh

CPPFLAGS=''
CFLAGS=''
LIBS=''

PKG=${1}

if test ! -z "${PKG}"; then
    pkg-config --cflags "${PKG}" > /dev/null 2>&1
    if test $? -ne 0; then
	exit 1
    fi
    for i in `pkg-config --cflags ${PKG}`; do
	case $i in
	    -I*|-U*|-D*)
		CPPFLAGS="${CPPFLAGS} ${i}";;
	    *)
		CFLAGS="${CFLAGS} ${i}";;
	esac
    done

    for i in `pkg-config --libs ${PKG}`; do
	case $i in
	    -L*)
		rpath=`echo ${i} | sed 's:^-L:-Wl,-rpath -Wl,:g'`
		LIBS="${LIBS} ${i} ${rpath}";;
	    *)
		LIBS="${LIBS} ${i}";;
	esac
    done

    echo "PKG_CPPFLAGS = ${CPPFLAGS}"
    echo "PKG_CFLAGS = ${CFLAGS}"
    echo "PKG_LIBS = ${LIBS}"
    exit 0
else
    exit 1
fi
