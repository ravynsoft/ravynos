/* Distance between 'one' and 'two' means that relaxation implies that
   .text.one's size increases. Even though the result would fit in
   'oneandhalf', this is not supported by
   --enable-non-contiguous-regions.  */

MEMORY {
       one (RXAI) : ORIGIN = 0x00000000, LENGTH = 0x00000010
       oneandhalf (RXAI) : ORIGIN = 0x00001000, LENGTH = 0x00001010
       two (RXAI) : ORIGIN = 0x20000000, LENGTH = 0x10000000
}

SECTIONS {
      one : {
         *(.text.one)
      } > one
      oneandhalf : {
         *(.text.one)
      } > oneandhalf
      two : {
      *(.text.two)
      } > two
}
