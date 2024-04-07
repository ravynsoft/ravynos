# Module makefile for Thread::Semaphore (using ExtUtils::MakeMaker)

require 5.008;

use strict;
use warnings;

use ExtUtils::MakeMaker;

# Construct make file
WriteMakefile(
    'NAME'          => 'Thread::Semaphore',
    'AUTHOR'        => 'Jerry D. Hedden <jdhedden AT cpan DOT org>',
    'VERSION_FROM'  => 'lib/Thread/Semaphore.pm',
    'ABSTRACT_FROM' => 'lib/Thread/Semaphore.pm',
    'PREREQ_PM'     => {
        'threads::shared' => 0,
        'Scalar::Util'    => 1.10,
        'Test::More'      => 0.50,
    },
    'INSTALLDIRS'   => (($] < 5.011) ? 'perl' : 'site'),

    ((ExtUtils::MakeMaker->VERSION() lt '6.25') ?
        ('PL_FILES' => { })            : ()),
    ((ExtUtils::MakeMaker->VERSION() gt '6.30') ?
        ('LICENSE'  => 'perl_5')       : ()),
);

# Additional 'make' targets
sub MY::postamble
{
    return <<'_EXTRAS_';
fixfiles:
	@dos2unix `cat MANIFEST`
	@$(CHMOD) 644 `cat MANIFEST`
	@$(CHMOD) 755 examples/*.pl
_EXTRAS_
}

# EOF
