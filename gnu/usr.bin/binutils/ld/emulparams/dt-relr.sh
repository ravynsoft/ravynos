HAVE_DT_RELR=yes
PARSE_AND_LIST_OPTIONS_PACK_RELATIVE_RELOCS='
  fprintf (file, _("\
  -z pack-relative-relocs     Pack relative relocations\n"));
  fprintf (file, _("\
  -z nopack-relative-relocs   Do not pack relative relocations (default)\n"));
'

PARSE_AND_LIST_ARGS_CASE_Z_PACK_RELATIVE_RELOCS='
      else if (strcmp (optarg, "pack-relative-relocs") == 0)
	link_info.enable_dt_relr = true;
      else if (strcmp (optarg, "nopack-relative-relocs") == 0)
	link_info.enable_dt_relr = false;
'


PARSE_AND_LIST_OPTIONS="$PARSE_AND_LIST_OPTIONS $PARSE_AND_LIST_OPTIONS_PACK_RELATIVE_RELOCS"
PARSE_AND_LIST_ARGS_CASE_Z="$PARSE_AND_LIST_ARGS_CASE_Z $PARSE_AND_LIST_ARGS_CASE_Z_PACK_RELATIVE_RELOCS"
