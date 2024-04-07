/* Beginning of modification history */
/* Written 02-04-10 by Paul Green (Paul.Green@stratus.com) */
/* End of modification history */

/* This test case is extracted from Perl version 5.7.3.  It is
   in the Perl_unpack_str function of the pp_pack.c source file.

   GCC 2.95.2 improperly assumes that it can compensate for an
   extra fsub by performing a fadd.  This would work in
   fixed-point arithmetic, but does not work in floating-point
   arithmetic.

   This problem has been seen on HP-UX and on Stratus VOS, both
   of which have an HP PA-RISC target (hppa1.1).  The Stratus
   bug number is gnu_g++-220.  */

/* #define _POSIX_C_SOURCE 199506L -- added by Configure */
#include <stdio.h>
#include <string.h>
#include <math.h>

void test(double *result)
{
        float	afloat;
        double	adouble;
        int		checksum = 0;
        unsigned	cuv = 0;
        double	cdouble = 0.0;
        const int	bits_in_uv = 8 * sizeof(cuv);

        checksum = 53;
        cdouble = -1.0;

        if (checksum) {
                if (checksum > bits_in_uv) {
                        double trouble;

                        adouble = (double) (1 << (checksum & 15));

                        while (checksum >= 16) {
                                checksum -= 16;
                                adouble *= 65536.0;
                        }

                        /* At -O1, GCC 2.95.2 compiles the following loop
                           into:

                           L$0014
                                fcmp,dbl,>= %fr4,%fr0
                                ftest
                                b L$0014
                                fadd,dbl %fr4,%fr12,%fr4
                                fsub,dbl %fr4,%fr12,%fr4

                                This code depends on the floating-add and
                                floating-subtract retaining all of the
                                precision present in the operands.  There is
                                no such guarantee when using floating-point,
                                as this test case demonstrates.

                                The code is okay at -O0.  */

                        while (cdouble < 0.0)
                                cdouble += adouble;

                        cdouble = modf (cdouble / adouble, &trouble) * adouble;
                }
        }

        *result = cdouble;
}

int main (int argc, char ** argv)
{
double	value;

        test (&value);

        if (argc == 2 && !strcmp(argv[1],"-v"))
                printf ("value = %.18e\n", value);

        if (value != 9.007199254740991e+15) {
                printf ("t001 fails!\n");
                return -1;
        }
        else {
                printf ("t001 works.\n");
                return 0;
        }
}
