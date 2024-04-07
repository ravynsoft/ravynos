#!/perl -w
use 5.010;
use strict;
use Config;

# This test checks that anything with an executable bit is
# identified in Porting/exec-bit.txt to makerel will set
# the exe bit in the release tarball
# and that anything with an executable bit also has a shebang

sub has_shebang {
  my $fname = shift;
  open my $fh, '<', $fname or die "Can't open '$fname': $!";
  my $line = <$fh>;
  close $fh;

  return $line =~ /^\#!\s*([A-Za-z0-9_\-\/\.])+\s?/ ? 1 : 0;
}

require './test.pl';
if ( $^O eq "MSWin32" ) {
  skip_all( "-x on MSWin32 only indicates file has executable suffix. Try Cygwin?" );
}

if ( $^O eq "VMS" ) {
  skip_all( "Filename case may not be preserved and other porting issues." );
}

if ( $^O eq "vos" ) {
  skip_all( "VOS combines the read and execute permission bits." );
}

if ( $Config{usecrosscompile} ) {
  skip_all( "Not all files are available during cross-compilation" );
}

plan('no_plan');

use ExtUtils::Manifest qw(maniread);

# Copied from Porting/makerel - these will get +x in the tarball
# XXX refactor? -- dagolden, 2010-07-23
my %exe_list =
  map   { $_ => 1 }
  map   { my ($f) = split; glob("../$f") }
  grep  { $_ !~ /\A#/ && $_ !~ /\A\s*\z/ }
  map   { split "\n" }
  do    { local (@ARGV, $/) = '../Porting/exec-bit.txt'; <> };

# Get MANIFEST
$ExtUtils::Manifest::Quiet = 1;
my @manifest = sort keys %{ maniread("../MANIFEST") };

# Check that +x files in repo get +x from makerel
for my $f ( map { "../$_" } @manifest ) {
  next unless -x $f;

  ok( has_shebang($f), "File $f has shebang" );

  ok( $exe_list{$f}, "tarball will chmod +x $f" )
    or diag( "Remove the exec bit or add '$f' to Porting/exec-bit.txt" );

  delete $exe_list{$f}; # seen it
}

ok( ! %exe_list, "Everything in Porting/exec-bit.txt has +x in repo" )
  or diag( "Files missing exec bit:\n  " . join("\n  ", sort keys %exe_list) . "\n");
