 .hidden export_1
 .include "dso-2.s"
dsofn:
 .type	dsofn,@function
 move.w expobj:GOT16,$r10
