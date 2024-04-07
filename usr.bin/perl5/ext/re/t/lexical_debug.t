#!./perl

BEGIN {
    require Config;
    if (($Config::Config{'extensions'} !~ /\bre\b/) ){
	print "1..0 # Skip -- Perl configured without re module\n";
	exit 0;
    }
}

use strict;

# must use a BEGIN or the prototypes wont be respected meaning 
# tests could pass that shouldn't
BEGIN { require "../../t/test.pl"; }
my $out = runperl(progfile => "t/lexical_debug.pl", stderr => 1 );

print "1..12\n";

# Each pattern will produce an EXACT node with a specific string in 
# it, so we will look for that. We can't just look for the string
# alone as the string being matched against contains all of them.

ok( $out =~ /EXACT <foo>/, "Expect 'foo'"    );
ok( $out !~ /EXACT <bar>/, "No 'bar'"        );
ok( $out =~ /EXACT <baz>/, "Expect 'baz'"    );
ok( $out !~ /EXACT <bop>/, "No 'bop'"        );
ok( $out =~ /EXACT <boq>/, "Expect 'boq'"    );
ok( $out !~ /EXACT <bor>/, "No 'bor'"        );
ok( $out =~ /EXACT <fip>/, "Expect 'fip'"    );
ok( $out !~ /EXACT <fop>/, "No 'baz'"        );
ok( $out =~ /<liz>/,       "Got 'liz'"       ); # in a TRIE so no EXACT
ok( $out =~ /<zoo>/,       "Got 'zoo'"       ); # in a TRIE so no EXACT
ok( $out =~ /<zap>/,       "Got 'zap'"       ); # in a TRIE so no EXACT
ok( $out =~ /Count=9\n/,   "Count is 9")
    or diag($out);

