#!/bin/bash -u

modules=(
    bigint
    bigfloat
    bigrat
    bignum
)

backends=(
    #FastCalc
    GMP
    Pari
    #GMPz
    #BitVect
    #LTM
)

dirname=$( dirname -- "$0" ) || exit
cd "$dirname" || exit

gitroot=$( git rev-parse --show-toplevel ) || exit
cd "$gitroot" || exit

for backend in ${backends[@]}; do
    for module in ${modules[@]}; do
        file=t/backend-${backend,,}-$module.t
        cat <<EOF >$file
# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More;

BEGIN {
    eval { require Math::BigInt::$backend; };
    if (\$@) {
        plan skip_all => "Math::BigInt::$backend not installed";
    } else {
        plan tests => "1";
    }
}

use $module only => "$backend";

my \$x = 1;
is(\$x -> config("lib"), "Math::BigInt::$backend",
   "backend is Math::BigInt::$backend");
EOF
        echo "Wrote '$file'"
    done
done
