# Len Makin <len@hpc.CSIRO.AU>

# No dynamically loaded libraries
so='none'

case "$optimize" in
# No compile option -O
'') optimize='-h2' ;;
esac

# size_t is 32 bits. Next version of compiler will have -hsize_t64
# enabling size_t to be 64 bits.
# Current cc version 4.80 allows -hsubscript64 for 64 bit array subscripts.
ccflags="$ccflags -hxint -hmath vector -hsubscript64"

case "$usemymalloc" in
'') # The perl malloc.c SHOULD work
    usemymalloc='y'
    ;;
esac
