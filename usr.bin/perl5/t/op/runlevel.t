#!./perl

##
## Many of these tests are originally from Michael Schroeder
## <Michael.Schroeder@informatik.uni-erlangen.de>
## Adapted and expanded by Gurusamy Sarathy <gsar@activestate.com>
##

chdir 't' if -d 't';
require './test.pl';
set_up_inc('../lib');

$|=1;

run_multiple_progs('', \*DATA);

done_testing();

__END__
@a = (1, 2, 3);
{
  @a = sort { last ; } @a;
}
EXPECT
Can't "last" outside a loop block at - line 3.
########
package TEST;
 
sub TIESCALAR {
  my $foo;
  return bless \$foo;
}
sub FETCH {
  eval 'die("test")';
  print "still in fetch\n";
  return ">$@<";
}
package main;
 
tie $bar, TEST;
print "- $bar\n";
EXPECT
still in fetch
- >test at (eval 1) line 1.
<
########
package TEST;
 
sub TIESCALAR {
  my $foo;
  eval('die("foo\n")');
  print "after eval\n";
  return bless \$foo;
}
sub FETCH {
  return "ZZZ";
}
 
package main;
 
tie $bar, TEST;
print "- $bar\n";
print "OK\n";
EXPECT
after eval
- ZZZ
OK
########
package TEST;
 
sub TIEHANDLE {
  my $foo;
  return bless \$foo;
}
sub PRINT {
print STDERR "PRINT CALLED\n";
(split(/./, 'x'x10000))[0];
eval('die("test\n")');
}
 
package main;
 
open FH, ">&STDOUT";
tie *FH, TEST;
print FH "OK\n";
print STDERR "DONE\n";
EXPECT
PRINT CALLED
DONE
########
sub warnhook {
  print "WARNHOOK\n";
  eval('die("foooo\n")');
}
$SIG{'__WARN__'} = 'warnhook';
warn("dfsds\n");
print "END\n";
EXPECT
WARNHOOK
END
########
package TEST;
 
use overload
     "\"\""   =>  \&str
;
 
sub str {
  eval('die("test\n")');
  return "STR";
}
 
package main;
 
$bar = bless {}, TEST;
print "$bar\n";
print "OK\n";
EXPECT
STR
OK
########
sub foo {
  $a <=> $b unless eval('$a == 0 ? bless undef : ($a <=> $b)');
}
@a = (3, 2, 0, 1);
@a = sort foo @a;
print join(', ', @a)."\n";
EXPECT
0, 1, 2, 3
########
sub foo {
  goto bar if $a == 0 || $b == 0;
  $a <=> $b;
}
@a = (3, 2, 0, 1);
@a = sort foo @a;
print join(', ', @a)."\n";
exit;
bar:
print "bar reached\n";
EXPECT
Can't "goto" out of a pseudo block at - line 2.
########
%seen = ();
sub sortfn {
  (split(/./, 'x'x10000))[0];
  my (@y) = ( 4, 6, 5);
  @y = sort { $a <=> $b } @y;
  my $t = "sortfn ".join(', ', @y)."\n";
  print $t if ($seen{$t}++ == 0);
  return $_[0] <=> $_[1];
}
@x = ( 3, 2, 1 );
@x = sort { &sortfn($a, $b) } @x;
print "---- ".join(', ', @x)."\n";
EXPECT
sortfn 4, 5, 6
---- 1, 2, 3
########
@a = (3, 2, 1);
@a = sort { eval('die("no way")') ,  $a <=> $b} @a;
print join(", ", @a)."\n";
EXPECT
1, 2, 3
########
@a = (1, 2, 3);
foo:
{
  @a = sort { last foo; } @a;
}
EXPECT
Label not found for "last foo" at - line 4.
########
package TEST;
 
sub TIESCALAR {
  my $foo;
  return bless \$foo;
}
sub FETCH {
  next;
  return "ZZZ";
}
sub STORE {
}
 
package main;
 
tie $bar, TEST;
{
  print "- $bar\n";
}
print "OK\n";
EXPECT
Can't "next" outside a loop block at - line 8.
########
package TEST;
 
sub TIESCALAR {
  my $foo;
  return bless \$foo;
}
sub FETCH {
  goto bbb;
  return "ZZZ";
}
 
package main;
 
tie $bar, TEST;
print "- $bar\n";
exit;
bbb:
print "bbb\n";
EXPECT
Can't find label bbb at - line 8.
########
sub foo {
  $a <=> $b unless eval('$a == 0 ? die("foo\n") : ($a <=> $b)');
}
@a = (3, 2, 0, 1);
@a = sort foo @a;
print join(', ', @a)."\n";
EXPECT
0, 1, 2, 3
########
package TEST;
sub TIESCALAR {
  my $foo;
  return bless \$foo;
}
sub FETCH {
  return "fetch";
}
sub STORE {
(split(/./, 'x'x10000))[0];
}
package main;
tie $bar, TEST;
$bar = "x";
########
package TEST;
sub TIESCALAR {
  my $foo;
  next;
  return bless \$foo;
}
package main;
{
tie $bar, TEST;
}
EXPECT
Can't "next" outside a loop block at - line 4.
########
@a = (1, 2, 3);
foo:
{
  @a = sort { exit(0) } @a;
}
END { print "foobar\n" }
EXPECT
foobar
########
$SIG{__DIE__} = sub {
    print "In DIE\n";
    $i = 0;
    while (($p,$f,$l,$s) = caller(++$i)) {
        print "$p|$f|$l|$s\n";
    }
};
eval { die };
&{sub { eval 'die' }}();
sub foo { eval { die } } foo();
{package rmb; sub{ eval{die} } ->() };	# check __ANON__ knows package	
EXPECT
In DIE
main|-|8|(eval)
In DIE
main|-|9|(eval)
main|-|9|main::__ANON__
In DIE
main|-|10|(eval)
main|-|10|main::foo
In DIE
rmb|-|11|(eval)
rmb|-|11|rmb::__ANON__
########
package TEST;
 
sub TIEARRAY {
  return bless [qw(foo fee fie foe)], $_[0];
}
sub FETCH {
  my ($s,$i) = @_;
  if ($i) {
    goto bbb;
  }
bbb:
  return $s->[$i];
}
 
package main;
tie my @bar, 'TEST';
print join('|', @bar[0..3]), "\n"; 
EXPECT
foo|fee|fie|foe
########
package TH;
sub TIEHASH { bless {}, TH }
sub STORE { eval { print "@_[1,2]\n" }; die "bar\n" }
tie %h, TH;
eval { $h{A} = 1; print "never\n"; };
print $@;
eval { $h{B} = 2; };
print $@;
EXPECT
A 1
bar
B 2
bar
########
sub n { 0 }
sub f { my $x = shift; d(); }
f(n());
f();

sub d {
    my $i = 0; my @a;
    while (do { { package DB; @a = caller($i++) } } ) {
        @a = @DB::args;
        for (@a) { print "$_\n"; $_ = '' }
    }
}
EXPECT
0
########
sub TIEHANDLE { bless {} }
sub PRINT { next }

tie *STDERR, '';
{ map ++$_, 1 }

EXPECT
Can't "next" outside a loop block at - line 2.
########
sub TIEHANDLE { bless {} }
sub PRINT { print "[TIE] $_[1]" }

tie *STDERR, '';
die "DIE\n";

EXPECT
[TIE] DIE
########
sub TIEHANDLE { bless {} }
sub PRINT { 
    (split(/./, 'x'x10000))[0];
    eval('die("test\n")');
    warn "[TIE] $_[1]";
}
open OLDERR, '>&STDERR';
tie *STDERR, '';

use warnings FATAL => qw(uninitialized);
print undef;

EXPECT
[TIE] Use of uninitialized value in print at - line 11.
