#source: start.s
#ld: -m mmo -T$srcdir/$subdir/sec-11.ld
#error: contents at non-multiple-of-4 address

# A trivial check that we get a graceful error when trying to emit
# (loadable, addressable) contents at a misaligned address.  Note
# that e.g. debug sections do not have loadable contents.
