#as: --no-underscore --em=criself
#ld: -shared -m crislinux -z nocombreloc
#ld_after_inputfiles: tmpdir/libdso-1b.so
#warning: \A[^\n]*\.o, section `.text', to symbol `expfn@@TST2':[^\n]*recompile with -fPIC\Z
#readelf: -a

# Building a DSO with (unrecommended) non-pic pc-relative references
# to a versioned symbol in a library got caught by an assert in
# elf_cris_copy_indirect_symbol wherein the list of pc-relative
# references wasn't merged, but simply asserted to be NULL before
# copied to, on the merged-to (direct) symbol.  For versioned symbols,
# there was an "extra" copy made, to make a base-version symbol, where
# the copied-from pc-relative list was NULL but the copied-to symbol
# already had a list merged.

# The list was used to emit warning messages, but incorrectly held the
# relocation section for the reference, resulting in warnings being
# emitted for any section with a pc-relative relocation.

# The test checks that there's a warning message only for the
# read-only sections section (.text) (not the read-write sections),
# that the correct number of relocations is emitted and we also check
# for the TEXTREL dynamic marker.

#...
 0x00000016 \(TEXTREL\)[ 	]+0x0
#...
Relocation section '\.rela\.text' at offset .* contains 4 entries:
#...
Relocation section '\.rela\.data' at offset .* contains 8 entries:
#...
Relocation section '.rela.data2' at offset .* contains 16 entries:
#pass
