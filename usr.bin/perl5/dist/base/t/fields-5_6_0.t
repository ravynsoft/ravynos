# The fields.pm and base.pm regression tests from 5.6.0

# We skip this on 5.9.0 and up since pseudohashes were removed and a lot
# of it won't work.
if( $] >= 5.009 ) { 
    print "1..0 # skip pseudo-hashes removed in 5.9.0\n";
    exit;
}

use strict;
our $Total_tests;

my $test_num = 1;
BEGIN { $| = 1; $^W = 1; }
print "1..$Total_tests\n";
use fields;
use base;
print "ok $test_num\n";
$test_num++;

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):
sub ok {
    my($test, $name) = @_;
    print "not " unless $test;
    print "ok $test_num";
    print " - $name" if defined $name;
    print "\n";
    $test_num++;
}

sub eqarray  {
    my($a1, $a2) = @_;
    return 0 unless @$a1 == @$a2;
    my $ok = 1;
    for (0..$#{$a1}) { 
        unless($a1->[$_] eq $a2->[$_]) {
            $ok = 0;
            last;
        }
    }
    return $ok;
}

# Change this to your # of ok() calls + 1
BEGIN { $Total_tests = 14 }


my $w;

BEGIN {
   $^W = 1;

   $SIG{__WARN__} = sub {
       if ($_[0] =~ /^Hides field 'b1' in base class/) {
           $w++;
           return;
       }
       print $_[0];
   };
}

use strict;
our $DEBUG;

package B1;
use fields qw(b1 b2 b3);

package B2;
use fields '_b1';
use fields qw(b1 _b2 b2);

sub new { bless [], shift }

package D1;
use base 'B1';
use fields qw(d1 d2 d3);

package D2;
use base 'B1';
use fields qw(_d1 _d2);
use fields qw(d1 d2);

package D3;
use base 'B2';
use fields qw(b1 d1 _b1 _d1);  # hide b1

package D4;
use base 'D3';
use fields qw(_d3 d3);

package M;
sub m {}

package D5;
use base qw(M B2);

package Foo::Bar;
use base 'B1';

package Foo::Bar::Baz;
use base 'Foo::Bar';
use fields qw(foo bar baz);

# Test repeatability for when modules get reloaded.
package B1;
use fields qw(b1 b2 b3);

package D3;
use base 'B2';
use fields qw(b1 d1 _b1 _d1);  # hide b1

package main;

sub fstr {
   my $h = shift;
   my @tmp;
   for my $k (sort {$h->{$a} <=> $h->{$b}} keys %$h) {
	my $v = $h->{$k};
        push(@tmp, "$k:$v");
   }
   my $str = join(",", @tmp);
   print "$h => $str\n" if $DEBUG;
   $str;
}

my %expect;
BEGIN {
    %expect = (
               B1 => "b1:1,b2:2,b3:3",
               B2 => "_b1:1,b1:2,_b2:3,b2:4",
               D1 => "b1:1,b2:2,b3:3,d1:4,d2:5,d3:6",
               D2 => "b1:1,b2:2,b3:3,_d1:4,_d2:5,d1:6,d2:7",
               D3 => "b2:4,b1:5,d1:6,_b1:7,_d1:8",
               D4 => "b2:4,b1:5,d1:6,_d3:9,d3:10",
               D5 => "b1:2,b2:4",
               'Foo::Bar::Baz' => 'b1:1,b2:2,b3:3,foo:4,bar:5,baz:6',
              );
    $Total_tests += int(keys %expect);
}
my $testno = 0;
while (my($class, $exp) = each %expect) {
   no strict 'refs';
   my $fstr = fstr(\%{$class."::FIELDS"});
   ok( $fstr eq $exp, "'$fstr' eq '$exp'" );
}

# Did we get the appropriate amount of warnings?
ok( $w == 1 );

# A simple object creation and AVHV attribute access test
my B2 $obj1 = D3->new;
$obj1->{b1} = "B2";
my D3 $obj2 = $obj1;
$obj2->{b1} = "D3";

ok( $obj1->[2] eq "B2" && $obj1->[5] eq "D3" );

# We should get compile time failures field name typos
eval q{ my D3 $obj3 = $obj2; $obj3->{notthere} = "" };
ok( $@ && $@ =~ /^No such pseudo-hash field "notthere"/,
                                 'compile error -- field name typos' );


# Slices
if( $] >= 5.006 ) {
    @$obj1{"_b1", "b1"} = (17, 29);
    ok( "@$obj1[1,2]" eq "17 29" );

    @$obj1[1,2] = (44,28);
    ok( "@$obj1{'b1','_b1','b1'}" eq "28 44 28" );
}
else {
    ok( 1, 'test skipped for perl < 5.6.0' );
    ok( 1, 'test skipped for perl < 5.6.0' );
}

my $ph = fields::phash(a => 1, b => 2, c => 3);
ok( fstr($ph) eq 'a:1,b:2,c:3' );

$ph = fields::phash([qw/a b c/], [1, 2, 3]);
ok( fstr($ph) eq 'a:1,b:2,c:3' );

# The way exists() works with pseudohashes changed from 5.005 to 5.6
$ph = fields::phash([qw/a b c/], [1]);
if( $] > 5.006 ) {
    ok( !( exists $ph->{b} or exists $ph->{c} or !exists $ph->{a} ) );
}
else {
    ok( !( defined $ph->{b} or defined $ph->{c} or !defined $ph->{a} ) );
}

eval { $ph = fields::phash("odd") };
ok( $@ && $@ =~ /^Odd number of/ );


# check if fields autovivify
if ( $] > 5.006 ) {
    package Foo;
    use fields qw(foo bar);
    sub new { bless [], $_[0]; }

    package main;
    my Foo $a = Foo->new();
    $a->{foo} = ['a', 'ok', 'c'];
    $a->{bar} = { A => 'ok' };
    ok( $a->{foo}[1]   eq 'ok' );
    ok( $a->{bar}->{A} eq 'ok' );
}
else {
    ok( 1, 'test skipped for perl < 5.6.0' );
    ok( 1, 'test skipped for perl < 5.6.0' );
}

# check if fields autovivify
{
    package Bar;
    use fields qw(foo bar);
    sub new { return fields::new($_[0]) }

    package main;
    my Bar $a = Bar::->new();
    $a->{foo} = ['a', 'ok', 'c'];
    $a->{bar} = { A => 'ok' };
    ok( $a->{foo}[1]   eq 'ok' );
    ok( $a->{bar}->{A} eq 'ok' );
}
