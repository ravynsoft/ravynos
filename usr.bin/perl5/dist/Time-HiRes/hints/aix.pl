# Many AIX installations seem not to have the right PATH
# for the C compiler.  Steal the logic from Perl's hints/aix.sh.
use Config;
unless ($Config{gccversion}) {
    my $cc = $Config{cc};
    if (! -x $cc && -x "/usr/vac/bin/$cc") {
	unless (":$ENV{PATH}:" =~ m{:/usr/vac/bin:}) {
	    die <<__EOE__;
***
*** You either implicitly or explicitly specified an IBM C compiler,
*** but you do not seem to have one in /usr/bin, but you seem to have
*** the VAC installed in /usr/vac, but you do not have the /usr/vac/bin
*** in your PATH.  I suggest adding that and retrying Makefile.PL.
***
__EOE__
	}
    }
}
