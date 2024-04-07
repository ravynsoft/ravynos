#ifndef isSLASH
#define isSLASH(c) ((c) == '/' || (c) == '\\')
#define SKIP_SLASHES(s) \
    STMT_START {				\
	while (*(s) && isSLASH(*(s)))		\
	    ++(s);				\
    } STMT_END
#define COPY_NONSLASHES(d,s) \
    STMT_START {				\
	while (*(s) && !isSLASH(*(s)))		\
	    *(d)++ = *(s)++;			\
    } STMT_END
#endif

/* Find the longname of a given path.  path is destructively modified.
 * It should have space for at least MAX_PATH characters. */

CHAR_T *
LONGPATH(CHAR_T *path)
{
    WIN32_FIND_DATA_T fdata;
    HANDLE fhand;
    CHAR_T tmpbuf[MAX_PATH+1];
    CHAR_T *tmpstart = tmpbuf;
    CHAR_T *start = path;
    CHAR_T sep;
    if (!path)
	return NULL;

    /* drive prefix */
    if (isALPHA(path[0]) && path[1] == ':') {
	start = path + 2;
	*tmpstart++ = toupper(path[0]);
	*tmpstart++ = ':';
    }
    /* UNC prefix */
    else if (isSLASH(path[0]) && isSLASH(path[1])) {
	start = path + 2;
	*tmpstart++ = path[0];
	*tmpstart++ = path[1];
	SKIP_SLASHES(start);
	COPY_NONSLASHES(tmpstart,start);	/* copy machine name */
	if (*start) {
	    *tmpstart++ = *start++;
	    SKIP_SLASHES(start);
	    COPY_NONSLASHES(tmpstart,start);	/* copy share name */
	}
    }
    *tmpstart = '\0';
    while (*start) {
	/* copy initial slash, if any */
	if (isSLASH(*start)) {
	    *tmpstart++ = *start++;
	    *tmpstart = '\0';
	    SKIP_SLASHES(start);
	}

	/* FindFirstFile() expands "." and "..", so we need to pass
	 * those through unmolested */
	if (*start == '.'
	    && (!start[1] || isSLASH(start[1])
		|| (start[1] == '.' && (!start[2] || isSLASH(start[2])))))
	{
	    COPY_NONSLASHES(tmpstart,start);	/* copy "." or ".." */
	    *tmpstart = '\0';
	    continue;
	}

	/* if this is the end, bust outta here */
	if (!*start)
	    break;

	/* now we're at a non-slash; walk up to next slash */
	while (*start && !isSLASH(*start))
	    ++start;

	/* stop and find full name of component */
	sep = *start;
	*start = '\0';
	fhand = FN_FINDFIRSTFILE(path,&fdata);
	*start = sep;
	if (fhand != INVALID_HANDLE_VALUE) {
	    STRLEN len = FN_STRLEN(fdata.cFileName);
	    if (len < (STRLEN)(tmpbuf - tmpstart + sizeof(tmpbuf))) {
		FN_STRCPY(tmpstart, fdata.cFileName);
		tmpstart += len;
		FindClose(fhand);
	    }
	    else {
		FindClose(fhand);
		errno = ERANGE;
		return NULL;
	    }
	}
	else {
	    /* failed a step, just return without side effects */
	    /*PerlIO_printf(Perl_debug_log, "Failed to find %s\n", path);*/
	    errno = EINVAL;
	    return NULL;
	}
    }
    FN_STRCPY(path,tmpbuf);
    return path;
}

#undef CHAR_T
#undef WIN32_FIND_DATA_T
#undef FN_FINDFIRSTFILE
#undef FN_STRLEN
#undef FN_STRCPY
#undef LONGPATH
