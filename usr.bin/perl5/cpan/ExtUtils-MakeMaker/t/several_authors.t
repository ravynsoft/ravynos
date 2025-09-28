#!/usr/bin/perl -w

# This is a test checking various aspects of the optional argument
# MIN_PERL_VERSION to WriteMakefile.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use TieOut;
use MakeMaker::Test::Utils;
use Config;
use ExtUtils::MM;
use Test::More
    !MM->can_run(make()) && $ENV{PERL_CORE} && $Config{'usecrosscompile'}
    ? (skip_all => "cross-compiling and make not available")
    : (tests => 19);
use File::Path;
use File::Temp qw[tempdir];

use ExtUtils::MakeMaker;
my $CM = eval { require CPAN::Meta; };

my $DIRNAME = 'Multiple-Authors';
my %FILES = (
    'Makefile.PL'   => <<'END',
use ExtUtils::MakeMaker;
WriteMakefile(
    NAME             => 'Multiple::Authors',
    AUTHOR           => ['John Doe <jd@example.com>', 'Jane Doe <jd@example.com>'],
    VERSION_FROM     => 'lib/Multiple/Authors.pm',
    PREREQ_PM        => { strict => 0 },
);
END

    'lib/Multiple/Authors.pm'    => <<'END',
package Multiple::Authors;

$VERSION = 0.05;

=head1 NAME

Multiple::Authors - several authors

=cut

1;
END

);

# avoid environment variables interfering with our make runs
delete @ENV{qw(PERL_JSON_BACKEND CPAN_META_JSON_BACKEND PERL_YAML_BACKEND)} if $ENV{PERL_CORE};
delete @ENV{qw(LIB MAKEFLAGS PERL_CORE)};

my $perl     = which_perl();
my $make     = make_run();
my $makefile = makefile_name();

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

hash2files($DIRNAME, \%FILES);
END {
    ok( chdir(File::Spec->updir), 'leaving dir' );
    ok( rmtree($DIRNAME), 'teardown' );
}

ok( chdir $DIRNAME, "entering dir $DIRNAME" ) ||
    diag("chdir failed: $!");

note "argument verification"; {
    my $stdout = tie *STDOUT, 'TieOut';
    ok( $stdout, 'capturing stdout' );
    my $warnings = '';
    local $SIG{__WARN__} = sub {
        $warnings .= join '', @_;
    };

    eval {
        WriteMakefile(
            NAME             => 'Multiple::Authors',
            AUTHOR           => ['John Doe <jd@example.com>', 'Jane Doe <jd@example.com>'],
        );
    };
    is( $warnings, '', 'arrayref in AUTHOR does not trigger a warning' );
    is( $@, '',        '  nor a hard failure' );

}


note "argument verification via CONFIGURE"; {
    my $stdout = tie *STDOUT, 'TieOut';
    ok( $stdout, 'capturing stdout' );
    my $warnings = '';
    local $SIG{__WARN__} = sub {
        $warnings .= join '', @_;
    };

    eval {
        WriteMakefile(
            NAME             => 'Multiple::Authors',
            CONFIGURE => sub {
               return {AUTHOR => 'John Doe <jd@example.com>',};
            },
        );
    };
    is( $warnings, '', 'scalar in AUTHOR inside CONFIGURE does not trigger a warning' );
    is( $@, '',        '  nor a hard failure' );

}


note "generated files verification"; {
    unlink $makefile;
    my @mpl_out = run(qq{$perl Makefile.PL});
    END { unlink $makefile, makefile_backup() }

    cmp_ok( $?, '==', 0, 'Makefile.PL exiting normally' ) || diag(@mpl_out);
    ok( -e $makefile, 'Makefile present' );
}


note "ppd output"; {
    my $ppd_file = 'Multiple-Authors.ppd';
    my @make_out = run(qq{$make ppd});
    END { unlink $ppd_file }

    cmp_ok( $?, '==', 0,    'Make ppd exiting normally' ) || diag(@make_out);

    my $ppd_html = slurp($ppd_file);
    ok( defined($ppd_html), '  .ppd file present' );

    like( $ppd_html, qr{John Doe &lt;jd\@example.com&gt;, Jane Doe &lt;jd\@example.com&gt;},
                            '  .ppd file content good' );
}


note "META.yml output"; SKIP: {
    skip 'Failed to load CPAN::Meta', 5 unless $CM;
    my $distdir  = 'Multiple-Authors-0.05';
    $distdir =~ s{\.}{_}g if $Is_VMS;

    my $meta_yml = "$distdir/META.yml";
    my $meta_json = "$distdir/META.json";
    my @make_out    = run(qq{$make metafile});
    END { rmtree $distdir if defined $distdir }

    cmp_ok( $?, '==', 0, 'Make metafile exiting normally' ) || diag(@make_out);

    for my $case (
        ['META.yml', $meta_yml],
        ['META.json', $meta_json],
    ) {
        my ($label, $meta_name) = @$case;
        ok(
          my $obj = eval {
            CPAN::Meta->load_file($meta_name, {lazy_validation => 0})
          },
          "$label validates"
        );
        is_deeply( [ $obj->authors ],
          [
            q{John Doe <jd@example.com>},
            q{Jane Doe <jd@example.com>},
          ],
          "$label content good"
        );
    }
}
