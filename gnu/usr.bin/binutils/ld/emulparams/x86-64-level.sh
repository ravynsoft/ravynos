PARSE_AND_LIST_OPTIONS_X86_64_LEVEL='
  fprintf (file, _("\
  -z x86-64-{baseline|v[234]} Mark x86-64-{baseline|v[234]} ISA level as needed\n"));
'
PARSE_AND_LIST_ARGS_CASE_Z_X86_64_LEVEL='
      else if (strcmp (optarg, "x86-64-baseline") == 0)
	params.isa_level = 1;
      else if (strncmp (optarg, "x86-64-v", 8) == 0)
	{
	  char *end;
	  unsigned int level = strtoul (optarg + 8 , &end, 10);
	  if (*end != '\0' || level < 2 || level > 4)
	    einfo (_("%F%P: invalid x86-64 ISA level: %s\n"), optarg);
	  params.isa_level = level;
	}
'

PARSE_AND_LIST_OPTIONS="$PARSE_AND_LIST_OPTIONS $PARSE_AND_LIST_OPTIONS_X86_64_LEVEL"
PARSE_AND_LIST_ARGS_CASE_Z="$PARSE_AND_LIST_ARGS_CASE_Z $PARSE_AND_LIST_ARGS_CASE_Z_X86_64_LEVEL"
