use 5.008001;
use strict;
use warnings;

package SubtestCompat;

# XXX must be used with no_plan or done_testing
use Test::More 0.88;

use base 'Exporter';
our @EXPORT;

our $INDENT = -2;

# intercept 'skip_all' in subtest and turn into a regular skip
sub _fake_plan {
    my ( $self, $cmd, $arg ) = @_;

    return unless $cmd;

    if ( $cmd eq 'skip_all' ) {
        die bless { reason => $arg }, "Subtest::SKIP";
    }
    else {
        goto &Test::Builder::plan;
    }
}

unless ( Test::More->can("subtest") ) {
    *subtest = sub {
        my ( $label, $code ) = @_;
        local $Test::Builder::Level = $Test::Builder::Level + 1;

        local $INDENT = $INDENT + 2;

        $label = "TEST: $label";
        my $sep_len = 60 - length($label);

        note( " " x $INDENT . "$label { " . ( " " x ($sep_len-$INDENT-2) ) );
        eval {
            no warnings 'redefine';
            local *Test::Builder::plan = \&_fake_plan;
            # only want subtest error reporting to look up to the code ref
            # for where test was called, not further up to *our* callers,
            # so we *reset* the Level, rather than increment it
            local $Test::Builder::Level = 1;
            $code->();
        };
        if ( my $err = $@ ) {
            if ( ref($err) eq 'Subtest::SKIP' ) {
                SKIP: {
                    skip $err->{reason}, 1;
                }
            }
            else {
                fail("SUBTEST: $label");
                diag("Caught exception: $err");
                die "$err\n";
            }
        }
        note( " " x $INDENT . "}" );
    };
    push @EXPORT, 'subtest';
}

1;
