#ld: -shared --out-implib dx.dll.a --gc-sections
#objdump: --syms
#xfail: mcore-*
#
# The MCORE-PE target does not support -shared.
#
# Check that the target specific entry symbol *Startup is still
# a defined (sec > 0), public (scl == 2) symbol, even after garbage
# collection.

#...
.*\(sec  1\)\(fl 0x00\)\(ty    0\)\(scl   2\) \(nx 0\) 0x0+000 .*Startup.*
#pass
