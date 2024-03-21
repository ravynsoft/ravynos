SECTIONS {
    .text : {
	foo_a = bar_a;
	foo_t = bar_t;
	foo_o = bar_o;
	*(.text) 
    }
    .ARM.attribues 0 : { *(.ARM.attributes) }
}
