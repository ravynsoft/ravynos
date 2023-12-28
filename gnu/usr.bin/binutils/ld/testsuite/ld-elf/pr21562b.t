SECTIONS {
	PROVIDE (__start_scnfoo = .);
	scnfoo : { *(scnfoo) }
	PROVIDE (__stop_scnfoo = .);
}
