#as: -no-predefined-syms -x -I$srcdir/$subdir
#ld: -m mmo --defsym __.MMIX.start..text=0x80000
#objdump: -dr

# PUSHJ reloc handling was wrong for weak undefined symbols, causing
# internal inconsistencies, leading to a call to abort.
# Note that we don't keep track of which symbols have pushj-stubs, so
# we get one stub each for the two calls to "foo".

.*:     file format mmo

Disassembly of section \.text:

0+80000 <(_start|Main)>:
   80000:	fe000004 	get \$0,rJ
   80004:	f2010004 	pushj \$1,80014 <Main\+0x14>
   80008:	f2010004 	pushj \$1,80018 <Main\+0x18>
   8000c:	f6040000 	put rJ,\$0
   80010:	f8010000 	pop 1,0
   80014:	f1fdfffb 	jmp 0 <Main-0x80000>
   80018:	f1fdfffa 	jmp 0 <Main-0x80000>
