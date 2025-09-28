#!./perl -w

# Some quick tests to see if h2xs actually runs and creates files as 
# expected.  File contents include date stamps and/or usernames
# hence are not checked.  File existence is checked with -e though.
# This test depends on File::Path::rmtree() to clean up with.
#  - pvhp
#
# We are now checking that the correct use $version; is present in
# Makefile.PL and $module.pm

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    # FIXME (or rather FIXh2xs)
    require Config;
    if (($Config::Config{'extensions'} !~ m!\bDevel/PPPort\b!) ){
	print "1..0 # Skip -- Perl configured without Devel::PPPort module\n";
	exit 0;
    }
}

# use strict; # we are not really testing this
use File::Path;  # for cleaning up with rmtree()
use Test::More;
use File::Spec;
use File::Find;
use ExtUtils::Manifest;
# Don't want its diagnostics getting in the way of ours.
$ExtUtils::Manifest::Quiet=1;
my $up = File::Spec->updir();

my $extracted_program = '../utils/h2xs'; # unix, nt, ...

my $Is_VMS_traildot = 0;
if ($^O eq 'VMS') {
    $extracted_program = '[-.utils]h2xs.com';

    # We have to know if VMS is in UNIX mode.  In UNIX mode, trailing dots
    # should not be present.  There are actually two settings that control this.

    $Is_VMS_traildot = 1;
    my $unix_rpt = 0;
    my $drop_dot = 0;
    if (eval 'require VMS::Feature') {
        $unix_rpt = VMS::Feature::current('filename_unix_report');
        $drop_dot = VMS::Feature::current('readdir_dropdotnotype');
    } else {
        my $unix_report = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        $unix_rpt = $unix_report =~ /^[ET1]/i; 
        my $drop_dot_notype = $ENV{'DECC$READDIR_DROPDOTNOTYPE'} || '';
        $drop_dot = $drop_dot_notype =~ /^[ET1]/i;
    }
    $Is_VMS_traildot = 0 if $drop_dot && unix_rpt;
}
if (!(-e $extracted_program)) {
    print "1..0 # Skip: $extracted_program was not built\n";
    exit 0;
}
# You might also wish to bail out if your perl platform does not
# do `$^X -e 'warn "Writing h2xst"' 2>&1`; duplicity.

# ok on unix, nt, VMS, ...
my $dupe = '2>&1';
# ok on unix, nt, The extra \" are for VMS
my $lib = '"-I../lib" "-I../../lib"';
# $name should differ from system header file names and must
# not already be found in the t/ subdirectory for perl.
my $name = 'h2xst';
my $header = "$name.h";
my $thisversion = sprintf "%vd", $^V;
$thisversion =~ s/^v//;

# If this test has failed previously a copy may be left.
rmtree($name) if -e $name;

my @tests = (
"-f -n $name", $], <<"EOXSFILES",
Defaulting to backwards compatibility with perl $thisversion
If you intend this module to be compatible with earlier perl versions, please
specify a minimum perl version with the -b option.

Writing $name/ppport.h
Writing $name/lib/$name.pm
Writing $name/$name.xs
Writing $name/fallback/const-c.inc
Writing $name/fallback/const-xs.inc
Writing $name/Makefile.PL
Writing $name/README
Writing $name/t/$name.t
Writing $name/Changes
Writing $name/MANIFEST
EOXSFILES

"-f -n $name -b $thisversion", $], <<"EOXSFILES",
Writing $name/ppport.h
Writing $name/lib/$name.pm
Writing $name/$name.xs
Writing $name/fallback/const-c.inc
Writing $name/fallback/const-xs.inc
Writing $name/Makefile.PL
Writing $name/README
Writing $name/t/$name.t
Writing $name/Changes
Writing $name/MANIFEST
EOXSFILES

"-f -n $name -b 5.6.1", "5.006001", <<"EOXSFILES",
Writing $name/ppport.h
Writing $name/lib/$name.pm
Writing $name/$name.xs
Writing $name/fallback/const-c.inc
Writing $name/fallback/const-xs.inc
Writing $name/Makefile.PL
Writing $name/README
Writing $name/t/$name.t
Writing $name/Changes
Writing $name/MANIFEST
EOXSFILES

"-f -n $name -b 5.5.3", "5.00503", <<"EOXSFILES",
Writing $name/ppport.h
Writing $name/lib/$name.pm
Writing $name/$name.xs
Writing $name/fallback/const-c.inc
Writing $name/fallback/const-xs.inc
Writing $name/Makefile.PL
Writing $name/README
Writing $name/t/$name.t
Writing $name/Changes
Writing $name/MANIFEST
EOXSFILES

"\"-X\" -f -n $name -b $thisversion", $], <<"EONOXSFILES",
Writing $name/lib/$name.pm
Writing $name/Makefile.PL
Writing $name/README
Writing $name/t/$name.t
Writing $name/Changes
Writing $name/MANIFEST
EONOXSFILES

"-f -n $name -b $thisversion $header", $], <<"EOXSFILES",
Writing $name/ppport.h
Writing $name/lib/$name.pm
Writing $name/$name.xs
Writing $name/fallback/const-c.inc
Writing $name/fallback/const-xs.inc
Writing $name/Makefile.PL
Writing $name/README
Writing $name/t/$name.t
Writing $name/Changes
Writing $name/MANIFEST
EOXSFILES
);

ok (open (HEADER, '>', $header), "open '$header'");
print HEADER <<HEADER or die $!;
#define Camel 2
#define Dromedary 1
#define Bactrian /* empty */
#define Bactrian2
HEADER
ok (close (HEADER), "close '$header'");

while (my ($args, $version, $expectation) = splice @tests, 0, 3) {
  # h2xs warns about what it is writing hence the (possibly unportable)
  # 2>&1 dupe:
  # does it run?
  my $prog = "$^X $lib $extracted_program $args $dupe";
  @result = `$prog`;
  cmp_ok ($?, "==", 0, "running $prog ");
  $result = join("",@result);

  #print "# expectation is >$expectation<\n";
  #print "# result is >$result<\n";
  # Was the output the list of files that were expected?
  is ($result, $expectation, "running $prog");

  my (%got);
  find (sub {$got{$File::Find::name}++ unless -d $_}, $name);

  foreach ($expectation =~ /Writing\s+(\S+)/gm) {
    if ($^O eq 'VMS') {
      if ($Is_VMS_traildot) {
          $_ .= '.' unless $_ =~ m/\./;
      }
      $_ = lc($_) unless exists $got{$_};
    }
    ok (-e $_, "check for $_") and delete $got{$_};
  }
  my @extra = keys %got;
  unless (ok (!@extra, "Are any extra files present?")) {
    print "# These files are unexpectedly present:\n";
    print "# $_\n" foreach sort @extra;
  }

  chdir ($name) or die "chdir $name failed: $!";
  # Aargh. Something wants to load a bit of regexp. And we have to chdir
  # for ExtUtils::Manifest. Caught between a rock and a hard place, so this
  # seems the least evil thing to do:
  push @INC, "../../lib";
  my ($missing, $extra) = ExtUtils::Manifest::fullcheck();
  is_deeply ($missing, [], "No files in the MANIFEST should be missing");
  is_deeply ($extra, [],   "and all files present should be in the MANIFEST");
  pop @INC;
  chdir ($up) or die "chdir $up failed: $!";
 
  if ($args =~ / \Q$header\E$/) {
    my $const_c = File::Spec->catfile($name, 'fallback', 'const-c.inc');
    my ($found, $diag);
    if (!open FILE, '<', $const_c) {
      $diag = "can't open $const_c: $!";
    }
    else {
      while (<FILE>) {
        next unless /\b Bactrian 2? \b/x;
        $found = 1;
        last;
      }
    }
    ok (!$found, "generated $const_c has no Bactrian(2)");
    diag ($diag) if defined $diag;
  }

  foreach my $leaf (File::Spec->catfile('lib', "$name.pm"), 'Makefile.PL') {
    my $file = File::Spec->catfile($name, $leaf);
    if (ok (open (FILE, '<', $file), "open $file")) {
      my $match = qr/use $version;/;
      my $found;
      while (<FILE>) {
        last if $found = /$match/;
      }
      ok ($found, "looking for /$match/ in $file");
      close FILE or die "close $file: $!";
    }
  }
  # clean up
  rmtree($name);
}

cmp_ok (unlink ($header), "==", 1, "unlink '$header'") or die "\$! is $!";

done_testing();
