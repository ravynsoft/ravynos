# Most targets will emit an R_*_RELATIVE reloc here, but RELATIVE
# relocs are superfluous.  A target can do without them by simply
# defining an ADDR32 or ADDR64 style reloc without a symbol to behave
# like a RELATIVE reloc.  GLOB_DAT relocs are similarly superfluous.
# In fact, a RELATIVE reloc can be wrong even if a target does have
# them, if the 32-bit or 64-bit field being relocated is unaligned.
# In that case the target ought to emit a UADDR32/64 or similar rather
# than a RELATIVE reloc.
#
# We also allow a dynamic reloc with a reference to .text as that
# should also resolve correctly.  No reloc, or one referencing "fun"
# is incorrect.  Also fail the test on finding a reloc at offset 0,
# typically a NONE reloc.

Relocation section.*
 *Offset.*
0*[1-9a-f][0-9a-f]* +[^ ]+ +[^ ]+ +([0-9a-f]+( +(\.text|fun)( \+ [0-9a-f]+)?)?)?
#pass
