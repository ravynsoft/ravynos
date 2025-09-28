#!./perl

BEGIN {
    @INC = qw(. ../lib);
    chdir 't' if -d 't';
}

print "1..14\n";

$blurfl = 123;
$foo = 3;

package xyz;

sub new {bless [];}

$bar = 4;

{
    package ABC;
    no warnings qw(syntax deprecated);
    $blurfl = 5;
    $main'a = $'b;
}
{
    no warnings qw(syntax deprecated);
    $ABC'dyick = 6;
}

$xyz = 2;

$main = join(':', sort(keys %main::));
$xyz = join(':', sort(keys %xyz::));
$ABC = join(':', sort(keys %ABC::));

if ('a' lt 'A') {
    print $xyz eq 'bar:main:new:xyz:ABC' ? "ok 1\n" : "not ok 1 '$xyz'\n";
} else {
    print $xyz eq 'ABC:BEGIN:bar:main:new:xyz' ? "ok 1\n" : "not ok 1 '$xyz'\n";
}    
print $ABC eq 'BEGIN:blurfl:dyick' ? "ok 2\n" : "not ok 2 '$ABC'\n";
{
    no warnings qw(syntax deprecated);
    print $main'blurfl == 123 ? "ok 3\n" : "not ok 3\n";
}

package ABC;

print $blurfl == 5 ? "ok 4\n" : "not ok 4\n";
eval 'print $blurfl == 5 ? "ok 5\n" : "not ok 5\n";';
eval 'package main; print $blurfl == 123 ? "ok 6\n" : "not ok 6\n";';
print $blurfl == 5 ? "ok 7\n" : "not ok 7\n";

package main;

sub c { caller(0) }

sub foo {
   my $s = shift;
   if ($s) {
	package PQR;
	main::c();
   }
}

print((foo(1))[0] eq 'PQR' ? "ok 8\n" : "not ok 8\n");

my $Q = xyz->new();
undef %xyz::;
eval { $a = *xyz::new{PACKAGE}; };
print $a eq "__ANON__" ? "ok 9\n" : "not ok 9 # '$a'\n";

eval { $Q->param; };
print $@ =~ /^Can't use anonymous symbol table for method lookup/ ?
  "ok 10\n" : "not ok 10 # '$@'\n";

print "$Q" =~ /^__ANON__=/ ? "ok 11\n" : "not ok 11 # '$Q'\n";

print ref $Q eq "__ANON__" ? "ok 12\n" : "not ok 12 # '$Q'\n";

package bug32562;

print       __PACKAGE__  eq 'bug32562' ? "ok 13\n" : "not ok 13\n";
print eval '__PACKAGE__' eq 'bug32562' ? "ok 14\n" : "not ok 14\n";

