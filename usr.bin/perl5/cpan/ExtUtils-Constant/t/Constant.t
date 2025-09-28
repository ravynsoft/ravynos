#!/usr/bin/perl -w

use Config;
unless ($Config{usedl}) {
    print "1..0 # no usedl, skipping\n";
    exit 0;
}

# use warnings;
use strict;
use ExtUtils::MakeMaker;
use ExtUtils::Constant qw (C_constant autoload);
use File::Spec;
use Cwd;

my $do_utf_tests = $] > 5.006;
my $better_than_56 = $] > 5.007;
# For debugging set this to 1.
my $keep_files = 0;
$| = 1;

# Because were are going to be changing directory before running Makefile.PL
# 5.005 doesn't have new enough File::Spec to have rel2abs. But actually we
# only need it when $^X isn't absolute, which is going to be 5.8.0 or later
# (where ExtUtils::Constant is in the core, and tests against the uninstalled
# perl)
my $perl = $] < 5.006 ? $^X : File::Spec->rel2abs($^X);
# ExtUtils::Constant::C_constant uses $^X inside a comment, and we want to
# compare output to ensure that it is the same. We were probably run as ./perl
# whereas we will run the child with the full path in $perl. So make $^X for
# us the same as our child will see.
$^X = $perl;
# 5.005 doesn't have rel2abs, but also doesn't need to load an uninstalled
# module from blib
@INC = map {File::Spec->rel2abs($_)} @INC if $] < 5.007 && $] >= 5.006;

my $make = $Config{make};
$make = $ENV{MAKE} if exists $ENV{MAKE};
if ($^O eq 'MSWin32' && $make eq 'nmake') { $make .= " -nologo"; }

# VMS may be using something other than MMS/MMK
my $mms_or_mmk = ($make =~ m/^MM(S|K)/i) ? 1 : 0;

# Renamed by make clean
my $makefile = ($mms_or_mmk ? 'descrip' : 'Makefile');
my $makefile_ext = ($mms_or_mmk ? '.mms' : '');
my $makefile_rename = $makefile . ($mms_or_mmk ? '.mms_old' : '.old');

my $output = "output";
my $package = "ExtTest";
my $dir = "ext-$$";
my $subdir = 0;
# The real test counter.
my $realtest = 1;

my $orig_cwd = cwd;
my $updir = File::Spec->updir;
die "Can't get current directory: $!" unless defined $orig_cwd;

print "# $dir being created...\n";
mkdir $dir, 0777 or die "mkdir: $!\n";

END {
  if (defined $orig_cwd and length $orig_cwd) {
    chdir $orig_cwd or die "Can't chdir back to '$orig_cwd': $!";
    use File::Path;
    print "# $dir being removed...\n";
    rmtree($dir) unless $keep_files;
  } else {
    # Can't get here.
    die "cwd at start was empty, but directory '$dir' was created" if $dir;
  }
}

chdir $dir or die $!;
push @INC, '../../lib', '../../../lib';

package TieOut;

sub TIEHANDLE {
    my $class = shift;
    bless(\( my $ref = ''), $class);
}

sub PRINT {
    my $self = shift;
    $$self .= join('', @_);
}

sub PRINTF {
    my $self = shift;
    $$self .= sprintf shift, @_;
}

sub read {
    my $self = shift;
    return substr($$self, 0, length($$self), '');
}

package main;

sub check_for_bonus_files {
  my $dir = shift;
  my %expect = map {($^O eq 'VMS' ? lc($_) : $_), 1} @_;

  my $fail;
  opendir DIR, $dir or die "opendir '$dir': $!";
  while (defined (my $entry = readdir DIR)) {
    $entry =~ s/(.*?)\.?$/\L$1/ if $^O eq 'VMS';
    next if $expect{$entry};

    # Normal relics
    next if $^O eq 'os390' && $entry =~ /\.dbg$/;

    print "# Extra file '$entry'\n";
    $fail = 1;
  }

  closedir DIR or warn "closedir '.': $!";
  if ($fail) {
    print "not ok $realtest\n";
  } else {
    print "ok $realtest\n";
  }
  $realtest++;
}

sub build_and_run {
  my ($tests, $expect, $files) = @_;
  my $core = $ENV{PERL_CORE} ? ' PERL_CORE=1' : '';
  my @perlout = `$perl Makefile.PL $core`;
  if ($?) {
    print "not ok $realtest # $perl Makefile.PL failed: $?\n";
    print "# $_" foreach @perlout;
    exit($?);
  } else {
    print "ok $realtest\n";
  }
  $realtest++;

  if (-f "$makefile$makefile_ext") {
    print "ok $realtest\n";
  } else {
    print "not ok $realtest\n";
  }
  $realtest++;

  my @makeout;

  if ($^O eq 'VMS') { $make .= ' all'; }

  # Sometimes it seems that timestamps can get confused

  # make failed: 256
  # Makefile out-of-date with respect to Makefile.PL
  # Cleaning current config before rebuilding Makefile...
  # make -f Makefile.old clean > /dev/null 2>&1 || /bin/sh -c true
  # ../../perl "-I../../../lib" "-I../../../lib" Makefile.PL "PERL_CORE=1"
  # Checking if your kit is complete...                         
  # Looks good
  # Writing Makefile for ExtTest
  # ==> Your Makefile has been rebuilt. <==
  # ==> Please rerun the make command.  <==
  # false

  my $timewarp = (-M "Makefile.PL") - (-M "$makefile$makefile_ext");
  # Convert from days to seconds
  $timewarp *= 86400;
  print "# Makefile.PL is $timewarp second(s) older than $makefile$makefile_ext\n";
  if ($timewarp < 0) {
      # Sleep for a while to catch up.
      $timewarp = -$timewarp;
      $timewarp+=2;
      $timewarp = 10 if $timewarp > 10;
      print "# Sleeping for $timewarp second(s) to try to resolve this\n";
      sleep $timewarp;
  }

  print "# make = '$make'\n";
  @makeout = `$make`;
  if ($?) {
    print "not ok $realtest # $make failed: $?\n";
    print "# $_" foreach @makeout;
    exit($?);
  } else {
    print "ok $realtest\n";
  }
  $realtest++;

  if ($^O eq 'VMS') { $make =~ s{ all}{}; }

  if ($Config{usedl}) {
    print "ok $realtest # This is dynamic linking, so no need to make perl\n";
  } else {
    my $makeperl = "$make perl";
    print "# make = '$makeperl'\n";
    @makeout = `$makeperl`;
    if ($?) {
      print "not ok $realtest # $makeperl failed: $?\n";
      print "# $_" foreach @makeout;
      exit($?);
    } else {
      print "ok $realtest\n";
    }
  }
  $realtest++;

  my $maketest = "$make test";
  print "# make = '$maketest'\n";

  @makeout = `$maketest`;

  if (open OUTPUT, "<$output") {
    local $/; # Slurp it - faster.
    print <OUTPUT>;
    close OUTPUT or print "# Close $output failed: $!\n";
  } else {
    # Harness will report missing test results at this point.
    print "# Open <$output failed: $!\n";
  }

  $realtest += $tests;
  if ($?) {
    print "not ok $realtest # $maketest failed: $?\n";
    print "# $_" foreach @makeout;
  } else {
    print "ok $realtest - maketest\n";
  }
  $realtest++;

  if (defined $expect) {
      # -x is busted on Win32 < 5.6.1, so we emulate it.
      my $regen;
      if( $^O eq 'MSWin32' && $] <= 5.006001 ) {
	  open(REGENTMP, ">regentmp") or die $!;
	  open(XS, "$package.xs")     or die $!;
	  my $saw_shebang;
	  while(<XS>) {
	      $saw_shebang++ if /^#!.*/i ;
	      print REGENTMP $_ if $saw_shebang;
	  }
	  close XS;  close REGENTMP;
	  $regen = `$perl regentmp`;
	  unlink 'regentmp';
      }
      else {
	  $regen = `$perl -x $package.xs`;
      }
      if ($?) {
	  print "not ok $realtest # $perl -x $package.xs failed: $?\n";
	  } else {
	      print "ok $realtest - regen\n";
	  }
      $realtest++;

      if ($expect eq $regen) {
	  print "ok $realtest - regen worked\n";
      } else {
	  print "not ok $realtest - regen worked\n";
	  # open FOO, ">expect"; print FOO $expect;
	  # open FOO, ">regen"; print FOO $regen; close FOO;
      }
      $realtest++;
  } else {
    for (0..1) {
      print "ok $realtest # skip no regen or expect for this set of tests\n";
      $realtest++;
    }
  }

  my $makeclean = "$make clean";
  print "# make = '$makeclean'\n";
  @makeout = `$makeclean`;
  if ($?) {
    print "not ok $realtest # $make failed: $?\n";
    print "# $_" foreach @makeout;
  } else {
    print "ok $realtest\n";
  }
  $realtest++;

  check_for_bonus_files ('.', @$files, $output, $makefile_rename, '.', '..');

  rename $makefile_rename, $makefile . $makefile_ext
    or die "Can't rename '$makefile_rename' to '$makefile$makefile_ext': $!";

  unlink $output or warn "Can't unlink '$output': $!";

  # Need to make distclean to remove ../../lib/ExtTest.pm
  my $makedistclean = "$make distclean";
  print "# make = '$makedistclean'\n";
  @makeout = `$makedistclean`;
  if ($?) {
    print "not ok $realtest # $make failed: $?\n";
    print "# $_" foreach @makeout;
  } else {
    print "ok $realtest\n";
  }
  $realtest++;

  check_for_bonus_files ('.', @$files, '.', '..');

  unless ($keep_files) {
    foreach (@$files) {
      unlink $_ or warn "unlink $_: $!";
    }
  }

  check_for_bonus_files ('.', '.', '..');
}

sub Makefile_PL {
  my $package = shift;
  ################ Makefile.PL
  # We really need a Makefile.PL because make test for a no dynamic linking perl
  # will run Makefile.PL again as part of the "make perl" target.
  my $makefilePL = "Makefile.PL";
  open FH, ">$makefilePL" or die "open >$makefilePL: $!\n";
  print FH <<"EOT";
#!$perl -w
use ExtUtils::MakeMaker;
WriteMakefile(
              'NAME'		=> "$package",
              'VERSION_FROM'	=> "$package.pm", # finds \$VERSION
              (\$] >= 5.005 ?
               (#ABSTRACT_FROM => "$package.pm", # XXX add this
                AUTHOR     => "$0") : ())
             );
EOT

  close FH or die "close $makefilePL: $!\n";
  return $makefilePL;
}

sub MANIFEST {
  my (@files) = @_;
  ################ MANIFEST
  # We really need a MANIFEST because make distclean checks it.
  my $manifest = "MANIFEST";
  push @files, $manifest;
  open FH, ">$manifest" or die "open >$manifest: $!\n";
  print FH "$_\n" foreach @files;
  close FH or die "close $manifest: $!\n";
  return @files;
}

sub write_and_run_extension {
  my ($name, $items, $export_names, $package, $header, $testfile, $num_tests,
      $wc_args) = @_;

  local *C;
  local *XS;

  my $c = tie *C, 'TieOut';
  my $xs = tie *XS, 'TieOut';

  ExtUtils::Constant::WriteConstants(C_FH => \*C,
				     XS_FH => \*XS,
				     NAME => $package,
				     NAMES => $items,
				     @$wc_args,
				     );

  my $C_code = $c->read();
  my $XS_code = $xs->read();

  undef $c;
  undef $xs;

  untie *C;
  untie *XS;

  # Don't check the regeneration code if we specify extra arguments to
  # WriteConstants. (Fix this to give finer grained control if needed)
  my $expect;
  $expect = $C_code . "\n#### XS Section:\n" . $XS_code unless $wc_args;

  print "# $name\n# $dir/$subdir being created...\n";
  mkdir $subdir, 0777 or die "mkdir: $!\n";
  chdir $subdir or die $!;

  my @files;

  ################ Header
  my $header_name = "test.h";
  push @files, $header_name;
  open FH, ">$header_name" or die "open >$header_name: $!\n";
  print FH $header or die $!;
  close FH or die "close $header_name: $!\n";

  ################ XS
  my $xs_name = "$package.xs";
  push @files, $xs_name;
  open FH, ">$xs_name" or die "open >$xs_name: $!\n";

  print FH <<"EOT";
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "$header_name"


$C_code
MODULE = $package		PACKAGE = $package
PROTOTYPES: ENABLE
$XS_code;
EOT

  close FH or die "close $xs: $!\n";

  ################ PM
  my $pm = "$package.pm";
  push @files, $pm;
  open FH, ">$pm" or die "open >$pm: $!\n";
  print FH "package $package;\n";
  print FH "use $];\n";

  print FH <<'EOT';

use strict;
EOT
  printf FH "use warnings;\n" unless $] < 5.006;
  print FH <<'EOT';
use Carp;

require Exporter;
require DynaLoader;
use vars qw ($VERSION @ISA @EXPORT_OK $AUTOLOAD);

$VERSION = '0.01';
@ISA = qw(Exporter DynaLoader);
EOT
  # Having this qw( in the here doc confuses cperl mode far too much to be
  # helpful. And I'm using cperl mode to edit this, even if you're not :-)
  print FH "\@EXPORT_OK = qw(\n";

  # Print the names of all our autoloaded constants
  print FH "\t$_\n" foreach (@$export_names);
  print FH ");\n";
  # Print the AUTOLOAD subroutine ExtUtils::Constant generated for us
  print FH autoload ($package, $]);
  print FH "$package->bootstrap(\$VERSION);\n1;\n__END__\n";
  close FH or die "close $pm: $!\n";

  ################ test.pl
  my $testpl = "test.pl";
  push @files, $testpl;
  open FH, ">$testpl" or die "open >$testpl: $!\n";
  # Standard test header (need an option to suppress this?)
  print FH <<"EOT" or die $!;
use strict;
use $package qw(@$export_names);

print "1..2\n";
if (open OUTPUT, ">$output") {
  print "ok 1\n";
  select OUTPUT;
} else {
  print "not ok 1 # Failed to open '$output': \$!\n";
  exit 1;
}
EOT
  print FH $testfile or die $!;
  print FH <<"EOT" or die $!;
select STDOUT;
if (close OUTPUT) {
  print "ok 2\n";
} else {
  print "not ok 2 # Failed to close '$output': \$!\n";
}
EOT
  close FH or die "close $testpl: $!\n";

  push @files, Makefile_PL($package);
  @files = MANIFEST (@files);

  build_and_run ($num_tests, $expect, \@files);

  chdir $updir or die "chdir '$updir': $!";
  ++$subdir;
}

# Tests are arrayrefs of the form
# $name, [items], [export_names], $package, $header, $testfile, $num_tests
my @tests;
my $before_tests = 4; # Number of "ok"s emitted to build extension
my $after_tests = 8; # Number of "ok"s emitted after make test run
my $dummytest = 1;

my $here;
sub start_tests {
  $dummytest += $before_tests;
  $here = $dummytest;
}
sub end_tests {
  my ($name, $items, $export_names, $header, $testfile, $args) = @_;
  push @tests, [$name, $items, $export_names, $package, $header, $testfile,
               $dummytest - $here, $args];
  $dummytest += $after_tests;
}

my $pound;
if (ord('A') == 193) {  # EBCDIC platform
  $pound = chr 177; # A pound sign. (Currency)
} else { # ASCII platform
  $pound = chr 163; # A pound sign. (Currency)
}
my @common_items = (
                    {name=>"perl", type=>"PV",},
                    {name=>"*/", type=>"PV", value=>'"CLOSE"', macro=>1},
                    {name=>"/*", type=>"PV", value=>'"OPEN"', macro=>1},
                    {name=>$pound, type=>"PV", value=>'"Sterling"', macro=>1},
                   );

my @args = undef;
push @args, [PROXYSUBS => 1] if $] > 5.009002;
foreach my $args (@args)
{
  # Simple tests
  start_tests();
  my $parent_rfc1149 =
    'A Standard for the Transmission of IP Datagrams on Avian Carriers';
  # Test the code that generates 1 and 2 letter name comparisons.
  my %compass = (
                 N => 0, 'NE' => 45, E => 90, SE => 135,
                 S => 180, SW => 225, W => 270, NW => 315
                );

  my $header = << "EOT";
#define FIVE 5
#define OK6 "ok 6\\n"
#define OK7 1
#define FARTHING 0.25
#define NOT_ZERO 1
#define Yes 0
#define No 1
#define Undef 1
#define RFC1149 "$parent_rfc1149"
#undef NOTDEF
#define perl "rules"
EOT

  while (my ($point, $bearing) = each %compass) {
    $header .= "#define $point $bearing\n"
  }

  my @items = ("FIVE", {name=>"OK6", type=>"PV",},
               {name=>"OK7", type=>"PVN",
                value=>['"not ok 7\\n\\0ok 7\\n"', 15]},
               {name => "FARTHING", type=>"NV"},
               {name => "NOT_ZERO", type=>"UV", value=>"~(UV)0"},
               {name => "OPEN", type=>"PV", value=>'"/*"', macro=>1},
               {name => "CLOSE", type=>"PV", value=>'"*/"',
                macro=>["#if 1\n", "#endif\n"]},
               {name => "ANSWER", default=>["UV", 42]}, "NOTDEF",
               {name => "Yes", type=>"YES"},
               {name => "No", type=>"NO"},
               {name => "Undef", type=>"UNDEF"},
  # OK. It wasn't really designed to allow the creation of dual valued
  # constants.
  # It was more for INADDR_ANY INADDR_BROADCAST INADDR_LOOPBACK INADDR_NONE
               {name=>"RFC1149", type=>"SV", value=>"sv_2mortal(temp_sv)",
                pre=>"SV *temp_sv = newSVpv(RFC1149, 0); "
                . "(void) SvUPGRADE(temp_sv,SVt_PVIV); SvIOK_on(temp_sv); "
                . "SvIV_set(temp_sv, 1149);"},
              );

  push @items, $_ foreach keys %compass;

  # Automatically compile the list of all the macro names, and make them
  # exported constants.
  my @export_names = map {(ref $_) ? $_->{name} : $_} @items;

  # Exporter::Heavy (currently) isn't able to export the last 3 of these:
  push @items, @common_items;

  my $test_body = <<"EOT";

my \$test = $dummytest;

EOT

  $test_body .= <<'EOT';
# What follows goes to the temporary file.
# IV
my $five = FIVE;
if ($five == 5) {
  print "ok $test\n";
} else {
  print "not ok $test # \$five\n";
}
$test++;

# PV
if (OK6 eq "ok 6\n") {
  print "ok $test\n";
} else {
  print "not ok $test # \$five\n";
}
$test++;

# PVN containing embedded \0s
$_ = OK7;
s/.*\0//s;
s/7/$test/;
$test++;
print;

# NV
my $farthing = FARTHING;
if ($farthing == 0.25) {
  print "ok $test\n";
} else {
  print "not ok $test # $farthing\n";
}
$test++;

EOT

  my $cond;
  if ($] >= 5.006 || $Config{longsize} < 8) {
    $cond = '$not_zero > 0 && $not_zero == ~0';
  } else {
    $cond = q{pack 'Q', $not_zero eq ~pack 'Q', 0};
  }

  $test_body .= sprintf <<'EOT', $cond;
# UV
my $not_zero = NOT_ZERO;
if (%s) {
  print "ok $test\n";
} else {
  print "not ok $test # \$not_zero=$not_zero ~0=" . (~0) . "\n";
}
$test++;

EOT

  $test_body .= <<'EOT';

# Value includes a "*/" in an attempt to bust out of a C comment.
# Also tests custom cpp #if clauses
my $close = CLOSE;
if ($close eq '*/') {
  print "ok $test\n";
} else {
  print "not ok $test # \$close='$close'\n";
}
$test++;

# Default values if macro not defined.
my $answer = ANSWER;
if ($answer == 42) {
  print "ok $test\n";
} else {
  print "not ok $test # What do you get if you multiply six by nine? '$answer'\n";
}
$test++;

# not defined macro
my $notdef = eval { NOTDEF; };
if (defined $notdef) {
  print "not ok $test # \$notdef='$notdef'\n";
} elsif ($@ !~ /Your vendor has not defined ExtTest macro NOTDEF/) {
  print "not ok $test # \$@='$@'\n";
} else {
  print "ok $test\n";
}
$test++;

# not a macro
my $notthere = eval { &ExtTest::NOTTHERE; };
if (defined $notthere) {
  print "not ok $test # \$notthere='$notthere'\n";
} elsif ($@ !~ /NOTTHERE is not a valid ExtTest macro/) {
  chomp $@;
  print "not ok $test # \$@='$@'\n";
} else {
  print "ok $test\n";
}
$test++;

# Truth
my $yes = Yes;
if ($yes) {
  print "ok $test\n";
} else {
  print "not ok $test # $yes='\$yes'\n";
}
$test++;

# Falsehood
my $no = No;
if (defined $no and !$no) {
  print "ok $test\n";
} else {
  print "not ok $test # \$no=" . defined ($no) ? "'$no'\n" : "undef\n";
}
$test++;

# Undef
my $undef = Undef;
unless (defined $undef) {
  print "ok $test\n";
} else {
  print "not ok $test # \$undef='$undef'\n";
}
$test++;

# invalid macro (chosen to look like a mix up between No and SW)
$notdef = eval { &ExtTest::So };
if (defined $notdef) {
  print "not ok $test # \$notdef='$notdef'\n";
} elsif ($@ !~ /^So is not a valid ExtTest macro/) {
  print "not ok $test # \$@='$@'\n";
} else {
  print "ok $test\n";
}
$test++;

# invalid defined macro
$notdef = eval { &ExtTest::EW };
if (defined $notdef) {
  print "not ok $test # \$notdef='$notdef'\n";
} elsif ($@ !~ /^EW is not a valid ExtTest macro/) {
  print "not ok $test # \$@='$@'\n";
} else {
  print "ok $test\n";
}
$test++;

my %compass = (
EOT

while (my ($point, $bearing) = each %compass) {
  $test_body .= "'$point' => $bearing, "
}

$test_body .= <<'EOT';

);

my $fail;
while (my ($point, $bearing) = each %compass) {
  my $val = eval $point;
  if ($@) {
    print "# $point: \$@='$@'\n";
    $fail = 1;
  } elsif (!defined $bearing) {
    print "# $point: \$val=undef\n";
    $fail = 1;
  } elsif ($val != $bearing) {
    print "# $point: \$val=$val, not $bearing\n";
    $fail = 1;
  }
}
if ($fail) {
  print "not ok $test\n";
} else {
  print "ok $test\n";
}
$test++;

EOT

$test_body .= <<"EOT";
my \$rfc1149 = RFC1149;
if (\$rfc1149 ne "$parent_rfc1149") {
  print "not ok \$test # '\$rfc1149' ne '$parent_rfc1149'\n";
} else {
  print "ok \$test\n";
}
\$test++;

if (\$rfc1149 != 1149) {
  printf "not ok \$test # %d != 1149\n", \$rfc1149;
} else {
  print "ok \$test\n";
}
\$test++;

EOT

$test_body .= <<'EOT';
# test macro=>1
my $open = OPEN;
if ($open eq '/*') {
  print "ok $test\n";
} else {
  print "not ok $test # \$open='$open'\n";
}
$test++;
EOT
$dummytest+=18;

  end_tests("Simple tests", \@items, \@export_names, $header, $test_body,
	    $args);
}

if ($do_utf_tests) {
  # utf8 tests
  start_tests();
  my ($inf, $pound_bytes, $pound_utf8);

  $inf = chr 0x221E;
  # Check that we can distiguish the pathological case of a string, and the
  # utf8 representation of that string.
  $pound_utf8 = $pound . '1';
  if ($better_than_56) {
    $pound_bytes = $pound_utf8;
    utf8::encode ($pound_bytes);
  } else {
    # Must have that "U*" to generate a zero length UTF string that forces
    # top bit set chars (such as the pound sign) into UTF8, so that the
    # unpack 'C*' then gets the byte form of the UTF8.
    $pound_bytes =  pack 'C*', unpack 'C*', $pound_utf8 . pack "U*";
  }

  my @items = (@common_items,
               {name=>$inf, type=>"PV", value=>'"Infinity"', macro=>1},
               {name=>$pound_utf8, type=>"PV", value=>'"1 Pound"', macro=>1},
               {name=>$pound_bytes, type=>"PV", value=>'"1 Pound (as bytes)"',
                macro=>1},
              );

=pod

The above set of names seems to produce a suitably bad set of compile
problems on a Unicode naive version of ExtUtils::Constant (ie 0.11):

nick@thinking-cap 15439-32-utf$ PERL_CORE=1 ./perl lib/ExtUtils/t/Constant.t
1..33
# perl=/stuff/perl5/15439-32-utf/perl
# ext-30370 being created...
Wide character in print at lib/ExtUtils/t/Constant.t line 140.
ok 1
ok 2
# make = 'make'
ExtTest.xs: In function `constant_1':
ExtTest.xs:80: warning: multi-character character constant
ExtTest.xs:80: warning: case value out of range
ok 3

=cut

# Grr `

  # Do this in 7 bit in case someone is testing with some settings that cause
  # 8 bit files incapable of storing this character.
  my @values
    = map {"'" . join (",", unpack "U*", $_ . pack "U*") . "'"}
      ($pound, $inf, $pound_bytes, $pound_utf8);
  # Values is a list of strings, such as ('194,163,49', '163,49')

  my $test_body .= "my \$test = $dummytest;\n";
  $dummytest += 7 * 3; # 3 tests for each of the 7 things:

  $test_body .= << 'EOT';

use utf8;
my $better_than_56 = $] > 5.007;

my ($pound, $inf, $pound_bytes, $pound_utf8) = map {eval "pack 'U*', $_"}
EOT

  $test_body .= join ",", @values;

  $test_body .= << 'EOT';
;

foreach (["perl", "rules", "rules"],
	 ["/*", "OPEN", "OPEN"],
	 ["*/", "CLOSE", "CLOSE"],
	 [$pound, 'Sterling', []],
         [$inf, 'Infinity', []],
	 [$pound_utf8, '1 Pound', '1 Pound (as bytes)'],
	 [$pound_bytes, '1 Pound (as bytes)', []],
        ) {
  # Flag an expected error with a reference for the expect string.
  my ($string, $expect, $expect_bytes) = @$_;
  (my $name = $string) =~ s/([^ !"#\$%&'()*+,\-.\/0-9:;<=>?\@A-Z[\\\]^_`a-z{|}~])/sprintf '\x{%X}', ord $1/ges;
  print "# \"$name\" => \'$expect\'\n";
  # Try to force this to be bytes if possible.
  if ($better_than_56) {
    utf8::downgrade ($string, 1);
  } else {
    if ($string =~ tr/0-\377// == length $string) {
      # No chars outside range 0-255
      $string = pack 'C*', unpack 'U*', ($string . pack 'U*');
    }
  }
EOT

  $test_body .=  "my (\$error, \$got) = ${package}::constant (\$string);\n";

  $test_body .= <<'EOT';
  if ($error or $got ne $expect) {
    print "not ok $test # error '$error', got '$got'\n";
  } else {
    print "ok $test\n";
  }
  $test++;
  print "# Now upgrade '$name' to utf8\n";
  if ($better_than_56) {
    utf8::upgrade ($string);
  } else {
    $string = pack ('U*') . $string;
  }
EOT

  $test_body .=  "my (\$error, \$got) = ${package}::constant (\$string);\n";

  $test_body .= <<'EOT';
  if ($error or $got ne $expect) {
    print "not ok $test # error '$error', got '$got'\n";
  } else {
    print "ok $test\n";
  }
  $test++;
  if (defined $expect_bytes) {
    print "# And now with the utf8 byte sequence for name\n";
    # Try the encoded bytes.
    if ($better_than_56) {
      utf8::encode ($string);
    } else {
      $string = pack 'C*', unpack 'C*', $string . pack "U*";
    }
EOT

    $test_body .= "my (\$error, \$got) = ${package}::constant (\$string);\n";

    $test_body .= <<'EOT';
    if (ref $expect_bytes) {
      # Error expected.
      if ($error) {
        print "ok $test # error='$error' (as expected)\n";
      } else {
        print "not ok $test # expected error, got no error and '$got'\n";
      }
    } elsif ($got ne $expect_bytes) {
      print "not ok $test # error '$error', expect '$expect_bytes', got '$got'\n";
    } else {
      print "ok $test\n";
    }
    $test++;
  }
}
EOT

  end_tests("utf8 tests", \@items, [], "#define perl \"rules\"\n", $test_body);
}

# XXX I think that I should merge this into the utf8 test above.
sub explict_call_constant {
  my ($string, $expect) = @_;
  # This does assume simple strings suitable for ''
  my $test_body = <<"EOT";
{
  my (\$error, \$got) = ${package}::constant ('$string');\n;
EOT

  if (defined $expect) {
    # No error expected
    $test_body .= <<"EOT";
  if (\$error or \$got ne "$expect") {
    print "not ok $dummytest # error '\$error', expect '$expect', got '\$got'\n";
  } else {
    print "ok $dummytest\n";
    }
  }
EOT
  } else {
    # Error expected.
    $test_body .= <<"EOT";
  if (\$error) {
    print "ok $dummytest # error='\$error' (as expected)\n";
  } else {
    print "not ok $dummytest # expected error, got no error and '\$got'\n";
  }
EOT
  }
  $dummytest++;
  return $test_body . <<'EOT';
}
EOT
}

# Simple tests to verify bits of the switch generation system work.
sub simple {
  start_tests();
  # Deliberately leave $name in @_, so that it is indexed from 1.
  my ($name, @items) = @_;
  my $test_header;
  my $test_body = "my \$value;\n";
  foreach my $counter (1 .. $#_) {
    my $thisname = $_[$counter];
    $test_header .= "#define $thisname $counter\n";
    $test_body .= <<"EOT";
\$value = $thisname;
if (\$value == $counter) {
  print "ok $dummytest\n";
} else {
  print "not ok $dummytest # $thisname gave \$value\n";
}
EOT
    ++$dummytest;
    # Yes, the last time round the loop appends a z to the string.
    for my $i (0 .. length $thisname) {
      my $copyname = $thisname;
      substr ($copyname, $i, 1) = 'z';
      $test_body .= explict_call_constant ($copyname,
                                           $copyname eq $thisname
                                             ? $thisname : undef);
    }
  }
  # Ho. This seems to be buggy in 5.005_03:
  # # Now remove $name from @_:
  # shift @_;
  end_tests($name, \@items, \@items, $test_header, $test_body);
}

# Check that the memeq clauses work correctly when there isn't a switch
# statement to bump off a character
simple ("Singletons", "A", "AB", "ABC", "ABCD", "ABCDE");
# Check the three code.
simple ("Three start", qw(Bea kea Lea lea nea pea rea sea tea Wea yea Zea));
# There were 162 2 letter words in /usr/share/dict/words on FreeBSD 4.6, which
# I felt was rather too many. So I used words with 2 vowels.
simple ("Twos and three middle", qw(aa ae ai ea eu ie io oe era eta));
# Given the choice go for the end, else the earliest point
simple ("Three end and four symetry", qw(ean ear eat barb marm tart));


# Need this if the single test below is rolled into @tests :
# --$dummytest;
print "1..$dummytest\n";

write_and_run_extension @$_ foreach @tests;

# This was causing an assertion failure (a C<confess>ion)
# Any single byte > 128 should do it.
C_constant ($package, undef, undef, undef, undef, undef, chr 255);
print "ok $realtest\n"; $realtest++;

print STDERR "# You were running with \$keep_files set to $keep_files\n"
  if $keep_files;
