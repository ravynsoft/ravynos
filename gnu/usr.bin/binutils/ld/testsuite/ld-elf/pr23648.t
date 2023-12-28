MEMORY { code : ORIGIN = 0, LENGTH = 8M }

SECTIONS
{
  ENTRY (entry)
  .text 1M : { *(.text*) } >code
  /DISCARD/ : { *(*) }
}

foo = LENGTH (code) != 8M ? test0 : test1;
