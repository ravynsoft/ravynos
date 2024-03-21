#source: dso-1.s
#source: dsov32-4.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#readelf: -d -r

# Checking that a bsr to a non-PLT-decorated nonvisible function
# doesn't make the DSO textrel.

Dynamic section at offset .* contains 6 entries:
  Tag[ 	]+Type[ 	]+Name/Value
 0x0+4 \(HASH\)[ 	]+0x94
 0x0+5 \(STRTAB\)[ 	]+0xdc
 0x0+6 \(SYMTAB\)[ 	]+0xac
 0x0+a \(STRSZ\)[ 	]+14 \(bytes\)
 0x0+b \(SYMENT\)[ 	]+16 \(bytes\)
 0x0+ \(NULL\)[ 	]+0x0

There are no relocations in this file.
