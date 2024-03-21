PARSE_AND_LIST_OPTIONS_NOEXTEN_PROTECTED_DATA='
  fprintf (file, _("\
  -z noextern-protected-data  Do not treat protected data symbol as external\n"));
  fprintf (file, _("\
  -z indirect-extern-access   Enable indirect external access\n"));
  fprintf (file, _("\
  -z noindirect-extern-access Disable indirect external access (default)\n"));
'

# Set link_info.indirect_extern_access to 2 to indicate that it is set
# by "-z indirect-extern-access".
PARSE_AND_LIST_ARGS_CASE_Z_NOEXTEN_PROTECTED_DATA='
      else if (strcmp (optarg, "noextern-protected-data") == 0)
	link_info.extern_protected_data = false;
      else if (strcmp (optarg, "indirect-extern-access") == 0)
	link_info.indirect_extern_access = 2;
      else if (strcmp (optarg, "noindirect-extern-access") == 0)
	link_info.indirect_extern_access = 0;
'


PARSE_AND_LIST_OPTIONS="$PARSE_AND_LIST_OPTIONS $PARSE_AND_LIST_OPTIONS_NOEXTEN_PROTECTED_DATA"
PARSE_AND_LIST_ARGS_CASE_Z="$PARSE_AND_LIST_ARGS_CASE_Z $PARSE_AND_LIST_ARGS_CASE_Z_NOEXTEN_PROTECTED_DATA"
