# AIX linker script.
# AIX always uses shared libraries.  The section VMA appears to be
# unimportant.  The native linker aligns the sections on boundaries
# specified by the -H option.
#
# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_ARCH(${ARCH})
${RELOCATING+${LIB_SEARCH_DIRS}}
${RELOCATING+ENTRY (__start)}
SECTIONS
{
  .pad 0 : { *(.pad) }

  . = ALIGN (0x10000000 + SIZEOF_HEADERS, 32);
  .text ${RELOCATING-0} : {
    ${RELOCATING+PROVIDE (_text = .);}
    *(.text)
    *(.pr)
    *(.ro)
    *(.db)
    *(.gl)
    *(.xo)
    *(.ti)
    *(.tb)
    ${RELOCATING+PROVIDE (_etext = .);}
  }

  /* .tdata and .tbss addresses are representing the offset from
     the TLS pointer. It starts at -0x7800 for 64bit and -0x7c00
     for 32bit.
     TODO: 32bit should have -0x7c00 but it works like this for
     now.
     The other particularity is that they must be before .data
     sections. But .data must be aligned correctly as if the
     addresses were contiguous. This means that the correct
     address must be restored, taking into account: the size of
     .text, its alignment 2^5, the size of .tdata and its
     aligment 2^4.  */
  .tdata -0x7800 : {
    *(.tdata)
    *(.tl)
  }

  .tbss : {
    *(.tbss)
    *(.ul)
  }

  . = ${RELOCATING+(ALIGN (0x10000000 + SIZEOF_HEADERS, 32)) + }SIZEOF(.text);
  . = ALIGN (.,32);
  . = . + SIZEOF(.tdata);
  . = ALIGN (.,16);

  /* .data starting address must be in a different segment than
     the .text addresses. Thus, 0x10000000 is added.  */
  . = ALIGN (0x10000000) + (. & 0xfff);
  .data . : {
    ${RELOCATING+PROVIDE (_data = .);}
    *(.data)
    *(.rw)
    *(.sv)
    *(.sv64)
    *(.sv3264)
    *(.ua)
    . = ALIGN(4);
    ${CONSTRUCTING+CONSTRUCTORS}
    *(.ds)
    *(.tc0)
    *(.tc)
    *(.td)
    *(.te)
    ${RELOCATING+PROVIDE (_edata = .);}
  }
  .bss : {
    *(.tocbss)
    *(.bss)
    *(.bs)
    *(.uc)
    *(COMMON)
    ${RELOCATING+PROVIDE (_end = .);}
    ${RELOCATING+PROVIDE (end = .);}
  }

  .loader : {
    *(.loader)
  }

  .debug : {
    *(.debug)
  }
}
EOF
