#!perl

# This test file contains 57 tests.
# You need to number them manually. Don't forget to update this line for the
# next kind hacker.

END {print "not ok 1\n" unless $loaded;}
use v5.6.0;
use Attribute::Handlers;
$loaded = 1;

CHECK { $main::phase++ }

######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

sub ok { $::count++; push @::results, [$_[1], $_[0]?"":"not ", defined($_[2])?$_[2]:""]; }

END { print "1..$::count\n";
      print map "$_->[1]ok $_->[0] $_->[2]\n",
		sort {$a->[0]<=>$b->[0]}
			grep $_->[0], @::results }

package Test;
use warnings;
no warnings 'redefine';

sub UNIVERSAL::Lastly :ATTR(INIT) { ::ok $_[4][0] && $main::phase, $_[4][1] }

sub UNIVERSAL::Okay :ATTR(BEGIN) {
::ok $_[4][0] && (!$main::phase || !ref $_[1] && $_[1] eq 'LEXICAL'), $_[4][1];
}

sub Dokay :ATTR(SCALAR) { ::ok @{$_[4]} }
sub Dokay :ATTR(HASH)   { ::ok @{$_[4]} }
sub Dokay :ATTR(ARRAY)  { ::ok @{$_[4]} }
sub Dokay :ATTR(CODE)   { ::ok @{$_[4]} }

sub Vokay :ATTR(VAR)    { ::ok @{$_[4]} }

sub Aokay :ATTR(ANY)    { ::ok @{$_[4]} }

package main;
use warnings;

my $x1 :Lastly(1,41);
my @x1 :Lastly(1=>42);
my %x1 :Lastly(1,43);
sub x1 :Lastly(1,44) {}

my Test $x2 :Dokay(1,5);

if ($] < 5.011) {
 ::ok(1, $_, '# skip : invalid before 5.11') for 55 .. 57;
} else {
 my $c = $::count;
 eval '
  my Test @x2 :Dokay(1,55);
  my Test %x2 :Dokay(1,56);
 ';
 $c = $c + 2 - $::count;
 while ($c > 0) {
  ::ok(0, 57 - $c);
  --$c;
 }
 ::ok(!$@, 57);
}

package Test;
my $x3 :Dokay(1,6);
my Test $x4 :Dokay(1,7);
sub x3 :Dokay(1,8) {}

my $y1 :Okay(1,9);
my @y1 :Okay(1,10);
my %y1 :Okay(1,11);
sub y1 :Okay(1,12) {}

my $y2 :Vokay(1,13);
my @y2 :Vokay(1,14);
my %y2 :Vokay(1,15);
# BEGIN {eval 'sub y2 :Vokay(0,16) {}; 1' or
::ok(1,16);
# }

my $z :Aokay(1,17);
my @z :Aokay(1,18);
my %z :Aokay(1,19);
sub z :Aokay(1,20) {};

package DerTest;
use parent qw(Test);
use warnings;

my $x5 :Dokay(1,21);
my Test $x6 :Dokay(1,22);
sub x5 :Dokay(1,23);

my $y3 :Okay(1,24);
my @y3 :Okay(1,25);
my %y3 :Okay(1,26);
sub y3 :Okay(1,27) {}

package Unrelated;

my $x11 :Okay(1,1);
my @x11 :Okay(1=>2);
my %x11 :Okay(1,3);
sub x11 :Okay(1,4) {}

BEGIN { eval 'my $x7 :Dokay(0,28)' or ::ok(1,28); }
my Test $x8 :Dokay(1,29);
eval 'sub x7 :Dokay(0,30) {}' or ::ok(1,30);


package Tie::Loud;

sub TIESCALAR { ::ok(1,31); bless {}, $_[0] }
sub FETCH { ::ok(1,32); return 1 }
sub STORE { ::ok(1,33); return 1 }

package Tie::Noisy;

sub TIEARRAY { ::ok(1,$_[1]); bless {}, $_[0] }
sub FETCH { ::ok(1,35); return 1 }
sub STORE { ::ok(1,36); return 1 }
sub FETCHSIZE { 100 }

package Tie::Row::dy;

sub TIEHASH { ::ok(1,$_[1]); bless {}, $_[0] }
sub FETCH { ::ok(1,38); return 1 }
sub STORE { ::ok(1,39); return 1 }

package main;

eval 'sub x7 :ATTR(SCALAR) :ATTR(CODE) {}' and ::ok(0,40) or ::ok(1,40);

use Attribute::Handlers autotie => {      Other::Loud => Tie::Loud,
				                Noisy => Tie::Noisy,
				     UNIVERSAL::Rowdy => Tie::Row::dy,
                                   };

my Other $loud : Loud;
$loud++;

my @noisy : Noisy(34);
$noisy[0]++;

my %rowdy : Rowdy(37,'this arg should be ignored');
$rowdy{key}++;


# check that applying attributes to lexicals doesn't unduly worry
# their refcounts
my $out = "begin\n";
my $applied;
sub UNIVERSAL::Dummy :ATTR { ++$applied };
sub Dummy::DESTROY { $out .= "bye\n" }

{ my $dummy;          $dummy = bless {}, 'Dummy'; }
ok( $out eq "begin\nbye\n", 45 );

{ my $dummy : Dummy;  $dummy = bless {}, 'Dummy'; }
if($] < 5.008) {
ok( 1, 46, " # skip lexicals are not runtime prior to 5.8");
} else {
ok( $out eq "begin\nbye\nbye\n", 46);
}
# are lexical attributes reapplied correctly?
sub dummy { my $dummy : Dummy; }
$applied = 0;
dummy(); dummy();
if($] < 5.008) {
ok(1, 47, " # skip does not work with perl prior to 5.8");
} else {
ok( $applied == 2, 47 );
}
# 45-47 again, but for our variables
$out = "begin\n";
{ our $dummy;          $dummy = bless {}, 'Dummy'; }
ok( $out eq "begin\n", 48 );
{ no warnings; our $dummy : Dummy;  $dummy = bless {}, 'Dummy'; }
ok( $out eq "begin\nbye\n", 49 );
undef $::dummy;
ok( $out eq "begin\nbye\nbye\n", 50 );

# are lexical attributes reapplied correctly?
sub dummy_our { no warnings; our $banjo : Dummy; }
$applied = 0;
dummy_our(); dummy_our();
ok( $applied == 0, 51 );

sub UNIVERSAL::Stooge :ATTR(END) {};
eval {
	local $SIG{__WARN__} = sub { die @_ };
	my $groucho : Stooge;
};
my $match = $@ =~ /^Won't be able to apply END handler/; 
if($] < 5.008) {
ok(1,52 ,"# Skip, no difference between lexical handlers and normal handlers prior to 5.8");
} else {
ok( $match, 52 );
}


# The next two check for the phase invariance that Marcel spotted.
# Subject: Attribute::Handlers phase variance
# Message-Id: <54EDDB80-FD75-11D6-A18D-00039379E28A@noug.at>

my ($code_applied, $scalar_applied);
sub Scotty :ATTR(CODE,BEGIN)   { $code_applied = $_[5] }
{
no warnings 'redefine';
sub Scotty :ATTR(SCALAR,CHECK) { $scalar_applied = $_[5] }
}

sub warp_coil :Scotty {}
my $photon_torpedo :Scotty;

ok( $code_applied   eq 'BEGIN', 53, "# phase variance" );
ok( $scalar_applied eq 'CHECK', 54 );
