#!/usr/bin/perl -w

# Wherein we ensure that postamble works ok.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Config;
use Test::More tests => 8;
use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;
use ExtUtils::MakeMaker;
use TieOut;

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

use File::Temp qw[tempdir];
my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;
$| = 1;

my $Makefile = makefile_name;

ok( setup_recurs(), 'setup' );
END {
    ok( chdir File::Spec->updir );
    ok( teardown_recurs(), 'teardown' );
}

ok( chdir 'Big-Dummy', q{chdir'd to Big-Dummy} ) ||
        diag("chdir failed: $!");

{
    my $warnings = '';
    local $SIG{__WARN__} = sub {
        if ( $Config{usecrosscompile} ) {
            # libraries might not be present on the target system
            # when cross-compiling
            return if $_[0] =~ /\A\QWarning (mostly harmless): No library found for \E.+/
        }
        $warnings = join '', @_;
    };

    my $stdout = tie *STDOUT, 'TieOut' or die;
    my $mm = WriteMakefile(
                           NAME            => 'Big::Dummy',
                           VERSION_FROM    => 'lib/Big/Dummy.pm',
                           postamble       => {
                                               FOO => 1,
                                               BAR => "fugawazads"
                                              }
                          );
    is( $warnings, '', 'postamble argument not warned about' );
}

sub MY::postamble {
    my($self, %extra) = @_;

    is_deeply( \%extra, { FOO => 1, BAR => 'fugawazads' },
               'postamble args passed' );

    return <<OUT;
# This makes sure the postamble gets written
OUT

}


ok( open(MAKEFILE, $Makefile) ) or diag "Can't open $Makefile: $!";
{ local $/;
  like( <MAKEFILE>, qr/^\# This makes sure the postamble gets written\n/m,
        'postamble added to the Makefile' );
}
close MAKEFILE;
