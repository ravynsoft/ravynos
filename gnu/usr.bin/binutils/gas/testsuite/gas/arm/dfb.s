// Test file for ARMv8-R dfb.
.arch armv8-r
.syntax unified

f_a32:
  dfb

.thumb
f_t32:
  dfb
  it ne
  dfbne
  it eq
  dfbeq
