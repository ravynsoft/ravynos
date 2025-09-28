#!/usr/bin/perl
use strict;
use warnings;
$| = 1;
use Test::More tests => 4;
use File::Spec;
use lib (-d 't' ? File::Spec->catdir(qw(t lib)) : 'lib');
use ExtUtils::ParseXS::Utilities qw(
    standard_XS_defs
);
use PrimitiveCapture;

my @statements = (
    '#ifndef PERL_UNUSED_VAR',
    '#ifndef PERL_ARGS_ASSERT_CROAK_XS_USAGE',
    '#ifdef newXS_flags',
);

my $stdout = PrimitiveCapture::capture_stdout(sub {
  standard_XS_defs();
});

foreach my $s (@statements) {
    like( $stdout, qr/$s/s, "Printed <$s>" );
}

pass("Passed all tests in $0");
