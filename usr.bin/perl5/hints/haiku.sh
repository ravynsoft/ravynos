# Haiku hints file
# $Id$

case "$prefix" in
'') prefix="$(finddir B_COMMON_DIRECTORY)" ;;
*) ;; # pass the user supplied value through
esac

libpth="$(finddir B_USER_DEVELOP_DIRECTORY)/lib $(finddir B_SYSTEM_DEVELOP_DIRECTORY)/lib $(finddir B_COMMON_DIRECTORY)/lib /system/lib"
usrinc="$(finddir B_SYSTEM_DEVELOP_DIRECTORY)/headers/posix"
locinc="$(finddir B_USER_CONFIG_DIRECTORY)/develop/headers $(finddir B_COMMON_DIRECTORY)/headers $(finddir B_SYSTEM_DEVELOP_DIRECTORY)/headers"

libc="$(finddir B_SYSTEM_LIB_DIRECTORY)/libroot.so"
libs='-lnetwork'

# Use Haiku's malloc() by default.
case "$usemymalloc" in
'') usemymalloc='n' ;;
esac

# Haiku generally supports hard links, but the default file system (BFS)
# doesn't. So better avoid using hard links.
d_link='undef'
dont_use_nlink='define'

# The array syserrlst[] is useless for the most part.
# Large negative numbers really kind of suck in arrays.
d_syserrlst='undef'

# Haiku uses gcc.
cc="gcc"
ld='gcc'

ccflags="$ccflags -fno-stack-protector"

# The runtime loader library path variable is LIBRARY_PATH.
case "$ldlibpthname" in
'') ldlibpthname=LIBRARY_PATH ;;
esac

# as of alpha 4.1 (at the latest) some symbols are versioned,
# confusing the nm lookup
case "$usenm" in
'') usenm='undef' ;;
esac

