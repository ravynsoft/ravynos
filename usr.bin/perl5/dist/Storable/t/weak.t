#!./perl -w
#
#  Copyright 2004, Larry Wall.
#
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

sub BEGIN {
  # This lets us distribute Test::More in t/
  unshift @INC, 't';
  unshift @INC, 't/compat' if $] < 5.006002;
  require Config; import Config;
  if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
    print "1..0 # Skip: Storable was not built\n";
    exit 0;
  }
  if ($Config{extensions} !~ /\bList\/Util\b/) {
    print "1..0 # Skip: List::Util was not built\n";
    exit 0;
  }

  require Scalar::Util;
  Scalar::Util->import(qw(weaken isweak));
  if (grep { /weaken/ } @Scalar::Util::EXPORT_FAIL) {
    print("1..0 # Skip: No support for weaken in Scalar::Util\n");
    exit 0;
  }
}

use Test::More 'no_plan';
use Storable qw (store retrieve freeze thaw nstore nfreeze dclone);
require 'testlib.pl';
our $file;
use strict;

# $Storable::flags = Storable::FLAGS_COMPAT;

sub tester {
  my ($contents, $sub, $testersub, $what) = @_;
  # Test that if we re-write it, everything still works:
  my $clone = &$sub ($contents);
  is ($@, "", "There should be no error extracting for $what");
  &$testersub ($clone, $what);
}

my $r = {};
my $s1 = [$r, $r];
weaken $s1->[1];
ok (isweak($s1->[1]), "element 1 is a weak reference");

my $s0 = [$r, $r];
weaken $s0->[0];
ok (isweak($s0->[0]), "element 0 is a weak reference");

my $w = [$r];
weaken $w->[0];
ok (isweak($w->[0]), "element 0 is a weak reference");

package OVERLOADED;

use overload
	'""' => sub { $_[0][0] };

package main;

$a = bless [77], 'OVERLOADED';

my $o = [$a, $a];
weaken $o->[0];
ok (isweak($o->[0]), "element 0 is a weak reference");

my @tests = (
[$s1,
 sub  {
  my ($clone, $what) = @_;
  isa_ok($clone,'ARRAY');
  isa_ok($clone->[0],'HASH');
  isa_ok($clone->[1],'HASH');
  ok(!isweak $clone->[0], "Element 0 isn't weak");
  ok(isweak $clone->[1], "Element 1 is weak");
}
],
# The weak reference needs to hang around long enough for other stuff to
# be able to make references to it. So try it second.
[$s0,
 sub  {
  my ($clone, $what) = @_;
  isa_ok($clone,'ARRAY');
  isa_ok($clone->[0],'HASH');
  isa_ok($clone->[1],'HASH');
  ok(isweak $clone->[0], "Element 0 is weak");
  ok(!isweak $clone->[1], "Element 1 isn't weak");
}
],
[$w,
 sub  {
  my ($clone, $what) = @_;
  isa_ok($clone,'ARRAY');
  if ($what eq 'nothing') {
    # We're the original, so we're still a weakref to a hash
    isa_ok($clone->[0],'HASH');
    ok(isweak $clone->[0], "Element 0 is weak");
  } else {
    is($clone->[0],undef);
  }
}
],
[$o,
sub {
  my ($clone, $what) = @_;
  isa_ok($clone,'ARRAY');
  isa_ok($clone->[0],'OVERLOADED');
  isa_ok($clone->[1],'OVERLOADED');
  ok(isweak $clone->[0], "Element 0 is weak");
  ok(!isweak $clone->[1], "Element 1 isn't weak");
  is ("$clone->[0]", 77, "Element 0 stringifies to 77");
  is ("$clone->[1]", 77, "Element 1 stringifies to 77");
}
],
);

foreach (@tests) {
  my ($input, $testsub) = @$_;

  tester($input, sub {return shift}, $testsub, 'nothing');

  ok (defined store($input, $file));

  # Read the contents into memory:
  my $contents = slurp ($file);

  tester($contents, \&store_and_retrieve, $testsub, 'file');

  # And now try almost everything again with a Storable string
  my $stored = freeze $input;
  tester($stored, \&freeze_and_thaw, $testsub, 'string');

  ok (defined nstore($input, $file));

  tester($contents, \&store_and_retrieve, $testsub, 'network file');

  $stored = nfreeze $input;
  tester($stored, \&freeze_and_thaw, $testsub, 'network string');
}

{
    # [perl #134179] sv_upgrade from type 7 down to type 1
    my $foo = [qr//,[]];
    weaken($foo->[1][0][0] = $foo->[1]);
    my $out = dclone($foo); # croaked here
    is_deeply($out, $foo, "check they match");
}
