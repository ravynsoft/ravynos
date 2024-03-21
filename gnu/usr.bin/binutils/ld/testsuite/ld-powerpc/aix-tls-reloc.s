# An external tdata symbol
  .globl tdata_ext[TL]
  .csect tdata_ext[TL]
  .long 1

  .csect tdata_int_csect[TL]
# A first internal tdata symbol
tdata_int1:
  .long 2
# A second internal tdata symbol
tdata_int2:
  .long 3

# Two external tbss symbols.
# XCOFF doesn't seem to allow internal tbss
# (or bss) symbols.
  .comm tbss_ext[UL],8

  .toc
# TC entries targeting the external tdata symbol
# Their value should be "tdata_ext" address,
# except TLSM value which must be 0.
# Their relocations should target it.
  .tc  tdata_ext_gd[TC],tdata_ext[TL]
  .tc .tdata_ext_gd[TC],tdata_ext[TL]@m
  .tc  tdata_ext_ld[TC],tdata_ext[TL]@ld
  .tc  tdata_ext_ie[TC],tdata_ext[TL]@ie
  .tc  tdata_ext_le[TC],tdata_ext[TL]@le

# TC entries targeting internal tdata symbols.
# Their value should be "tdata_int1" or "tdata_int2"
# addresses, except TLSM value which must be 0.
# Their relocations should target "tdata_int_csect".
  .tc  tdata_int1_gd[TC],tdata_int1
  .tc .tdata_int1_gd[TC],tdata_int1@m
  .tc  tdata_int1_ld[TC],tdata_int1@ld
  .tc  tdata_int1_ie[TC],tdata_int1@ie
  .tc  tdata_int1_le[TC],tdata_int1@le
  .tc  tdata_int2_gd[TC],tdata_int2
  .tc .tdata_int2_gd[TC],tdata_int2@m
  .tc  tdata_int2_ld[TC],tdata_int2@ld
  .tc  tdata_int2_ie[TC],tdata_int2@ie
  .tc  tdata_int2_le[TC],tdata_int2@le

# TC entries targeting the external tdata symbol
# Their value should be "tbss_ext" address,
# except TLSM value which must be 0.
# Their relocations should target "tbss_ext".
  .tc  tbss_ext_gd[TC],tbss_ext[UL]
  .tc .tbss_ext_gd[TC],tbss_ext[UL]@m
  .tc  tbss_ext_ld[TC],tbss_ext[UL]@ld
  .tc  tbss_ext_ie[TC],tbss_ext[UL]@ie
  .tc  tbss_ext_le[TC],tbss_ext[UL]@le

# Module entry
  .tc  mh[TC],mh[TC]@ml
  .rename mh[TC], "_$TLSML" # Symbol for the module handle

# Macros
  .macro gd_macro,sym
  .if size == 32
  lwz 4, \sym\()[TC](2)
  lwz 3, .\sym\()[TC](2)
  .else
  ld 4, \sym\()[TC](2)
  ld 3, .\sym\()[TC](2)
  .endif
  bla __tls_get_addr
  .endm

  .macro ld_macro, sym
  .if size == 32
  lwz 4, \sym\()[TC](2)
  .else
  ld 4, \sym\()[TC](2)
  .endif
  add 5,3,4
  .endm

  .macro ie_macro, sym
  .if size == 32
  bla __get_tpointer
  lwz 4, \sym\()[TC](2)
  add 5,3,4
  .else
  ld 4, \sym\()[TC](2)
  add 5,4,13
  .endif
  .endm

  .macro le_macro, sym
  .if size == 32
  bla __get_tpointer
  lwz 4, \sym\()[TC](2)
  add 5,3,4
  .else
  ld 4, \sym\()[TC](2)
  add 5,3,13
  .endif
  .endm


# As TLS relocations are made in for TC symbols,
# this function only aims to avoid garbage collection
# of these symbols, especially hidden ones.
  .globl foo
  .globl .foo
  .csect foo[DS],3
foo:
  .if size == 32
  .long	.foo, TOC[tc0], 0
  .else
  .llong .foo, TOC[tc0], 0
  .endif

  .csect foo[PR]
.foo:
  # External syms
  #GD
  gd_macro tdata_ext_gd
  gd_macro tdata_int1_gd
  gd_macro tdata_int2_gd
  gd_macro tbss_ext_gd

  #LD
  .if size == 32
  lwz 3, mh[TC](2)
  .else
  ld 3, mh[TC](2)
  .endif
  bla __tls_get_mod
  ld_macro tdata_ext_ld
  ld_macro tdata_int1_ld
  ld_macro tdata_int2_ld
  ld_macro tbss_ext_ld

  #IE
  ie_macro tdata_ext_ie
  ie_macro tdata_int1_ie
  ie_macro tdata_int2_ie
  ie_macro tbss_ext_ie

  #LE
  le_macro tdata_ext_le
  le_macro tdata_int1_le
  le_macro tdata_int2_le
  le_macro tbss_ext_le
