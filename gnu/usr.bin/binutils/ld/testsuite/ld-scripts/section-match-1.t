SECTIONS
{
	/* .secA should collect all sections with a prefix of ".sec."
	   except for those with a prefix of ".sec..".  */
        .secA : { *(.sec.[^.]*) }

	/* .secB should be empty because .secA will have taken all of
	   the potential matches.  */	
        .secB : { *(.sec.[!.]*) }

	/* .secC should match any sections with a ".sec.." prefix.  */
        .secC : { *(.sec.*) }

	/* Ignore anything else.  */
        /DISCARD/ : { *(*) }
}
