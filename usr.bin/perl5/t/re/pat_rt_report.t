#!./perl
#
# This is a home for regular expression tests that don't fit into
# the format supported by re/regexp.t.  If you want to add a test
# that does fit that format, add it to re/re_tests, not here.

sub run_tests;

$| = 1;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( '../lib', '.' );
    skip_all_if_miniperl("miniperl can't load Tie::Hash::NamedCapture, need for %+ and %-");
}

use strict;
use warnings;
use 5.010;
use Config;

plan tests => 2514;  # Update this when adding/deleting tests.

run_tests() unless caller;

#
# Tests start here.
#
sub run_tests {

    like("A \x{263a} B z C", qr/A . B (??{ "z" }) C/,
	 "Match UTF-8 char in presence of (??{ }); Bug 20000731.001 (#3600)");

    {
        no warnings 'uninitialized';
        ok(undef =~ /^([^\/]*)(.*)$/, "Used to cause a SEGV; Bug 20001021.005 (#4492)");
    }

    {
        my $message = 'bug id 20001008.001 (#4407)';

        my @x = ("stra\337e 138", "stra\337e 138");
        for (@x) {
            ok(s/(\d+)\s*([\w\-]+)/$1 . uc $2/e, $message);
            ok(my ($latin) = /^(.+)(?:\s+\d)/, $message);
            is($latin, "stra\337e", $message);
	    ok($latin =~ s/stra\337e/straße/, $message);
            #
            # Previous code follows, but outcommented - there were no tests.
            #
            # $latin =~ s/stra\337e/straße/; # \303\237 after the 2nd a
            # use utf8; # needed for the raw UTF-8
            # $latin =~ s!(s)tr(?:aß|s+e)!$1tr.!; # \303\237 after the a
        }
    }

    {
        # Fist half of the bug.
        my $message = 'HEBREW ACCENT QADMA matched by .*; Bug 20001028.003 (#4536)';
        my $X = chr (1448);
        ok(my ($Y) = $X =~ /(.*)/, $message);
        is($Y, v1448, $message);
        is(length $Y, 1, $message);

        # Second half of the bug.
        $message = 'HEBREW ACCENT QADMA in replacement; Bug 20001028.003 (#4536)';
        $X = '';
        $X =~ s/^/chr(1488)/e;
        is(length $X, 1, $message);
        is(ord $X, 1488, $message);
    }

    {   
        my $message = 'Repeated s///; Bug 20001108.001 (#4631)';
        my $X = "Szab\x{f3},Bal\x{e1}zs";
        my $Y = $X;
        $Y =~ s/(B)/$1/ for 0 .. 3;
        is($Y, $X, $message);
        is($X, "Szab\x{f3},Bal\x{e1}zs", $message);
    }

    {
        my $message = 's/// on UTF-8 string; Bug 20000517.001 (#3253)';
        my $x = "\x{100}A";
        $x =~ s/A/B/;
        is($x, "\x{100}B", $message);
        is(length $x, 2, $message);
    }

    {
        # The original bug report had 'no utf8' here but that was irrelevant.

        my $message = "Don't dump core; Bug 20010306.008 (#5982)";
        my $a = "a\x{1234}";
        like($a, qr/\w/, $message);  # used to core dump.
    }

    {
        my $message = '/g in scalar context; Bug 20010410.006 (#6796)';
        for my $rx ('/(.*?)\{(.*?)\}/csg',
		    '/(.*?)\{(.*?)\}/cg',
		    '/(.*?)\{(.*?)\}/sg',
		    '/(.*?)\{(.*?)\}/g',
		    '/(.+?)\{(.+?)\}/csg',) {
            my $i = 0;
            my $input = "a{b}c{d}";
            eval <<"            --";
                while (eval \$input =~ $rx) {
                    \$i ++;
                }
            --
            is($i, 2, $message);
        }
    }

    {
        # Amazingly vertical tabulator is the same in ASCII and EBCDIC.
        for ("\n", "\t", "\014", "\r") {
            unlike($_, qr/[[:print:]]/, sprintf "\\%03o not in [[:print:]]; Bug 20010619.003 (#7131)", ord $_);
        }
        for (" ") {
            like($_, qr/[[:print:]]/, "'$_' in [[:print:]]; Bug 20010619.003 (#7131)");
        }
    }

    {
        # [ID 20010814.004 (#7526)] pos() doesn't work when using =~m// in list context

        $_ = "ababacadaea";
        my $a = join ":", /b./gc;
        my $b = join ":", /a./gc;
        my $c = pos;
        is("$a $b $c", 'ba:ba ad:ae 10', "pos() works with () = m//; Bug 20010814.004 (#7526)");
    }

    {
        # [ID 20010407.006 (#6767)] matching utf8 return values from
        # functions does not work

        my $message = 'UTF-8 return values from functions; Bug 20010407.006 (#6767)';
        package ID_20010407_006;
        sub x {"a\x{1234}"}
        my $x = x;
        my $y;
      ::ok($x =~ /(..)/, $message);
        $y = $1;
      ::ok(length ($y) == 2 && $y eq $x, $message);
      ::ok(x =~ /(..)/, $message);
        $y = $1;
      ::ok(length ($y) == 2 && $y eq $x, $message);
    }

    {
        # High bit bug -- japhy
        my $x = "ab\200d";
        ok $x =~ /.*?\200/, "High bit fine";
    }

    {
        my $message = 'UTF-8 hash keys and /$/';
        # http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters
        #                                         /2002-01/msg01327.html

        my $u = "a\x{100}";
        my $v = substr ($u, 0, 1);
        my $w = substr ($u, 1, 1);
        my %u = ($u => $u, $v => $v, $w => $w);
        for (keys %u) {
            my $m1 =            /^\w*$/ ? 1 : 0;
            my $m2 = $u {$_} =~ /^\w*$/ ? 1 : 0;
            is($m1, $m2, $message);
        }
    }

    {
        my $message = "s///eg [change 13f46d054db22cf4]; Bug 20020124.005 (#8335)";

        for my $char ("a", "\x{df}", "\x{100}") {
            my $x = "$char b $char";
            $x =~ s{($char)}{
                  "c" =~ /c/;
                  "x";
            }ge;
            is(substr ($x, 0, 1), substr ($x, -1, 1), $message);
        }
    }

    {
        my $message = "Correct pmop flags checked when empty pattern; Bug 20020412.005 (#8935)";

        # Requires reuse of last successful pattern.
        my $num = 123;
        $num =~ /\d/;
        for (0 .. 1) {
            my $match = m?? + 0;
            ok($match != $_, $message)
                or diag(sprintf "'match one' %s on %s iteration" =>
			$match ? 'succeeded' : 'failed',
			$_     ? 'second'    : 'first');
        }
        $num =~ /(\d)/;
        my $result = join "" => $num =~ //g;
        is($result, $num, $message);
    }

    {
        my $message = 'UTF-8 regex matches above 32k; Bug 20020630.002 (#10013)';
        for (['byte', "\x{ff}"], ['utf8', "\x{1ff}"]) {
            my ($type, $char) = @$_;
            for my $len (32000, 32768, 33000) {
                my  $s = $char . "f" x $len;
                my  $r = $s =~ /$char([f]*)/gc;
                ok($r, $message) or diag("<$type x $len>");
                ok(!$r || pos ($s) == $len + 1, $message)
		    or diag("<$type x $len>; pos = @{[pos $s]}");
            }
        }
    }

    {
        my $s = "\x{100}" x 5;
        my $ok = $s =~ /(\x{100}{4})/;
        my ($ord, $len) = (ord $1, length $1);
        ok $ok && $ord == 0x100 && $len == 4, "No panic: end_shift [change 0e933229fa758625]";
    }

    {
        my $message = 'UTF-8 matching; Bug 15397';
        like("\x{100}", qr/\x{100}/, $message);
        like("\x{100}", qr/(\x{100})/, $message);
        like("\x{100}", qr/(\x{100}){1}/, $message);
        like("\x{100}\x{100}", qr/(\x{100}){2}/, $message);
        like("\x{100}\x{100}", qr/(\x{100})(\x{100})/, $message);
    }

    {
        my $message = 'Neither ()* nor ()*? sets $1 when matched 0 times; Bug 7471';
        local $_       = 'CD';
        ok(/(AB)*?CD/ && !defined $1, $message);
        ok(/(AB)*CD/  && !defined $1, $message);
    }

    {
        my $message = "Caching shouldn't prevent match; Bug 3547";
        my $pattern = "^(b+?|a){1,2}c";
        ok("bac"    =~ /$pattern/ && $1 eq 'a', $message);
        ok("bbac"   =~ /$pattern/ && $1 eq 'a', $message);
        ok("bbbac"  =~ /$pattern/ && $1 eq 'a', $message);
        ok("bbbbac" =~ /$pattern/ && $1 eq 'a', $message);
    }

    {
        ok("\x{100}" =~ /(.)/, '$1 should keep UTF-8 ness; Bug 18232');
        is($1, "\x{100}",  '$1 is UTF-8; Bug 18232');
        { 'a' =~ /./; }
        is($1, "\x{100}",  '$1 is still UTF-8; Bug 18232');
        isnt($1, "\xC4\x80", '$1 is not non-UTF-8; Bug 18232');
    }

    {
        my $message = "Optimizer doesn't prematurely reject match; Bug 19767";
        use utf8;

        my $attr = 'Name-1';
        my $NormalChar      = qr /[\p{IsDigit}\p{IsLower}\p{IsUpper}]/;
        my $NormalWord      = qr /${NormalChar}+?/;
        my $PredNameHyphen  = qr /^${NormalWord}(\-${NormalWord})*?$/;

        $attr =~ /^$/;
        like($attr, $PredNameHyphen, $message);  # Original test.

        "a" =~ m/[b]/;
        like("0", qr/\p{N}+\z/, $message);         # Variant.
    }

    {
        my $message = "(??{ }) doesn't return stale values; Bug 20683";
        our $p = 1;
        foreach (1, 2, 3, 4) {
            $p ++ if /(??{ $p })/
        }
        is($p, 5, $message);

        {
            package P;
            $a = 1;
            sub TIESCALAR {bless []}
            sub FETCH     {$a ++}
        }
        tie $p, "P";
        foreach (1, 2, 3, 4) {
            /(??{ $p })/
        }
        is($p, 5, $message);
    }

    {
        # Subject: Odd regexp behavior
        # From: Markus Kuhn <Markus.Kuhn@cl.cam.ac.uk>
        # Date: Wed, 26 Feb 2003 16:53:12 +0000
        # Message-Id: <E18o4nw-0008Ly-00@wisbech.cl.cam.ac.uk>
        # To: perl-unicode@perl.org

        my $message = 'Markus Kuhn 2003-02-26';
    
        my $x = "\x{2019}\nk";
        ok($x =~ s/(\S)\n(\S)/$1 $2/sg, $message);
        is($x, "\x{2019} k", $message);

        $x = "b\nk";
        ok($x =~ s/(\S)\n(\S)/$1 $2/sg, $message);
        is($x, "b k", $message);

        like("\x{2019}", qr/\S/, $message);
    }

    {
        my $message = "(??{ .. }) in split doesn't corrupt its stack; Bug 21411";
        our $i;
        is('-1-3-5-', join('', split /((??{$i++}))/, '-1-3-5-'), $message);
        no warnings 'syntax';
        @_ = split /(?{'WOW'})/, 'abc';
        local $" = "|";
        is("@_", "a|b|c", $message);
    }

    is(join('-', split /(?{ split "" })/, "abc"), 'a-b-c', 'nested split');

    {
        $_ = "code:   'x' { '...' }\n"; study;
        my @x; push @x, $& while m/'[^\']*'/gx;
        local $" = ":";
        is("@x", "'x':'...'", "Parse::RecDescent triggered infinite loop; Bug 17757");
    }

    {
        sub func ($) {
            ok("a\nb" !~ /^b/,  "Propagated modifier; $_[0]; Bug 22354");
            ok("a\nb" =~ /^b/m, "Propagated modifier; $_[0] - with /m; Bug 22354");
        }
        func "standalone";
        $_ = "x"; s/x/func "in subst"/e;
        $_ = "x"; s/x/func "in multiline subst"/em;
        $_ = "x"; /x(?{func "in regexp"})/;
        $_ = "x"; /x(?{func "in multiline regexp"})/m;
    }

    {
        $_    = "abcdef\n";
        my @x = m/./g;
        is("abcde", $`, 'Global match sets $`; Bug 19049');
    }

    {
        # [perl #23769] Unicode regex broken on simple example
        # regrepeat() didn't handle UTF-8 EXACT case right.

        my $Mess       = 'regrepeat() handles UTF-8 EXACT case right';
        my $message = "$Mess; Bug 23769";

        my $s = "\x{a0}\x{a0}\x{a0}\x{100}"; chop $s;

        like($s, qr/\x{a0}/, $message);
        like($s, qr/\x{a0}+/, $message);
        like($s, qr/\x{a0}\x{a0}/, $message);

        $message = "$Mess (easy variant); Bug 23769";
        ok("aaa\x{100}" =~ /(a+)/, $message);
        is($1, "aaa", $message);

        $message = "$Mess (easy invariant); Bug 23769";
        ok("aaa\x{100}     " =~ /(a+?)/, $message);
        is($1, "a", $message);

        $message = "$Mess (regrepeat variant); Bug 23769";
        ok("\xa0\xa0\xa0\x{100}    " =~ /(\xa0+?)/, $message);
        is($1, "\xa0", $message);

        $message = "$Mess (regrepeat invariant); Bug 23769";
        ok("\xa0\xa0\xa0\x{100}" =~ /(\xa0+)/, $message);
        is($1, "\xa0\xa0\xa0", $message);

        $message = "$Mess (hard variant); Bug 23769";
        ok("\xa0\xa1\xa0\xa1\xa0\xa1\x{100}" =~ /((?:\xa0\xa1)+?)/, $message);
        is($1, "\xa0\xa1", $message);

        $message = "$Mess (hard invariant); Bug 23769";
        ok("ababab\x{100}  " =~ /((?:ab)+)/, $message);
        is($1, 'ababab', $message);

        ok("\xa0\xa1\xa0\xa1\xa0\xa1\x{100}" =~ /((?:\xa0\xa1)+)/, $message);
        is($1, "\xa0\xa1\xa0\xa1\xa0\xa1", $message);

        ok("ababab\x{100}  " =~ /((?:ab)+?)/, $message);
        is($1, "ab", $message);

        $message = "Don't match first byte of UTF-8 representation; Bug 23769";
        unlike("\xc4\xc4\xc4", qr/(\x{100}+)/, $message);
        unlike("\xc4\xc4\xc4", qr/(\x{100}+?)/, $message);
        unlike("\xc4\xc4\xc4", qr/(\x{100}++)/, $message);
    }

    {
        # perl panic: pp_match start/end pointers

        is(eval {my ($x, $y) = "bca" =~ /^(?=.*(a)).*(bc)/; "$x-$y"}, "a-bc",
	   'Captures can move backwards in string; Bug 25269');
    }

    {
        # \cA not recognized in character classes
        like("a\cAb", qr/\cA/, '\cA in pattern; Bug 27940');
        like("a\cAb", qr/[\cA]/, '\cA in character class; Bug 27940');
        like("a\cAb", qr/[\cA-\cB]/, '\cA in character class range; Bug 27940');
        like("abc", qr/[^\cA-\cB]/, '\cA in negated character class range; Bug 27940');
        like("a\cBb", qr/[\cA-\cC]/, '\cB in character class range; Bug 27940');
        like("a\cCbc", qr/[^\cA-\cB]/, '\cC in negated character class range; Bug 27940');
        like("a\cAb", qr/(??{"\cA"})/, '\cA in ??{} pattern; Bug 27940');
        unlike("ab", qr/a\cIb/x, '\cI in pattern; Bug 27940');
    }

    {
        # perl #28532: optional zero-width match at end of string is ignored

        ok("abc" =~ /^abc(\z)?/ && defined($1),
           'Optional zero-width match at end of string; Bug 28532');
        ok("abc" =~ /^abc(\z)??/ && !defined($1),
           'Optional zero-width match at end of string; Bug 28532');
    }

    {
        my $utf8 = "\xe9\x{100}"; chop $utf8;
        my $latin1 = "\xe9";

        like($utf8, qr/\xe9/i, "utf8/latin; Bug 36207");
        like($utf8, qr/$latin1/i, "utf8/latin runtime; Bug 36207");
        like($utf8, qr/(abc|\xe9)/i, "utf8/latin trie; Bug 36207");
        like($utf8, qr/(abc|$latin1)/i, "utf8/latin trie runtime; Bug 36207");

        like("\xe9", qr/$utf8/i, "latin/utf8; Bug 36207");
        like("\xe9", qr/(abc|$utf8)/i, "latin/utf8 trie; Bug 36207");
        like($latin1, qr/$utf8/i, "latin/utf8 runtime; Bug 36207");
        like($latin1, qr/(abc|$utf8)/i, "latin/utf8 trie runtime; Bug 36207");
    }

    {
        my $s = "abcd";
        $s =~ /(..)(..)/g;
        $s = $1;
        $s = $2;
        is($2, 'cd',
	   "Assigning to original string does not corrupt match vars; Bug 37038");
    }

    {
        {
            package wooosh;
            sub gloople {"!"}
        }
        my $aeek = bless {} => 'wooosh';
        is(do {$aeek -> gloople () =~ /(.)/g}, 1,
	   "//g match against return value of sub [change e26a497577f3ce7b]");

        sub gloople {"!"}
        is(do{gloople () =~ /(.)/g}, 1,
	   "change e26a497577f3ce7b didn't affect sub calls for some reason");
    }

    {
        # [perl #78680]
        # See changes 26925-26928, which reverted change 26410
        {
            package lv;
            our $var = "abc";
            sub variable : lvalue {$var}
        }
        my $o = bless [] => 'lv';
        my $f = "";
        my $r = eval {
            for (1 .. 2) {
                $f .= $1 if $o -> variable =~ /(.)/g;
            }
            1;
        };
        if ($r) {
            is($f, "ab", "pos() retained between calls");
        }
        else {
            ok 0, "Code failed: $@";
        }

        our $var = "abc";
        sub variable : lvalue {$var}
        my $g = "";
        my $s = eval {
            for (1 .. 2) {
                $g .= $1 if variable =~ /(.)/g;
            }
            1;
        };
        if ($s) {
            is($g, "ab", "pos() retained between calls");
        }
        else {
            ok 0, "Code failed: $@";
        }
    }

  SKIP:
    {
        skip "In EBCDIC and unclear what would trigger this bug there" if $::IS_EBCDIC;
        fresh_perl_like(
            'no warnings "utf8";
             $_ = pack "U0C2", 0xa2, 0xf8;  # Ill-formed UTF-8
             my $ret = 0;
             do {!($ret = s/[a\0]+//g)}',
             qr/Malformed UTF-8/,
             {}, "Ill-formed UTF-8 doesn't match NUL in class; Bug 37836");
    }

    {
        # chr(65535) should be allowed in regexes

        no warnings 'utf8'; # To allow non-characters
        my ($c, $r, $s);

        $c = chr 0xffff;
        $c =~ s/$c//g;
        is($c, "", "U+FFFF, parsed as atom; Bug 38293");

        $c = chr 0xffff;
        $r = "\\$c";
        $c =~ s/$r//g;
        is($c, "", "U+FFFF backslashed, parsed as atom; Bug 38293");

        $c = chr 0xffff;
        $c =~ s/[$c]//g;
        is($c, "", "U+FFFF, parsed in class; Bug 38293");

        $c = chr 0xffff;
        $r = "[\\$c]";
        $c =~ s/$r//g;
        is($c, "", "U+FFFF backslashed, parsed in class; Bug 38293");

        $s = "A\x{ffff}B";
        $s =~ s/\x{ffff}//i;
        is($s, "AB", "U+FFFF, EXACTF; Bug 38293");

        $s = "\x{ffff}A";
        $s =~ s/\bA//;
        is($s, "\x{ffff}", "U+FFFF, BOUND; Bug 38293");

        $s = "\x{ffff}!";
        $s =~ s/\B!//;
        is($s, "\x{ffff}", "U+FFFF, NBOUND; Bug 38293");
    }

    {
        
        # The printing characters
        my @chars = ("A" .. "Z");
        my $delim = ",";
        my $size = 32771 - 4;
        my $str = '';

        # Create some random junk. Inefficient, but it works.
        for (my $i = 0; $i < $size; $ i++) {
            $str .= $chars [rand @chars];
        }

        $str .= ($delim x 4);
        my $res;
        my $matched;
        ok($str =~ s/^(.*?)${delim}{4}//s, "Pattern matches; Bug 39583");
        is($str, "", "Empty string; Bug 39583");
        ok(defined $1 && length ($1) == $size, '$1 is correct size; Bug 39583');
    }

    {
        like("\0-A", qr/\c@-A/, '@- should not be interpolated in a pattern; Bug 27940');
        like("\0\0A", qr/\c@+A/, '@+ should not be interpolated in a pattern; Bug 27940');
        like("X\@-A", qr/X@-A/, '@- should not be interpolated in a pattern; Bug 27940');
        like("X\@\@A", qr/X@+A/, '@+ should not be interpolated in a pattern; Bug 27940');

        like("X\0A", qr/X\c@?A/,  '\c@?; Bug 27940');
        like("X\0A", qr/X\c@*A/,  '\c@*; Bug 27940');
        like("X\0A", qr/X\c@(A)/, '\c@(; Bug 27940');
        like("X\0A", qr/X(\c@)A/, '\c@); Bug 27940');
        like("X\0A", qr/X\c@|ZA/, '\c@|; Bug 27940');

        like("X\@A", qr/X@?A/,  '@?; Bug 27940');
        like("X\@A", qr/X@*A/,  '@*; Bug 27940');
        like("X\@A", qr/X@(A)/, '@(; Bug 27940');
        like("X\@A", qr/X(@)A/, '@); Bug 27940');
        like("X\@A", qr/X@|ZA/, '@|; Bug 27940');

        local $" = ','; # non-whitespace and non-RE-specific
        like('abc', qr/(.)(.)(.)/, 'The last successful match is bogus; Bug 27940');
        like("A@+B", qr/A@{+}B/,  'Interpolation of @+ in /@{+}/; Bug 27940');
        like("A@-B", qr/A@{-}B/,  'Interpolation of @- in /@{-}/; Bug 27940');
        like("A@+B", qr/A@{+}B/x, 'Interpolation of @+ in /@{+}/x; Bug 27940');
        like("A@-B", qr/A@{-}B/x, 'Interpolation of @- in /@{-}/x; Bug 27940');
    }

    {
        my $s = 'foo bar baz';
        my (@k, @v, @fetch, $res);
        my $count = 0;
        my @names = qw ($+{A} $+{B} $+{C});
        if ($s =~ /(?<A>foo)\s+(?<B>bar)?\s+(?<C>baz)/) {
            while (my ($k, $v) = each (%+)) {
                $count++;
            }
            @k = sort keys   (%+);
            @v = sort values (%+);
            $res = 1;
            push @fetch,
                ["$+{A}", "$1"],
                ["$+{B}", "$2"],
                ["$+{C}", "$3"],
            ;
        } 
        foreach (0 .. 2) {
            if ($fetch [$_]) {
                is($fetch[$_][0], $fetch[$_][1], "$names[$_]; Bug 50496");
            } else {
                ok 0, $names[$_];
            }
        }
        is($res, 1, "'$s' =~ /(?<A>foo)\\s+(?<B>bar)?\\s+(?<C>baz)/; Bug 50496");
        is($count, 3, "Got 3 keys in %+ via each; Bug 50496");
        is(0 + @k, 3, "Got 3 keys in %+ via keys; Bug 50496");
        is("@k", "A B C", "Got expected keys; Bug 50496");
        is("@v", "bar baz foo", "Got expected values; Bug 50496");
        eval '
            no warnings "uninitialized";
            print for $+ {this_key_doesnt_exist};
        ';
        is($@, '', 'lvalue $+ {...} should not throw an exception; Bug 50496');
    }

    {
        #
        # Almost the same as the block above, except that the capture is nested.
        #

        my $s = 'foo bar baz';
        my (@k, @v, @fetch, $res);
        my $count = 0;
        my @names = qw ($+{A} $+{B} $+{C} $+{D});
        if ($s =~ /(?<D>(?<A>foo)\s+(?<B>bar)?\s+(?<C>baz))/) {
            while (my ($k,$v) = each(%+)) {
                $count++;
            }
            @k = sort keys   (%+);
            @v = sort values (%+);
            $res = 1;
            push @fetch,
                ["$+{A}", "$2"],
                ["$+{B}", "$3"],
                ["$+{C}", "$4"],
                ["$+{D}", "$1"],
            ;
        }
        foreach (0 .. 3) {
            if ($fetch [$_]) {
                is($fetch[$_][0], $fetch[$_][1], "$names[$_]; Bug 50496");
            } else {
                ok 0, $names [$_];
            }
        }
        is($res, 1, "'$s' =~ /(?<D>(?<A>foo)\\s+(?<B>bar)?\\s+(?<C>baz))/; Bug 50496");
        is($count, 4, "Got 4 keys in %+ via each; Bug 50496");
        is(@k, 4, "Got 4 keys in %+ via keys; Bug 50496");
        is("@k", "A B C D", "Got expected keys; Bug 50496");
        is("@v", "bar baz foo foo bar baz", "Got expected values; Bug 50496");
        eval '
            no warnings "uninitialized";
            print for $+ {this_key_doesnt_exist};
        ';
        is($@, '', 'lvalue $+ {...} should not throw an exception; Bug 50496');
    }

    {
        my $str = 'abc'; 
        my $count = 0;
        my $mval = 0;
        my $pval = 0;
        while ($str =~ /b/g) {$mval = $#-; $pval = $#+; $count ++}
        is($mval,  0, '@- should be empty; Bug 36046');
        is($pval,  0, '@+ should be empty; Bug 36046');
        is($count, 1, 'Should have matched once only; Bug 36046');
    }

    {
        my $message = '/m in precompiled regexp; Bug 40684';
        my $s = "abc\ndef";
        my $rex = qr'^abc$'m;
        ok($s =~ m/$rex/, $message);
        ok($s =~ m/^abc$/m, $message);
    }

    {
        my $message = '(?: ... )? should not lose $^R; Bug 36909';
        $^R = 'Nothing';
        {
            local $^R = "Bad";
            ok('x foofoo y' =~ m {
                      (foo) # $^R correctly set
                      (?{ "last regexp code result" })
            }x, $message);
            is($^R, 'last regexp code result', $message);
        }
        is($^R, 'Nothing', $message);

        {
            local $^R = "Bad";

            ok('x foofoo y' =~ m {
                      (?:foo|bar)+ # $^R correctly set
                      (?{ "last regexp code result" })
            }x, $message);
            is($^R, 'last regexp code result', $message);
        }
        is($^R, 'Nothing', $message);

        {
            local $^R = "Bad";
            ok('x foofoo y' =~ m {
                      (foo|bar)\1+ # $^R undefined
                      (?{ "last regexp code result" })
            }x, $message);
            is($^R, 'last regexp code result', $message);
        }
        is($^R, 'Nothing', $message);

        {
            local $^R = "Bad";
            ok('x foofoo y' =~ m {
                      (foo|bar)\1 # This time without the +
                      (?{"last regexp code result"})
            }x, $message);
            is($^R, 'last regexp code result', $message);
        }
        is($^R, 'Nothing', $message);
    }

    {
        my $message = 'Match is quadratic due to eval; See Bug 22395';
        our $count;
        for my $l (10, 100, 1000) {
            $count = 0;
            ('a' x $l) =~ /(.*)(?{ $count++ })[bc]/;
            is($count, $l*($l+3)/2+1, $message);
        }
    }
    {
        my $message = 'Match is linear, not quadratic; Bug 22395.';
        our $count;
        my $ok= 0;
        for my $l (10, 100, 1000) {
            $count = 0;
            ('a' x $l) =~ /(.*)(*{ $count++ })[bc]/;
            $ok += is($count, $l + 1, $message);
        }
        is($ok,3, "Optimistic eval does not disable optimisations");
    }

    {
        my $message = '@-/@+ should not have undefined values; Bug 22614';
        local $_ = 'ab';
        our @len = ();
        /(.){1,}(?{push @len,0+@-})(.){1,}(?{})^/;
        is("@len", "2 2 2", $message);
    }

    {
        my $message = '$& set on s///; Bug 18209';
        my $text = ' word1 word2 word3 word4 word5 word6 ';

        my @words = ('word1', 'word3', 'word5');
        my $count;
        foreach my $word (@words) {
            $text =~ s/$word\s//gi; # Leave a space to separate words
                                    # in the resultant str.
            # The following block is not working.
            if ($&) {
                $count ++;
            }
            # End bad block
        }
        is($count, 3, $message);
        is($text, ' word2 word4 word6 ', $message);
    }

    {
        # RT#6893

        local $_ = qq (A\nB\nC\n); 
        my @res;
        while (m#(\G|\n)([^\n]*)\n#gsx) { 
            push @res, "$2"; 
            last if @res > 3;
        }
        is("@res", "A B C", "/g pattern shouldn't infinite loop; Bug 6893");
    }

    {
        # No optimizer bug
        my @tails  = ('', '(?(1))', '(|)', '()?');    
        my @quants = ('*','+');
        my $doit = sub {
            my $pats = shift;
            for (@_) {
                for my $pat (@$pats) {
                    for my $quant (@quants) {
                        for my $tail (@tails) {
                            my $re = "($pat$quant\$)$tail";
                            ok(/$re/  && $1 eq $_, "'$_' =~ /$re/; Bug 41010");
                            ok(/$re/m && $1 eq $_, "'$_' =~ /$re/m; Bug 41010");
                        }
                    }
                }
            }
        };    
        
        my @dpats = ('\d',
                     '[1234567890]',
                     '(1|[23]|4|[56]|[78]|[90])',
                     '(?:1|[23]|4|[56]|[78]|[90])',
                     '(1|2|3|4|5|6|7|8|9|0)',
                     '(?:1|2|3|4|5|6|7|8|9|0)');
        my @spats = ('[ ]', ' ', '( |\t)', '(?: |\t)', '[ \t]', '\s');
        my @sstrs = ('  ');
        my @dstrs = ('12345');
        $doit -> (\@spats, @sstrs);
        $doit -> (\@dpats, @dstrs);
    }

    {
        # [perl #45605] Regexp failure with utf8-flagged and byte-flagged string

        my $utf_8 = "\xd6schel";
        utf8::upgrade ($utf_8);
        $utf_8 =~ m {(\xd6|&Ouml;)schel};
        is($1, "\xd6", "Upgrade error; Bug 45605");
    }

    {
        # Regardless of utf8ness any character matches itself when 
        # doing a case insensitive match. See also [perl #36207] 

        for my $o (0 .. 255) {
            my @ch = (chr ($o), chr ($o));
            utf8::upgrade ($ch [1]);
            for my $u_str (0, 1) {
                for my $u_pat (0, 1) {
                    like($ch[$u_str], qr/\Q$ch[$u_pat]\E/i,
			 "\$c =~ /\$c/i : chr ($o) : u_str = $u_str u_pat = $u_pat; Bug 36207");
                    like($ch[$u_str], qr/\Q$ch[$u_pat]\E|xyz/i,
			 "\$c=~/\$c|xyz/i : chr($o) : u_str = $u_str u_pat = $u_pat; Bug 36207");
                }
            }
        }
    }

    {
         my $message = '$REGMARK in replacement; Bug 49190';
         our $REGMARK;
         local $_ = "A";
         ok(s/(*:B)A/$REGMARK/, $message);
         is($_, "B", $message);
         $_ = "CCCCBAA";
         ok(s/(*:X)A+|(*:Y)B+|(*:Z)C+/$REGMARK/g, $message);
         is($_, "ZYX", $message);
         # Use a longer name to force reallocation of $REGMARK.
         $_ = "CCCCBAA";
         ok(s/(*:X)A+|(*:YYYYYYYYYYYYYYYY)B+|(*:Z)C+/$REGMARK/g, $message);
         is($_, "ZYYYYYYYYYYYYYYYYX", $message);
    }

    {
        my $message = 'Substitution evaluation in list context; Bug 52658';
        my $reg = '../xxx/';
        my @te  = ($reg =~ m{^(/?(?:\.\./)*)},
                   $reg =~ s/(x)/'b'/eg > 1 ? '##' : '++');
        is($reg, '../bbb/', $message);
        is($te [0], '../', $message);
    }

    {
        my $a = "xyzt" x 8192;
        like($a, qr/\A(?>[a-z])*\z/,
	     '(?>) does not cause wrongness on long string; Bug 60034');
        my $b = $a . chr 256;
        chop $b;
	is($a, $b, 'Bug 60034');
        like($b, qr/\A(?>[a-z])*\z/,
	     '(?>) does not cause wrongness on long string with UTF-8; Bug 60034');
    }

    #
    # Keep the following tests last -- they may crash perl
    #
    print "# Tests that follow may crash perl\n";
    {   

        my $message = 'Pattern in a loop, failure should not ' .
                         'affect previous success; Bug 19049/38869';
        my @list = (
            'ab cdef',             # Matches regex
            ('e' x 40000 ) .'ab c' # Matches not, but 'ab c' matches part of it
        );
        my $y;
        my $x;
        foreach (@list) {
            m/ab(.+)cd/i; # The ignore-case seems to be important
            $y = $1;      # Use $1, which might not be from the last match!
            $x = substr ($list [0], $- [0], $+ [0] - $- [0]);
        }
        is($y, ' ', $message);
        is($x, 'ab cd', $message);
    }

    SKIP: {
        skip("Can run out of memory on os390", 1) if $^O eq 'os390';
        ok (("a" x (2 ** 15 - 10)) =~ /^()(a|bb)*$/, "Recursive stack cracker; Bug 24274");
    }
    {
        ok ((q(a)x 100) =~ /^(??{'(.)'x 100})/, 
            "Regexp /^(??{'(.)'x 100})/ crashes older perls; Bug 24274");
    }

    {
        # [perl #45337] utf8 + "[a]a{2}" + /$.../ = panic: sv_len_utf8 cache

        local ${^UTF8CACHE} = -1;
        my $message = "Shouldn't panic; Bug 45337";
        my $s = "[a]a{2}";
        utf8::upgrade $s;
        like("aaa", qr/$s/, $message);
    }
    {
	my $message = "Check if tree logic breaks \$^R; Bug 57042";
	my $cond_re = qr/\s*
	    \s* (?:
		   \( \s* A  (?{1})
		 | \( \s* B  (?{2})
	       )
	   /x;
	my @res;
	for my $line ("(A)","(B)") {
	   if ($line =~ m/$cond_re/) {
	       push @res, $^R ? "#$^R" : "UNDEF";
	   }
	}
	is("@res","#1 #2", $message);
    }
    {
	no warnings 'closure';
	my $re = qr/A(??{"1"})/;
	ok "A1B" =~ m/^((??{ $re }))((??{"B"}))$/;
	ok $1 eq "A1";
	ok $2 eq "B";
    }

    # This only works under -DEBUGGING because it relies on an assert().
    {
	# Check capture offset re-entrancy of utf8 code.

        sub fswash { $_[0] =~ s/([>X])//g; }

        my $k1 = "." x 4 . ">>";
        fswash($k1);

        my $k2 = "\x{f1}\x{2022}";
        $k2 =~ s/([\360-\362])/>/g;
        fswash($k2);

        is($k2, "\x{2022}", "utf8::SWASHNEW doesn't cause capture leaks; Bug 60508");
    }

    {
	# minimal CURLYM limited to 32767 matches
	my @pat = (
	    qr{a(x|y)*b},	# CURLYM
	    qr{a(x|y)*?b},	# .. with minmod
	    qr{a([wx]|[yz])*b},	# .. and without tries
	    qr{a([wx]|[yz])*?b},
	);
	my $len = 32768;
	my $s = join '', 'a', 'x' x $len, 'b';
	for my $pat (@pat) {
	    like($s, $pat, "$pat; Bug 65372");
	}
    }

    {
        local $::TODO = "[perl #38133]";

        "A" =~ /(((?:A))?)+/;
        my $first = $2;

        "A" =~ /(((A))?)+/;
        my $second = $2;

        is($first, $second);
    }    

    {
       my $message
        = 'utf8 =~ /trie/ where trie matches a continuation octet; Bug 70998';

       # Catch warnings:
       my $w;
       local $SIG{__WARN__} = sub { $w .= shift };

       # This bug can be reduced to
       qq{\x{30ab}} =~ /\xab|\xa9/;
       # but it's nice to have a more 'real-world' test. The original test
       # case from the RT ticket follows:

       my %conv = (
                   "\xab"     => "&lt;",
                   "\xa9"     => "(c)",
                  );
       my $conv_rx = '(' . join('|', map { quotemeta } keys %conv) . ')';
       $conv_rx = qr{$conv_rx};

       my $x
        = qq{\x{3042}\x{304b}\x{3055}\x{305f}\x{306a}\x{306f}\x{307e}}
        . qq{\x{3084}\x{3089}\x{308f}\x{3093}\x{3042}\x{304b}\x{3055}}
        . qq{\x{305f}\x{306a}\x{306f}\x{307e}\x{3084}\x{3089}\x{308f}}
        . qq{\x{3093}\x{30a2}\x{30ab}\x{30b5}\x{30bf}\x{30ca}\x{30cf}}
        . qq{\x{30de}\x{30e4}\x{30e9}\x{30ef}\x{30f3}\x{30a2}\x{30ab}}
        . qq{\x{30b5}\x{30bf}\x{30ca}\x{30cf}\x{30de}\x{30e4}\x{30e9}}
        . qq{\x{30ef}\x{30f3}\x{30a2}\x{30ab}\x{30b5}\x{30bf}\x{30ca}}
        . qq{\x{30cf}\x{30de}\x{30e4}\x{30e9}\x{30ef}\x{30f3}};

       $x =~ s{$conv_rx}{$conv{$1}}eg;

       is($w, undef, $message);
    }

    {
        # minimal CURLYM limited to 32767 matches

        is(join("-", "   abc   def  " =~ /(?=(\S+))/g), "abc-bc-c-def-ef-f",
	   'stclass optimisation does not break + inside (?=); Bug 68564');
    }

    {
        use charnames ":full";
        # Delayed interpolation of \N'
        my $r1 = qr/\N{THAI CHARACTER SARA I}/;
        my $r2 = qr'\N{THAI CHARACTER SARA I}';
        my $s1 = "\x{E34}\x{E34}\x{E34}\x{E34}";

        # Bug #56444
        ok $s1 =~ /$r1+/, 'my $r1 = qr/\N{THAI CHARACTER SARA I}/; my $s1 = "\x{E34}\x{E34}\x{E34}\x{E34}; $s1 =~ /$r1+/';
        ok $s1 =~ /$r2+/, 'my $r2 = qr\'\N{THAI CHARACTER SARA I}\'; my $s1 = "\x{E34}\x{E34}\x{E34}\x{E34}; $s1 =~ \'$r2+\'';

        # Bug #62056
        ok "${s1}A" =~ m/$s1\N{LATIN CAPITAL LETTER A}/, '"${s1}A" =~ m/$s1\N{LATIN CAPITAL LETTER A}/';

        ok "abbbbc" =~ m/\N{1}/ && $& eq "a", '"abbbbc" =~ m/\N{1}/ && $& eq "a"';
        ok "abbbbc" =~ m'\N{1}' && $& eq "a", '"abbbbc" =~ m\'\N{1}\' && $& eq "a"';
        ok "abbbbc" =~ m/\N{3,4}/ && $& eq "abbb", '"abbbbc" =~ m/\N{3,4}/ && $& eq "abbb"';
        ok "abbbbc" =~ m'\N{3,4}' && $& eq "abbb", '"abbbbc" =~ m\'\N{3,4}\' && $& eq "abbb"';
    }

    {
        use charnames ":full";
        my $message = '[perl #74982] Period coming after \N{}';
        ok("\x{ff08}." =~ m/\N{FULLWIDTH LEFT PARENTHESIS}./ && $& eq "\x{ff08}.", $message);
        ok("\x{ff08}." =~ m'\N{FULLWIDTH LEFT PARENTHESIS}.' && $& eq "\x{ff08}.", $message);
        ok("\x{ff08}." =~ m/[\N{FULLWIDTH LEFT PARENTHESIS}]./ && $& eq "\x{ff08}.", $message);
        ok("\x{ff08}." =~ m'[\N{FULLWIDTH LEFT PARENTHESIS}].' && $& eq "\x{ff08}.", $message);
    }

SKIP: {
    ######## "Segfault using HTML::Entities", Richard Jolly <richardjolly@mac.com>, <A3C7D27E-C9F4-11D8-B294-003065AE00B6@mac.com> in perl-unicode@perl.org

    skip('Perl configured without Encode module', 1)
	unless $Config{extensions} =~ / Encode /;

    # Test case cut down by jhi
    fresh_perl_like(<<'EOP', qr!Malformed UTF-8 character \(unexpected end of string\)!, {}, 'Segfault using HTML::Entities');
use Encode;
my $t = ord('A') == 193 ? "\xEA" : "\xE9";
Encode::_utf8_on($t);
substr($t,0);
$t =~ s/([^a])//ge;
EOP
    }

    {
        # pattern must be compiled late or we can break the test file
        my $message = '[perl #115050] repeated nothings in a trie can cause panic';
        my $pattern;
        $pattern = '[xyz]|||';
        ok("blah blah" =~ /$pattern/, $message);
        ok("blah blah" =~ /(?:$pattern)h/, $message);
        $pattern = '|||[xyz]';
        ok("blah blah" =~ /$pattern/, $message);
        ok("blah blah" =~ /(?:$pattern)h/, $message);
    }

    {
        # [perl #4289] First mention $& after a match
	local $::TODO = "these tests fail without Copy-on-Write enabled"
	    if $Config{ccflags} =~ /PERL_NO_COW/;
        fresh_perl_is(
            '$_ = "abc"; /b/g; $_ = "hello"; print eval q|$&|, "\n"',
            "b\n", {}, '$& first mentioned after match');
        fresh_perl_is(
            '$_ = "abc"; /b/g; $_ = "hello"; print eval q|$`|, "\n"',
            "a\n", {}, '$` first mentioned after match');
        fresh_perl_is(
            '$_ = "abc"; /b/g; $_ = "hello"; print eval q|$\'|,"\n"',
            "c\n", {}, '$\' first mentioned after match');
    }

    {
	# [perl #118175] threaded perl-5.18.0 fails pat_rt_report_thr.t
	# this tests some related failures
	#
	# The tests in the block *only* fail when run on 32-bit systems
	# with a malloc that allocates above the 2GB line.  On the system
	# in the report above that only happened in a thread.
	my $s = "\x{1ff}" . "f" x 32;
	ok($s =~ /\x{1ff}[[:alpha:]]+/gca, "POSIXA pointer wrap");
    }

    {
        # RT #129012 heap-buffer-overflow Perl_fbm_instr.
        # This test is unlikely to not pass, but it used to fail
        # ASAN/valgrind

        my $s ="\x{100}0000000";
        ok($s !~ /00000?\x80\x80\x80/, "RT #129012");
    }

    {
        # RT #129085 heap-buffer-overflow Perl_re_intuit_start
        # this did fail under ASAN, but didn't under valgrind
        my $s = "\x{f2}\x{140}\x{fe}\x{ff}\x{ff}\x{ff}";
        ok($s !~ /^0000.\34500\376\377\377\377/, "RT #129085");
    }
    {
        # rt
        fresh_perl_is(
            'no warnings "regexp"; "foo"=~/((?1)){8,0}/; print "ok"',
            "ok", {},  'RT #130561 - allowing impossible quantifier should not cause SEGVs');
        my $s= "foo";
        no warnings 'regexp';
        ok($s=~/(foo){1,0}|(?1)/,
            "RT #130561 - allowing impossible quantifier should not break recursion");
    }
	{
		# RT #133892 Coredump in Perl_re_intuit_start
		# Second match flips to checking floating substring before fixed
		# substring, which triggers a pathway that failed to check there
		# was a non-utf8 version of the string before trying to use it
		# resulting in a SEGV.
		my $result = grep /b\x{1c0}ss0/i, qw{ xxxx xxxx0 };
		ok($result == 0);
	}

} # End of sub run_tests

1;
