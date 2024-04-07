require 5.006001;

use strict;
use warnings;
use ExtUtils::MakeMaker;

my $clean = {};

if (-f "Collate.xs") {
    print "Making header files for XS...\n";
    # Should $mkh be File::Spec->catfile(File::Spec->curdir(), 'mkheader') ?
    my $mkh = './mkheader';
    do $mkh or die $@ || "mkheader: $!";
    $clean = { FILES => 'ucatbl.h' };
}

WriteMakefile(
    'AUTHOR'            => 'SADAHIRO Tomoyuki <SADAHIRO@cpan.org>',
    'ABSTRACT'          => 'Unicode Collation Algorithm',
    'INSTALLDIRS'       => ($] >= 5.007002 && $] < 5.011) ? 'perl' : 'site',
                            # see perl5110delta, @INC reorganization
    'LICENSE'           => 'perl',
    'NAME'              => 'Unicode::Collate',
    'VERSION_FROM'      => 'Collate.pm', # finds $VERSION
    'clean'             => $clean,
    'PREREQ_PM'         => {
	'Carp'          => 0,
	'File::Spec'    => 0,
	'XSLoader'      => 0,
	'constant'      => 0,
	'strict'        => 0,
	'warnings'      => 0,
    },
);
