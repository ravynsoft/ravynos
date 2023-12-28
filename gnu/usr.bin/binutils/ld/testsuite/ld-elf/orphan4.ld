/* The .foo section doesn't specify *any* objects, but the object
   we're linking has sections named ".foo".  Make sure these sections
   are linked into the .foo output section anyway.  The bug that was
   fixed was that a new .foo output section would be created at
   address 0.  */

SECTIONS {
	.foo  0x00001000 : {
	}
	.text 0x00002000 : {
		*(.text);
	}
}
