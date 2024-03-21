Dynamic section at offset 0x[0-9a-f]+ contains \d+ entries:
\s+Tag\s+Type\s+Name/Value
\s*0x[0-9a-f]+ \(HASH\).*
\s*0x[0-9a-f]+ \(STRTAB\).*
\s*0x[0-9a-f]+ \(SYMTAB\).*
\s*0x[0-9a-f]+ \(STRSZ\).*
\s*0x[0-9a-f]+ \(SYMENT\).*
# Specifically want *not* to see here:
# (REL)
# (RELSZ)
# (RELENT)
# (TEXTREL)
#...
\s*0x[0-9a-f]+ \(NULL\).*

There are no relocations in this file\.
