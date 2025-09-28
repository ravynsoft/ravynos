
	/* Test the error messages that should be generated.  */
	.attach_to_group does.not.exist  /* This is OK, the group does not have to exist.  */
	.attach_to_group foo.group  /* Already attached.  */
	.attach_to_group  /* Missing group name.  */
