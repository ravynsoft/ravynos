#!./perl -w

use strict;
use Devel::SelfStubber;
use File::Spec::Functions;

my $runperl = $^X;

# ensure correct output ordering for system() calls

select STDERR; $| = 1; select STDOUT; $| = 1;

print "1..12\n";

my @cleanup;

END {
  foreach my $file (reverse @cleanup) {
    unlink $file or warn "unlink $file failed: $!" while -f $file;
    rmdir $file or warn "rmdir $file failed: $!" if -d $file;
  }
}

my $inlib = "SSI-$$";
mkdir $inlib, 0777 or die $!;
push @cleanup, $inlib;

while (<DATA>) {
  if (/^\#{16,}\s+(.*)/) {
    my $f = $1;
    my $file = catfile(curdir(),$inlib,$f);
    push @cleanup, $file;
    open FH, '>', $file or die $!;
  } else {
    print FH;
  }
}
close FH;

{
  my $file = "A-$$";
  push @cleanup, $file;
  open FH, '>', $file or die $!;
  select FH;
  Devel::SelfStubber->stub('xChild', $inlib);
  select STDOUT;
  print "ok 1\n";
  close FH or die $!;

  open FH, '<', $file or die $!;
  my @A = <FH>;

  if (@A == 1 && $A[0] =~ /^\s*sub\s+xChild::foo\s*;\s*$/) {
    print "ok 2\n";
  } else {
    print "not ok 2\n";
    print "# $_" foreach (@A);
  }
}

{
  my $file = "B-$$";
  push @cleanup, $file;
  open FH, '>', $file or die $!;
  select FH;
  Devel::SelfStubber->stub('Proto', $inlib);
  select STDOUT;
  print "ok 3\n"; # Checking that we did not die horribly.
  close FH or die $!;

  open FH, '<', $file or die $!;
  my @B = <FH>;

  if (@B == 1 && $B[0] =~ /^\s*sub\s+Proto::bar\s*\(\$\$\);\s*$/) {
    print "ok 4\n";
  } else {
    print "not ok 4\n";
    print "# $_" foreach (@B);
  }

  close FH or die $!;
}

{
  my $file = "C-$$";
  push @cleanup, $file;
  open FH, '>', $file or die $!;
  select FH;
  Devel::SelfStubber->stub('Attribs', $inlib);
  select STDOUT;
  print "ok 5\n"; # Checking that we did not die horribly.
  close FH or die $!;

  open FH, '<', $file or die $!;
  my @C = <FH>;

  if (@C == 2 && $C[0] =~ /^\s*sub\s+Attribs::baz\s+:\s*locked\s*;\s*$/
      && $C[1] =~ /^\s*sub\s+Attribs::lv\s+:\s*lvalue\s*:\s*method\s*;\s*$/) {
    print "ok 6\n";
  } else {
    print "not ok 6\n";
    print "# $_" foreach (@C);
  }

  close FH or die $!;
}

# "wrong" and "right" may change if SelfLoader is changed.
my %wrong = ( xParent => 'xParent', xChild => 'xParent' );
my %right = ( xParent => 'xParent', xChild => 'xChild' );
if ($^O eq 'VMS') {
    # extra line feeds for MBX IPC
    %wrong = ( xParent => "xParent\n", xChild => "xParent\n" );
    %right = ( xParent => "xParent\n", xChild => "xChild\n" );
}
my @module = qw(xParent xChild)
;
sub fail {
  my ($left, $right) = @_;
  while (my ($key, $val) = each %$left) {
    # warn "$key $val $$right{$key}";
    return 1
      unless $val eq $$right{$key};
  }
  return;
}

sub faildump {
  my ($expect, $got) = @_;
  foreach (sort keys %$expect) {
    print "# $_ expect '$$expect{$_}' got '$$got{$_}'\n";
  }
}

# Now test that the module tree behaves "wrongly" as expected

foreach my $module (@module) {
  my $file = "$module--$$";
  push @cleanup, $file;
  open FH, '>', $file or die $!;
  print FH "use $module;
print ${module}->foo;
";
  close FH or die $!;
}

{
  my %output;
  foreach my $module (@module) {
    print "# $runperl \"-I$inlib\" $module--$$\n";
    ($output{$module} = `$runperl "-I$inlib" $module--$$`)
      =~ s/\'s foo//;
  }

  if (&fail (\%wrong, \%output)) {
    print "not ok 7\n", &faildump (\%wrong, \%output);
  } else {
    print "ok 7\n";
  }
}

my $lib="SSO-$$";
mkdir $lib, 0777 or die $!;
push @cleanup, $lib;
$Devel::SelfStubber::JUST_STUBS=0;

undef $/;
foreach my $module (@module, 'Data', 'End') {
  my $file = catfile(curdir(),$lib,"$module.pm");
  my $fileo = catfile(curdir(),$inlib,"$module.pm");
  open FH, '<', $fileo or die "Can't open $fileo: $!";
  my $contents = <FH>;
  close FH or die $!;
  push @cleanup, $file;
  open FH, '>', $file or die $!;
  select FH;
  if ($contents =~ /__DATA__/) {
    # This will die for any module with no  __DATA__
    Devel::SelfStubber->stub($module, $inlib);
  } else {
    print $contents;
  }
  select STDOUT;
  close FH or die $!;
}
print "ok 8\n";

{
  my %output;
  foreach my $module (@module) {
    print "# $runperl \"-I$lib\" $module--$$\n";
    ($output{$module} = `$runperl "-I$lib" $module--$$`)
      =~ s/\'s foo//;
  }

  if (&fail (\%right, \%output)) {
    print "not ok 9\n", &faildump (\%right, \%output);
  } else {
    print "ok 9\n";
  }
}

# Check that the DATA handle stays open
system "$runperl -w \"-I$lib\" \"-MData\" -e \"Data::ok\"";

# Possibly a pointless test as this doesn't really verify that it's been
# stubbed.
system "$runperl -w \"-I$lib\" \"-MEnd\" -e \"End::lime\"";

# But check that the documentation after the __END__ survived.
open FH, '<', catfile(curdir(),$lib,"End.pm") or die $!;
$_ = <FH>;
close FH or die $!;

if (/Did the documentation here survive\?/) {
  print "ok 12\n";
} else {
  print "not ok 12 # information after an __END__ token seems to be lost\n";
}

__DATA__
################ xParent.pm
package xParent;

sub foo {
  return __PACKAGE__;
}
1;
__END__
################ xChild.pm
package xChild;
require xParent;
@ISA = 'xParent';
use SelfLoader;

1;
__DATA__
sub foo {
  return __PACKAGE__;
}
__END__
################ Proto.pm
package Proto;
use SelfLoader;

1;
__DATA__
sub bar ($$) {
}
################ Attribs.pm
package Attribs;
use SelfLoader;

1;
__DATA__
sub baz : locked {
}
sub lv : lvalue : method {
  my $a;
  \$a;
}
################ Data.pm
package Data;
use SelfLoader;

1;
__DATA__
sub ok {
  print <DATA>;
}
__END__ DATA
ok 10
################ End.pm
package End;
use SelfLoader;

1;
__DATA__
sub lime {
  print "ok 11\n";
}
__END__
Did the documentation here survive?
