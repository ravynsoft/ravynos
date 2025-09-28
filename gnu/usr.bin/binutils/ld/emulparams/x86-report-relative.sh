PARSE_AND_LIST_OPTIONS_X86_REPORT_RELATIVE='
  fprintf (file, _("\
  -z report-relative-reloc    Report relative relocations\n"));
'
PARSE_AND_LIST_ARGS_CASE_Z_X86_REPORT_RELATIVE='
      else if (strcmp (optarg, "report-relative-reloc") == 0)
	params.report_relative_reloc = 1;
'

PARSE_AND_LIST_OPTIONS="$PARSE_AND_LIST_OPTIONS $PARSE_AND_LIST_OPTIONS_X86_REPORT_RELATIVE"
PARSE_AND_LIST_ARGS_CASE_Z="$PARSE_AND_LIST_ARGS_CASE_Z $PARSE_AND_LIST_ARGS_CASE_Z_X86_REPORT_RELATIVE"
