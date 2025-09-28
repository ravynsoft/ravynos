#!./perl -w

BEGIN {
    $SIG{__WARN__} = sub { die "Dying on warning: ", @_ };
    chdir 't' if -d 't';
    require './test.pl';
    skip_all_if_miniperl(
	"no dynamic loading on miniperl, no Tie::Hash::NamedCapture"
    );
}

plan(tests => 2);

use strict;

# Test for bug [perl #27839]
{
    my $x;
    sub f {
	"abc" =~ /(.)./;
	$x = "@+";
	return @+;
    };
    "pqrstuvwxyz" =~ /..(....)../; # prime @+ etc in this scope
    my @y = f();
    is $x, "@y", "return a magic array ($x) vs (@y)";

    sub f2 {
	"abc" =~ /(?<foo>.)./;
	my @h =  %+;
	$x = "@h";
	return %+;
    };
    @y = f();
    is $x, "@y", "return a magic hash ($x) vs (@y)";
}

