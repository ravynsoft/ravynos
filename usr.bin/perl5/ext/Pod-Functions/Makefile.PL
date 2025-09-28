#!perl -w

use strict;
use ExtUtils::MakeMaker;
use File::Spec::Functions;

WriteMakefile(NAME => 'Pod::Functions',
	      VERSION_FROM => 'Functions_pm.PL',
	      LICENSE => 'perl',
	      PREREQ_PM => {},
	      ABSTRACT_FROM => 'Functions_pm.PL',
	      AUTHOR => 'Perl 5 Porters <perl5-porters@perl.org>',
	      INSTALLDIRS => 'perl',
	      PL_FILES => {}, # Stop EU::MM defaulting this to run our PL
	      PM => {'Functions.pm' => '$(INST_LIBDIR)/Functions.pm'},
	      clean => {FILES => 'Functions.pm'},
	     );

# There doesn't seem to be any way to get ExtUtils::MakeMaker to add a
# dependency on another file (or target), and as it's using :: rules, not :
# rules, then we can't simply add a one line dependency. So we need to provide
# the entire thing. Fortunately, the same code in MM_Unix.pm is actually used
# for all platforms, so this code below should also be portable:

sub MY::postamble {
    my $pf = catfile(updir, updir, 'pod', 'perlfunc.pod');
    return <<"EOT";
all :: Functions.pm
	\$(NOECHO) \$(NOOP)

Functions.pm :: Functions_pm.PL $pf
	\$(PERLRUN) Functions_pm.PL $pf
EOT
}
