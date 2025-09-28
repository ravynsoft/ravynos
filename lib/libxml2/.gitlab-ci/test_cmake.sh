#!/bin/sh

set -e

CFLAGS="-Werror $CFLAGS" \
cmake "$@" \
    -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS \
    -DCMAKE_INSTALL_PREFIX=libxml2-install \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -S . -B libxml2-build
cmake --build libxml2-build --target install

(cd libxml2-build && ctest -VV)

mkdir -p libxml2-install/share/libxml2
cp Copyright libxml2-install/share/libxml2
(cd libxml2-install &&
    tar -czf ../libxml2-$CI_COMMIT_SHORT_SHA-$SUFFIX.tar.gz *)
