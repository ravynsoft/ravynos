PARSE_AND_LIST_OPTIONS_DYNAMIC_UNDEFINED_WEAK='
  fprintf (file, _("\
  -z dynamic-undefined-weak   Make undefined weak symbols dynamic\n\
  -z nodynamic-undefined-weak Do not make undefined weak symbols dynamic\n"));
'

PARSE_AND_LIST_ARGS_CASE_Z_DYNAMIC_UNDEFINED_WEAK='
      else if (strcmp (optarg, "dynamic-undefined-weak") == 0)
	link_info.dynamic_undefined_weak = true;
      else if (strcmp (optarg, "nodynamic-undefined-weak") == 0)
	link_info.dynamic_undefined_weak = false;
'

PARSE_AND_LIST_OPTIONS="$PARSE_AND_LIST_OPTIONS $PARSE_AND_LIST_OPTIONS_DYNAMIC_UNDEFINED_WEAK"
PARSE_AND_LIST_ARGS_CASE_Z="$PARSE_AND_LIST_ARGS_CASE_Z $PARSE_AND_LIST_ARGS_CASE_Z_DYNAMIC_UNDEFINED_WEAK"
