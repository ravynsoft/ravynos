#source: dso-1.s
#source: dsov32-3.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux --version-script $srcdir/$subdir/hidedsofns2468 --hash-style=sysv
#readelf: -d -r

# Like libdso-13.d, but without -z nocombreloc and with a version
# script hiding the function called pcrel-without-plt.  There should
# be no warning, no relocations in the output and no TEXTREL marking.

Dynamic section at offset .* contains 9 entries:
  Tag        Type                         Name/Value
 0x00000004 \(HASH\) .*
 0x00000005 \(STRTAB\) .*
 0x00000006 \(SYMTAB\) .*
 0x0000000a \(STRSZ\) .*
 0x0000000b \(SYMENT\) .*
 0x6ffffffc \(VERDEF\) .*
 0x6ffffffd \(VERDEFNUM\) .*
 0x6ffffff0 \(VERSYM\) .*
 0x00000000 \(NULL\)                       0x0

There are no relocations in this file.
