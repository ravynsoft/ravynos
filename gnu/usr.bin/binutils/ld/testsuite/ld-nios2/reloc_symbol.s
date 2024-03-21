.text
.global _start
_start:

# byte aligned
.align 0
.byte byte_sym

# short aligned
.align 1
.short short_sym

# word aligned
.align 2
.long  long_sym

# now lets try some unaligned words and halfwords
.byte byte_sym
.2byte short_sym
.4byte  long_sym

#.align 2
#nop

