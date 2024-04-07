# Test problems in Makefile.PL's and hint files.

BEGIN {
    unshift @INC, 't/lib';
}
chdir 't';

use strict;
use warnings;
use Test::More tests => 5;
use ExtUtils::MM;
use MakeMaker::Test::Utils;
use File::Path;
use TieOut;

my $MM = bless { DIR => ['subdir'] }, 'MM';
my $DIRNAME = 'Problem-Module';
my %FILES = (
    'Makefile.PL'   => <<'END',
use ExtUtils::MakeMaker;
WriteMakefile(NAME    => 'Problem::Module');
END

    'subdir/Makefile.PL'    => <<'END',
printf "\@INC %s .\n", (grep { $_ eq '.' } @INC) ? "has" : "doesn't have";
warn "I think I'm going to be sick\n";
die "YYYAaaaakkk\n";
END

);

hash2files($DIRNAME, \%FILES);
END {
    ok( chdir File::Spec->updir, 'chdir ..' );
    ok( rmtree($DIRNAME), 'teardown' );
}

ok( chdir $DIRNAME, "chdir'd to Problem-Module" ) ||
  diag("chdir failed: $!");


# Make sure when Makefile.PL's break, they issue a warning.
# Also make sure Makefile.PL's in subdirs still have '.' in @INC.
{
    my $stdout = tie *STDOUT, 'TieOut' or die;

    my $warning = '';
    local $SIG{__WARN__} = sub { $warning = join '', @_ };
    eval { $MM->eval_in_subdirs; };

    is( $stdout->read, qq{\@INC has .\n}, 'cwd in @INC' );
    like( $@,
          qr{^ERROR from evaluation of .*subdir.*Makefile.PL: YYYAaaaakkk},
          'Makefile.PL death in subdir warns' );

    untie *STDOUT;
}
