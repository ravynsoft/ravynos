#source: start1.s
#source: dso-2.s
#source: dso-1.s
#as: --pic --no-underscore --em=criself
#ld: -m crislinux --as-needed tmpdir/libdso-1.so
#objdump: -p

# Using --as-needed caused a elf_hash_table (info)->dynobj to be
# registered before cris_elf_check_relocs was called, thereby
# voiding an assumption that it was the sole setter of
# htab->dynobj, trigging a SEGV due to a NULL dereference for
# the variable holding the .got section.
# The test-case would FAIL for the SEGV and we also check that
# we don't get the DT_NEEDED tag (indeed no dynamic things at
# all) because the library isn't needed and would have to move
# to the end of the link-line to have effect if actually needed.

.*:     file format elf32-cris

Program Header:
    LOAD off    .*
         filesz .*
    LOAD off    .*
         filesz .*
private flags = 0:
