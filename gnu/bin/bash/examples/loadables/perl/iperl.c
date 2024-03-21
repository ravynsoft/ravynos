#include <EXTERN.h>               /* from the Perl distribution     */
#include <perl.h>                 /* from the Perl distribution     */

#define iperl my_perl		/* I guess the name `my_perl' is required */

extern void xs_init (pTHX);

static PerlInterpreter *iperl;  /***    The Perl interpreter    ***/

static int first = 1;

void
perl_close (void)
{
	PERL_SYS_TERM();
}

int
perl_main(int argc, char **argv, char **env)
{
	int	r;

	if (first) {
		first = 0;
		PERL_SYS_INIT3(&argc, &argv, &env);
	}
	iperl = perl_alloc();
	perl_construct(iperl);
	perl_parse(iperl, xs_init, argc, argv, (char **)NULL);
	r = perl_run(iperl);

PerlIO_flush(PerlIO_stdout());
PerlIO_flush(PerlIO_stderr());

	perl_destruct(iperl);
	perl_free(iperl);
	return (r);
}
