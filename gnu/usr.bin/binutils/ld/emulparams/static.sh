PARSE_AND_LIST_ARGS_CASES="$PARSE_AND_LIST_ARGS_CASES
    case OPTION_DYNAMIC_LINKER:
      params.has_dynamic_linker = true;
      return false;

    case OPTION_NON_SHARED:
      /* Check if -static is passed at command-line before all input
	 files.  */
      if (!lang_has_input_file)
	params.static_before_all_inputs = true;
      return false;
"
