/*
 * Written by Alan Burlinson -- taken from his blog post
 * at <http://bleaklow.com/2005/09/09/dtrace_and_perl.html>.
 */

provider perl {
    probe sub__entry(const char *, const char *, int, const char *);
    probe sub__return(const char *, const char *, int, const char *);

    probe phase__change(const char *, const char *);

    probe op__entry(const char *);

    probe loading__file(const char *);
    probe loaded__file(const char *);
};

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
