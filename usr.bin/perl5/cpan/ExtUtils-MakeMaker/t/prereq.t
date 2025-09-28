#!/usr/bin/perl -w

# This is a test of the verification of the arguments to
# WriteMakefile.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Config;
use Test::More tests => 21;
use File::Temp qw[tempdir];

use TieOut;
use MakeMaker::Test::Utils;
use MakeMaker::Test::Setup::BFD;

use ExtUtils::MakeMaker;

chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

ok( setup_recurs(), 'setup' );
END {
    ok( chdir File::Spec->updir, 'chdir updir' );
    ok( teardown_recurs(), 'teardown' );
}

ok( chdir 'Big-Dummy', "chdir'd to Big-Dummy" ) ||
  diag("chdir failed: $!");

{
    ok my $stdout = tie(*STDOUT, 'TieOut'), 'tie STDOUT';
    my $warnings = '';
    local $SIG{__WARN__} = sub {
        if ( $Config{usecrosscompile} ) {
            # libraries might not be present on the target system
            # when cross-compiling
            return if $_[0] =~ /\A\QWarning (mostly harmless): No library found for \E.+/
        }
        $warnings .= join '', @_;
    };
    # prerequisite warnings are disabled while building the perl core:
    local $ExtUtils::MakeMaker::UNDER_CORE = 0;

    WriteMakefile(
        NAME            => 'Big::Dummy',
        PREREQ_PM       => {
            strict  => 0
        }
    );
    is $warnings, '', 'basic prereq';

    SKIP: {
	skip 'No CMR, no version ranges', 1
	    unless ExtUtils::MakeMaker::_has_cpan_meta_requirements;
	$warnings = '';
	WriteMakefile(
	    NAME            => 'Big::Dummy',
	    PREREQ_PM       => {
		strict  =>  '>= 0, <= 99999',
	    }
	);
	is $warnings, '', 'version range';
    }

    $warnings = '';
    WriteMakefile(
        NAME            => 'Big::Dummy',
        PREREQ_PM       => {
            strict  => 99999
        }
    );
    is $warnings,
    sprintf("Warning: prerequisite strict 99999 not found. We have %s.\n",
            $strict::VERSION), 'strict 99999';

    $warnings = '';
    WriteMakefile(
        NAME            => 'Big::Dummy',
        PREREQ_PM       => {
            "I::Do::Not::Exist" => 0,
        }
    );
    is $warnings,
    "Warning: prerequisite I::Do::Not::Exist 0 not found.\n", 'non-exist prereq';

    $warnings = '';
    WriteMakefile(
        NAME            => 'Big::Dummy',
        CONFIGURE_REQUIRES => {
            "I::Do::Not::Configure" => 0,
        }
    );
    is $warnings,
    "Warning: prerequisite I::Do::Not::Configure 0 not found.\n", 'non-exist prereq';

    $warnings = '';
    WriteMakefile(
        NAME            => 'Big::Dummy',
        TEST_REQUIRES => {
            "I::Do::Not::Test" => 0,
        }
    );
    is $warnings,
    "Warning: prerequisite I::Do::Not::Test 0 not found.\n", 'non-exist prereq';


    $warnings = '';
    WriteMakefile(
        NAME            => 'Big::Dummy',
        PREREQ_PM       => {
            "I::Do::Not::Exist" => "",
        }
    );
    my @warnings = split /\n/, $warnings;
    is @warnings, 2, '2 warnings';
    like $warnings[0], qr{^Undefined requirement for I::Do::Not::Exist\b}, 'undef version warning';
    is $warnings[1], "Warning: prerequisite I::Do::Not::Exist 0 not found.", 'not found warning';


    $warnings = '';
    WriteMakefile(
        NAME            => 'Big::Dummy',
        PREREQ_PM       => {
            "I::Do::Not::Exist" => 0,
            "strict"            => 99999,
        }
    );

    my $strict_warn
        = sprintf("Warning: prerequisite strict 99999 not found. We have %s.\n",
                                                            $strict::VERSION);
    # Done this way because EBCDIC sorts in a different order
    ok(   $warnings =~ s/Warning: prerequisite I::Do::Not::Exist 0 not found\.\n//
       && $warnings =~ s/\Q$strict_warn//
       && $warnings eq "", '2 bad prereq warnings');

    $warnings = '';
    eval {
        WriteMakefile(
            NAME            => 'Big::Dummy',
            PREREQ_PM       => {
                "I::Do::Not::Exist" => 0,
                "Nor::Do::I"        => 0,
                "strict"            => 99999,
            },
            PREREQ_FATAL    => 1,
        );
    };

    is $warnings, '', 'no warnings on PREREQ_FATAL';
    is $@, <<'END', "PREREQ_FATAL";
MakeMaker FATAL: prerequisites not found.
    I::Do::Not::Exist not installed
    Nor::Do::I not installed
    strict 99999

Please install these modules first and rerun 'perl Makefile.PL'.
END


    $warnings = '';
    eval {
        WriteMakefile(
            NAME            => 'Big::Dummy',
            PREREQ_PM       => {
                "I::Do::Not::Exist" => 0,
            },
            CONFIGURE => sub {
                require I::Do::Not::Exist;
            },
            PREREQ_FATAL    => 1,
        );
    };

    is $warnings, '', 'CONFIGURE sub non-exist req no warn';
    is $@, <<'END', "PREREQ_FATAL happens before CONFIGURE";
MakeMaker FATAL: prerequisites not found.
    I::Do::Not::Exist not installed

Please install these modules first and rerun 'perl Makefile.PL'.
END


    $warnings = '';
    @ARGV = 'PREREQ_FATAL=1';
    eval {
        WriteMakefile(
            NAME => 'Big::Dummy',
            PREREQ_PM => { "I::Do::Not::Exist" => 0, },
        );
    };
    is $warnings, "Warning: prerequisite I::Do::Not::Exist 0 not found.\n",
      'CLI PREREQ_FATAL warns';
    isnt $@, '', "CLI PREREQ_FATAL works";

}
