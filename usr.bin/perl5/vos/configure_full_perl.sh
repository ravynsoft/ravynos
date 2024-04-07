# Configure environment variables for perl build.
declare -x AWK="/system/gnu_library/bin/gawk.pm"
export AWK
declare -x CC="/system/gnu_library/bin/gcc.pm"
export CC
declare -x CPP="$CC $CPPFLAGS -E"
export CPP
declare -x CPPFLAGS="-D_POSIX_SOURCE=199506L -D_SYSV"
export CPPFLAGS
declare -x CONFIG_SHELL="bash"
export CONFIG_SHELL
declare -x LD="/system/gnu_library/bin/gcc.pm"
export LD
declare -x MAKE="/system/gnu_library/bin/gmake.pm"
export MAKE
declare -x RANLIB=":"
export RANLIB
declare -x SHELL="/system/gnu_library/bin/bash.pm"
export SHELL
#
bash Configure -des
