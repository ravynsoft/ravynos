#source: gotrel1.s
#as: --pic --underscore --em=criself
#ld: -m crislinux -shared
#objdump: -dr
#error: \A[^\nc][^\n]*o: uses _-prefixed [^\nc]*\n[^\nc][^\n]* failed to merge [^\n]*\n[^\nc][^\n]* no GOT [^\n]*\n[^\nc][^\n]* bad value\Z

# The error regex above is supposed to not match if we get a
# SEGV, in which case we'll see "child killed: segmentation
# violation", supposedly at the beginning (seen) or end (in
# theory) of it.  The input ELF type (with underscores on
# symbols) mismatches the output type (no underscores).
