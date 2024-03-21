  .csect .text[PR]

  # .function without 5th argument means
  # that the size is the size of the csect.
  .globl .foo
.foo:
  .function .foo,.foo,2,0
  blr

  # .function without 5th argument means
  # that the size is the size given.
  .globl .bar
.bar:
  .function .bar,.bar,2,0, E..bar-.bar
  blr
E..bar:
