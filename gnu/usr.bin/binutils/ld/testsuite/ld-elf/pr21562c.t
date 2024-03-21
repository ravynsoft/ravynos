SECTIONS {
    .foo : {
	PROVIDE (__start_scnfoo = .);
	PROVIDE (___start_scnfoo = .);
	*(scnfoo)
	PROVIDE (__stop_scnfoo = .);
	PROVIDE (___stop_scnfoo = .);
    }
}
