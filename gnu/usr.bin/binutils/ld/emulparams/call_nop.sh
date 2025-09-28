PARSE_AND_LIST_OPTIONS_CALL_NOP='
  fprintf (file, _("\
  -z call-nop=PADDING         Use PADDING as 1-byte NOP for branch\n"));
'
PARSE_AND_LIST_ARGS_CASE_Z_CALL_NOP='
      else if (strncmp (optarg, "call-nop=", 9) == 0)
	{
	  if (strcmp (optarg + 9, "prefix-addr") == 0)
	    {
	      params.call_nop_as_suffix = false;
	      params.call_nop_byte = 0x67;
	    }
	  else if (strcmp (optarg + 9, "suffix-nop") == 0)
	    {
	      params.call_nop_as_suffix = true;
	      params.call_nop_byte = 0x90;
	    }
	  else if (strncmp (optarg + 9, "prefix-", 7) == 0)
	    {
	      char *end;
	      params.call_nop_byte = strtoul (optarg + 16 , &end, 0);
	      if (*end)
		einfo (_("%F%P: invalid number for -z call-nop=prefix-: %s\n"),
		       optarg + 16);
	      params.call_nop_as_suffix = false;
	    }
	  else if (strncmp (optarg + 9, "suffix-", 7) == 0)
	    {
	      char *end;
	      params.call_nop_byte = strtoul (optarg + 16, &end, 0);
	      if (*end)
		einfo (_("%F%P: invalid number for -z call-nop=suffix-: %s\n"),
		       optarg + 16);
	      params.call_nop_as_suffix = true;
	    }
	  else
	    einfo (_("%F%P: unsupported option: -z %s\n"), optarg);
	}
'

PARSE_AND_LIST_OPTIONS="$PARSE_AND_LIST_OPTIONS $PARSE_AND_LIST_OPTIONS_CALL_NOP"
PARSE_AND_LIST_ARGS_CASE_Z="$PARSE_AND_LIST_ARGS_CASE_Z $PARSE_AND_LIST_ARGS_CASE_Z_CALL_NOP"
CALL_NOP_BYTE=0x67
