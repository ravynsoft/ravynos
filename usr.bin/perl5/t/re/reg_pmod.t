#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;

our @tests = (
    # /p      Pattern   PRE     MATCH   POST
    [ '/p',   "345",    "012-", "345",  "-6789"],
    [ '/$r/p',"345",    "012-", "345",  "-6789"],
    [ '(?p)', "345",    "012-", "345",  "-6789"],
    [ '(?p:)',"345",    "012-", "345",  "-6789"],
    [ '',     "(345)",  undef,  undef,  undef ],
    [ '',     "345",    undef,  undef,  undef ],
);

plan tests => 14 * @tests + 4;
my $W = "";

$SIG{__WARN__} = sub { $W.=join("",@_); };
sub _u($$) { "$_[0] is ".(defined $_[1] ? "'$_[1]'" : "undef") }

foreach my $test (@tests) {
    my ($p, $pat,$l,$m,$r) = @$test;
    my $qr = qr/$pat/;
    for my $sub (0,1) {
	my $test_name = $p eq '/p'   ? "/$pat/p"
		      : $p eq '/$r/p'? $p
		      : $p eq '(?p)' ? "/(?p)$pat/"
		      : $p eq '(?p:)'? "/(?p:$pat)/"
		      :                "/$pat/";
	$test_name = "s$test_name" if $sub;

	#
	# Cannot use if/else due to the scope invalidating ${^MATCH} and friends.
	#
	$_ = '012-345-6789';
	my $ok =
		$sub ?
			(   $p eq '/p'   ? s/$pat/abc/p
			  : $p eq '/$r/p'? s/$qr/abc/p
			  : $p eq '(?p)' ? s/(?p)$pat/abc/
			  : $p eq '(?p:)'? s/(?p:$pat)/abc/
			  :                s/$pat/abc/
			)
		     :
			(   $p eq '/p'   ? /$pat/p
			  : $p eq '/$r/p'? /$qr/p
			  : $p eq '(?p)' ? /(?p)$pat/
			  : $p eq '(?p:)'? /(?p:$pat)/
			  :                /$pat/
			);
	ok $ok, $test_name;
	SKIP: {
	    skip "/$pat/$p failed to match", 6
		unless $ok;
	    is(${^PREMATCH},  $l,_u "$test_name: ^PREMATCH",$l);
	    is(${^MATCH},     $m,_u "$test_name: ^MATCH",$m );
	    is(${^POSTMATCH}, $r,_u "$test_name: ^POSTMATCH",$r );
	    is(length ${^PREMATCH}, length $l, "$test_name: ^PREMATCH length");
	    is(length ${^MATCH},    length $m, "$test_name: ^MATCH length");
	    is(length ${^POSTMATCH},length $r, "$test_name: ^POSTMATCH length");
	}
    }
}
is($W,"","No warnings should be produced");
ok(!defined ${^MATCH}, "No /p in scope so ^MATCH is undef");

#RT 117135

{
    my $m;
    ok("a"=~ /(?p:a(?{ $m = ${^MATCH} }))/, '(?{})');
    is($m, 'a', '(?{}) ^MATCH');
}
