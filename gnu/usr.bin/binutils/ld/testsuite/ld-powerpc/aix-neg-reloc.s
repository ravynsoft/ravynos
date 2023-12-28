  .toc
  .csect .text[PR]
  .globl foo
  .globl .foo
  .csect foo[DS],3
foo:
  .if size == 32
  .long	.foo, TOC[tc0], 0
  .else
  .llong .foo, TOC[tc0], 0
  .endif

  .csect .text[PR]
.foo:
LFB..0:
  blr

  .csect _foo.ro_[RO],4
  .globl _GLOBAL__F_foo
_GLOBAL__F_foo:
  .if size == 32
  .vbyte 4,LFB..0-$
  .else
  .vbyte 8,LFB..0-$
  .endif

# Make sure that .ref is also enough to keep _GLOBAL__F_foo
# when exporting foo.
  .csect .text[PR]
  .ref _GLOBAL__F_foo
