# AtheOS hints file ( http://www.atheos.cx/ )
# Kurt Skauen, kurt@atheos.cx 
 
prefix="/usr/perl5"

libpth='/system/libs /usr/lib'
usrinc='/include'

libs=' '

d_htonl='define'
d_htons='define'
d_ntohl='define'
d_ntohs='define'

d_locconv='undef'

# POSIX and BSD functions are scattered over several non-standard libraries
# in AtheOS, so I figured it would be safer to let the linker figure out
# which symbols are available.

usenm='false'

# Hopefully, the native malloc knows better than perl's.
usemymalloc='n'

# AtheOS native FS does not support hard-links, but link() is defined
# (for other FS's).

d_link='undef'
dont_use_nlink='define'

ld='gcc'
cc='gcc'

