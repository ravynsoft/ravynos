PARSE_AND_LIST_OPTIONS_LAM='
  fprintf (file, _("\
  -z lam-u48                  Generate GNU_PROPERTY_X86_FEATURE_1_LAM_U48\n"));
  fprintf (file, _("\
  -z lam-u48-report=[none|warning|error] (default: none)\n\
                              Report missing LAM_U48 property\n"));
  fprintf (file, _("\
  -z lam-u57                  Generate GNU_PROPERTY_X86_FEATURE_1_LAM_U57\n"));
  fprintf (file, _("\
  -z lam-u57-report=[none|warning|error] (default: none)\n\
                              Report missing LAM_U57 property\n"));
  fprintf (file, _("\
  -z lam-report=[none|warning|error] (default: none)\n\
                              Report missing LAM_U48 and LAM_U57 properties\n"));
'
PARSE_AND_LIST_ARGS_CASE_Z_LAM='
      else if (strcmp (optarg, "lam-u48") == 0)
	params.lam_u48 = true;
      else if (strncmp (optarg, "lam-u48-report=", 15) == 0)
	{
	  if (strcmp (optarg + 15, "none") == 0)
	    params.lam_u48_report = prop_report_none;
	  else if (strcmp (optarg + 15, "warning") == 0)
	    params.lam_u48_report = prop_report_warning;
	  else if (strcmp (optarg + 15, "error") == 0)
	    params.lam_u48_report = prop_report_error;
	  else
	    einfo (_("%F%P: invalid option for -z lam-u48-report=: %s\n"),
		   optarg + 15);
	}
      else if (strcmp (optarg, "lam-u57") == 0)
	params.lam_u57 = true;
      else if (strncmp (optarg, "lam-u57-report=", 15) == 0)
	{
	  if (strcmp (optarg + 15, "none") == 0)
	    params.lam_u57_report = prop_report_none;
	  else if (strcmp (optarg + 15, "warning") == 0)
	    params.lam_u57_report = prop_report_warning;
	  else if (strcmp (optarg + 15, "error") == 0)
	    params.lam_u57_report = prop_report_error;
	  else
	    einfo (_("%F%P: invalid option for -z lam-u57-report=: %s\n"),
		   optarg + 15);
	}
      else if (strncmp (optarg, "lam-report=", 11) == 0)
	{
	  if (strcmp (optarg + 11, "none") == 0)
	    {
	      params.lam_u48_report = prop_report_none;
	      params.lam_u57_report = prop_report_none;
	    }
	  else if (strcmp (optarg + 11, "warning") == 0)
	    {
	      params.lam_u48_report = prop_report_warning;
	      params.lam_u57_report = prop_report_warning;
	    }
	  else if (strcmp (optarg + 11, "error") == 0)
	    {
	      params.lam_u48_report = prop_report_error;
	      params.lam_u57_report = prop_report_error;
	    }
	  else
	    einfo (_("%F%P: invalid option for -z lam-report=: %s\n"),
		   optarg + 11);
	}
'

PARSE_AND_LIST_OPTIONS="$PARSE_AND_LIST_OPTIONS $PARSE_AND_LIST_OPTIONS_LAM"
PARSE_AND_LIST_ARGS_CASE_Z="$PARSE_AND_LIST_ARGS_CASE_Z $PARSE_AND_LIST_ARGS_CASE_Z_LAM"
