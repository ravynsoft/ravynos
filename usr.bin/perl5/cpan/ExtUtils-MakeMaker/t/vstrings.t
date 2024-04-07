#!/usr/bin/perl -w

# test support for various forms of vstring versions in PREREQ_PM

# Magic for core
BEGIN {
    # Always run in t to unify behaviour with core
    chdir 't' if -d 't';
}

# Use things from t/lib/
use lib './lib';
use strict;
use warnings;
use TieOut;
use MakeMaker::Test::Utils qw(makefile_name);
use File::Temp qw[tempdir];
use Test::More;

use ExtUtils::MakeMaker;

my $tmpdir = tempdir( DIR => '.', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

my $UNDEFRE = qr/Undefined requirement .* treated as '0'/;
my $UNPARSABLERE = qr/Unparsable\s+version/;
# [ pkg, version, okwarningRE, descrip, nocmrRE ]
# only supply nocmrRE if want to treat differently when no CMR
my @DATA = (
  [ Undef => undef, $UNDEFRE, 'Undef' ],
  [ ZeroLength => '', $UNDEFRE, 'Zero-length' ],
  [ SemiColon => '0;', $UNPARSABLERE, 'Semi-colon after 0' ],
  [ BrokenString => 'nan', $UNPARSABLERE, 'random string' ],
  [ Decimal2 => 1.2, qr/^$/, '2-part Decimal' ],
  [ Decimal2String => '1.2', qr/^$/, '2-part Decimal String' ],
  [ Decimal2Underscore => '1.02_03', qr/^$/, '2-part Underscore String' ],
  [ Decimal3String => '1.2.3', qr/^$/, '3-part Decimal String', $UNPARSABLERE ],
  [ BareV2String => v1.2, qr/^$/, '2-part bare v-string', $UNPARSABLERE ],
  [ BareV3String => v1.2.3, qr/^$/, '3-part bare V-string', $UNPARSABLERE ],
  [ V2DecimalString => 'v1.2', qr/^$/, '2-part v-decimal string', $UNPARSABLERE ],
  [ V3DecimalString => 'v1.2.3', qr/^$/, '3-part v-Decimal String', $UNPARSABLERE ],
  [ RangeString => '>= 5.0, <= 6.0', qr/^$/, 'Version range', $UNPARSABLERE ],
  [ Scientific => 0.000005, qr/^$/, 'Scientific Notation' ],
);

plan tests => (1 + (@DATA * 4));

ok my $stdout = tie(*STDOUT, 'TieOut'), 'tie STDOUT';

# fake CMR to test fallback if CMR not present
my $CMR = 'CPAN/Meta/Requirements.pm';
my $CM = 'CPAN/Meta.pm';
$INC{$CMR} = undef;
$INC{$CM} = undef;
run_test(0, @$_) for @DATA;

# now try to load real CMR
delete $INC{$CMR};
delete $INC{$CM};
SKIP: {
  skip 'No actual CMR found', 2 * @DATA
    unless ExtUtils::MakeMaker::_has_cpan_meta_requirements;
  run_test(1, @$_) for @DATA;
}

sub capture_make {
    my ($package, $version) = @_ ;

    my $warnings = '';
    local $SIG{__WARN__} = sub {
        $warnings .= join '', @_;
    };

    local $ExtUtils::MakeMaker::UNDER_CORE = 0;

    WriteMakefile(
        NAME      => 'VString::Test',
        PREREQ_PM => { $package , $version }
    );

    return $warnings;
}

sub makefile_content {
    my $file = makefile_name;
    open my $fh, '<', $file or return "$file: $!\n";
    join q{}, grep { $_ =~ /Fake/i } <$fh>;
}

sub run_test {
  my ($gotrealcmr, $pkg, $version, $okwarningRE, $descrip, $nocmrRE) = @_;
  local $_;
  SKIP: {
    skip "No vstring test <5.8", 2
      if "$]" <= 5.008 && $pkg eq 'BareV2String' && $descrip =~ m!^2-part!;
    my $warnings;
    eval { $warnings = capture_make("Fake::$pkg" => $version); };
    is($@, '', "$descrip not fatal") or skip "$descrip WM failed", 1;
    $warnings =~ s#^Warning: prerequisite Fake::$pkg.* not found\.\n##m;
    my $re = (!$gotrealcmr && $nocmrRE) ? $nocmrRE : $okwarningRE;
    like $warnings, $re, "$descrip handled right";
  }
#  diag makefile_content();
}
