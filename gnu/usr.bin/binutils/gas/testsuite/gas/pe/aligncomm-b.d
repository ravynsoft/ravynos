#objdump: -s -j .drectve
#name: aligned common B

# Test the aligned form of the .comm pseudo-op.

.*: .*

Contents of section .drectve:
 0000 202d616c 69676e63 6f6d6d3a 225f6822   -aligncomm:"_h"
 0010 2c38202d 616c6967 6e636f6d 6d3a225f  ,8 -aligncomm:"_
 0020 69222c34 202d616c 69676e63 6f6d6d3a  i",4 -aligncomm:
 0030 225f6a22 2c32202d 616c6967 6e636f6d  "_j",2 -aligncom
 0040 6d3a225f 6b222c31 202d616c 69676e63  m:"_k",1 -alignc
 0050 6f6d6d3a 5f682c35 202d616c 69676e63  omm:_h,5 -alignc
 0060 6f6d6d3a 5f692c34 202d616c 69676e63  omm:_i,4 -alignc
 0070 6f6d6d3a 5f6a2c33 202d616c 69676e63  omm:_j,3 -alignc
 0080 6f6d6d3a 5f6b2c32 .*omm:_k,2.*
