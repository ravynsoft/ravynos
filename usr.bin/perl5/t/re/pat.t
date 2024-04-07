#!./perl
#
# This is a home for regular expression tests that don't fit into
# the format supported by re/regexp.t.  If you want to add a test
# that does fit that format, add it to re/re_tests, not here.

use strict;
use warnings;
no warnings 'experimental::vlb';
use 5.010;

sub run_tests;

$| = 1;


BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib', '.', '../ext/re');
    require Config; import Config;
    require './charset_tools.pl';
    require './loc_tools.pl';
}

skip_all_without_unicode_tables();

my $has_locales = locales_enabled('LC_CTYPE');

plan tests => 1265;  # Update this when adding/deleting tests.

run_tests() unless caller;

#
# Tests start here.
#
sub run_tests {
    {
        # see https://github.com/Perl/perl5/issues/12948
        my $string="ABCDEFGHIJKL";
        my $pat= "(.)" x length($string);
        my $ok= $string=~/^$pat\z/;
        foreach my $n (1 .. length($string)) {
            $ok= eval sprintf 'is $%d, "%s", q($%d = %s); 1', ($n, substr($string,$n-1,1))x2;
            ok($ok, "eval for \$$n test");
            $ok= eval sprintf 'is ${%d}, "%s", q(${%d} = %s); 1', ($n, substr($string,$n-1,1))x2;
            ok($ok, "eval for \${$n} test");

            $ok= eval sprintf 'is $0%d, "%s", q($0%d = %s); 1', ($n, substr($string,$n-1,1))x2;
            ok(!$ok, "eval failed as expected for \$0$n test");
            $ok= eval sprintf 'is ${0%d}, "%s", q(${0%d} = %s); 1', ($n, substr($string,$n-1,1))x2;
            ok(!$ok, "eval failed as expected for \${0$n} test");

            no strict 'refs';
            $ok= eval sprintf 'is ${0b%b}, "%s", q(${0b%b} = %s); 1', ($n, substr($string,$n-1,1))x2;
            ok($ok, sprintf "eval for \${0b%b} test", $n);
            $ok= eval sprintf 'is ${0x%x}, "%s", q(${0x%x} = %s); 1', ($n, substr($string,$n-1,1))x2;
            ok($ok, sprintf "eval for \${0x%x} test", $n);
            $ok= eval sprintf 'is ${0b%08b}, "%s", q(${0b%08b} = %s); 1', ($n, substr($string,$n-1,1))x2;
            ok($ok, sprintf "eval for \${0b%b} test", $n);
            $ok= eval sprintf 'is ${0x%04x}, "%s", q(${0x%04x} = %s); 1', ($n, substr($string,$n-1,1))x2;
            ok($ok, sprintf "eval for \${0x%04x} test", $n);
        }
    }

    my $sharp_s = uni_to_native("\xdf");

    {
        my $x = "abc\ndef\n";
	(my $x_pretty = $x) =~ s/\n/\\n/g;

        ok $x =~ /^abc/,  qq ["$x_pretty" =~ /^abc/];
        ok $x !~ /^def/,  qq ["$x_pretty" !~ /^def/];

        # used to be a test for $*
        ok $x =~ /^def/m, qq ["$x_pretty" =~ /^def/m];

        ok(!($x =~ /^xxx/), qq ["$x_pretty" =~ /^xxx/]);
        ok(!($x !~ /^abc/), qq ["$x_pretty" !~ /^abc/]);

         ok $x =~ /def/, qq ["$x_pretty" =~ /def/];
        ok(!($x !~ /def/), qq ["$x_pretty" !~ /def/]);

         ok $x !~ /.def/, qq ["$x_pretty" !~ /.def/];
        ok(!($x =~ /.def/), qq ["$x_pretty" =~ /.def/]);

         ok $x =~ /\ndef/, qq ["$x_pretty" =~ /\\ndef/];
        ok(!($x !~ /\ndef/), qq ["$x_pretty" !~ /\\ndef/]);
    }

    {
        $_ = '123';
        ok /^([0-9][0-9]*)/, qq [\$_ = '$_'; /^([0-9][0-9]*)/];
    }

    {
        $_ = 'aaabbbccc';
         ok /(a*b*)(c*)/ && $1 eq 'aaabbb' && $2 eq 'ccc',
                                             qq [\$_ = '$_'; /(a*b*)(c*)/];
         ok /(a+b+c+)/ && $1 eq 'aaabbbccc', qq [\$_ = '$_'; /(a+b+c+)/];
        unlike($_, qr/a+b?c+/, qq [\$_ = '$_'; /a+b?c+/]);

        $_ = 'aaabccc';
         ok /a+b?c+/, qq [\$_ = '$_'; /a+b?c+/];
         ok /a*b?c*/, qq [\$_ = '$_'; /a*b?c*/];

        $_ = 'aaaccc';
         ok /a*b?c*/, qq [\$_ = '$_'; /a*b?c*/];
        unlike($_, qr/a*b+c*/, qq [\$_ = '$_'; /a*b+c*/]);

        $_ = 'abcdef';
         ok /bcd|xyz/, qq [\$_ = '$_'; /bcd|xyz/];
         ok /xyz|bcd/, qq [\$_ = '$_'; /xyz|bcd/];
         ok m|bc/*d|,  qq [\$_ = '$_'; m|bc/*d|];
         ok /^$_$/,    qq [\$_ = '$_'; /^\$_\$/];
    }

    {
        # used to be a test for $*
        ok "ab\ncd\n" =~ /^cd/m, q ["ab\ncd\n" =~ /^cd/m];
    }

    {
        our %XXX = map {($_ => $_)} 123, 234, 345;

        our @XXX = ('ok 1','not ok 1', 'ok 2','not ok 2','not ok 3');
        while ($_ = shift(@XXX)) {
            my $e = index ($_, 'not') >= 0 ? '' : 1;
            my $r = m?(.*)?;
            is($r, $e, "?(.*)?");
            /not/ && reset;
            if (/not ok 2/) {
                if ($^O eq 'VMS') {
                    $_ = shift(@XXX);
                }
                else {
                    reset 'X';
                }
            }
        }

        SKIP: {
            if ($^O eq 'VMS') {
                skip "Reset 'X'", 1;
            }
            ok !keys %XXX, "%XXX is empty";
        }

    }

    {
        my $message = "Test empty pattern";
        my $xyz = 'xyz';
        my $cde = 'cde';

        $cde =~ /[^ab]*/;
        $xyz =~ //;
        is($&, $xyz, $message);

        my $foo = '[^ab]*';
        $cde =~ /$foo/;
        $xyz =~ //;
        is($&, $xyz, $message);

        $cde =~ /$foo/;
        my $null;
        no warnings 'uninitialized';
        $xyz =~ /$null/;
        is($&, $xyz, $message);

        $null = "";
        $xyz =~ /$null/;
        is($&, $xyz, $message);

        # each entry: regexp, match string, $&, //o match success
        my @tests =
          (
           [ "", "xy", "x", 1 ],
           [ "y", "yz", "y", !1 ],
          );
        for my $test (@tests) {
            my ($re, $str, $matched, $omatch) = @$test;
            $xyz =~ /x/o;
            ok($str =~ /$re/, "$str matches /$re/");
            is($&, $matched, "on $matched");
            $xyz =~ /x/o;
            is($str =~ /$re/o, $omatch, "$str matches /$re/o (or not)");
        }
    }

    {
        my $message = q !Check $`, $&, $'!;
        $_ = 'abcdefghi';
        /def/;        # optimized up to cmd
        is("$`:$&:$'", 'abc:def:ghi', $message);

        no warnings 'void';
        /cde/ + 0;    # optimized only to spat
        is("$`:$&:$'", 'ab:cde:fghi', $message);

        /[d][e][f]/;    # not optimized
        is("$`:$&:$'", 'abc:def:ghi', $message);
    }

    {
        $_ = 'now is the {time for all} good men to come to.';
        / \{([^}]*)}/;
        is($1, 'time for all', "Match braces");
    }

    {
        my $message = "{N,M} quantifier";
        $_ = 'xxx {3,4}  yyy   zzz';
        ok(/( {3,4})/, $message);
        is($1, '   ', $message);
        unlike($_, qr/( {4,})/, $message);
        ok(/( {2,3}.)/, $message);
        is($1, '  y', $message);
        ok(/(y{2,3}.)/, $message);
        is($1, 'yyy ', $message);
        unlike($_, qr/x {3,4}/, $message);
        unlike($_, qr/^xxx {3,4}/, $message);
    }

    {
        my $message = "Test /g";
        local $" = ":";
        $_ = "now is the time for all good men to come to.";
        my @words = /(\w+)/g;
        my $exp   = "now:is:the:time:for:all:good:men:to:come:to";

        is("@words", $exp, $message);

        @words = ();
        while (/\w+/g) {
            push (@words, $&);
        }
        is("@words", $exp, $message);

        @words = ();
        pos = 0;
        while (/to/g) {
            push(@words, $&);
        }
        is("@words", "to:to", $message);

        pos $_ = 0;
        @words = /to/g;
        is("@words", "to:to", $message);
    }

    {
        $_ = "abcdefghi";

        my $pat1 = 'def';
        my $pat2 = '^def';
        my $pat3 = '.def.';
        my $pat4 = 'abc';
        my $pat5 = '^abc';
        my $pat6 = 'abc$';
        my $pat7 = 'ghi';
        my $pat8 = '\w*ghi';
        my $pat9 = 'ghi$';

        my $t1 = my $t2 = my $t3 = my $t4 = my $t5 =
        my $t6 = my $t7 = my $t8 = my $t9 = 0;

        for my $iter (1 .. 5) {
            $t1++ if /$pat1/o;
            $t2++ if /$pat2/o;
            $t3++ if /$pat3/o;
            $t4++ if /$pat4/o;
            $t5++ if /$pat5/o;
            $t6++ if /$pat6/o;
            $t7++ if /$pat7/o;
            $t8++ if /$pat8/o;
            $t9++ if /$pat9/o;
        }
        my $x = "$t1$t2$t3$t4$t5$t6$t7$t8$t9";
        is($x, '505550555', "Test /o");
    }

    {
        my $xyz = 'xyz';
        ok "abc" =~ /^abc$|$xyz/, "| after \$";

        # perl 4.009 says "unmatched ()"
        my $message = '$ inside ()';

        my $result;
        eval '"abc" =~ /a(bc$)|$xyz/; $result = "$&:$1"';
        is($@, "", $message);
        is($result, "abc:bc", $message);
    }

    {
        my $message = "Scalar /g";
        $_ = "abcfooabcbar";

        ok( /abc/g && $` eq "", $message);
        ok( /abc/g && $` eq "abcfoo", $message);
        ok(!/abc/g, $message);

        $message = "Scalar /gi";
        pos = 0;
        ok( /ABC/gi && $` eq "", $message);
        ok( /ABC/gi && $` eq "abcfoo", $message);
        ok(!/ABC/gi, $message);

        $message = "Scalar /g";
        pos = 0;
        ok( /abc/g && $' eq "fooabcbar", $message);
        ok( /abc/g && $' eq "bar", $message);

        $_ .= '';
        my @x = /abc/g;
        is(@x, 2, "/g reset after assignment");
    }

    {
        my $message = '/g, \G and pos';
        $_ = "abdc";
        pos $_ = 2;
        /\Gc/gc;
        is(pos $_, 2, $message);
        /\Gc/g;
        is(pos $_, undef, $message);
    }

    {
        my $message = '(?{ })';
        our $out = 1;
        'abc' =~ m'a(?{ $out = 2 })b';
        is($out, 2, $message);

        $out = 1;
        'abc' =~ m'a(?{ $out = 3 })c';
        is($out, 1, $message);
    }

    {
        $_ = 'foobar1 bar2 foobar3 barfoobar5 foobar6';
        my @out = /(?<!foo)bar./g;
        is("@out", 'bar2 barf', "Negative lookbehind");
    }

    {
        my $message = "REG_INFTY tests";
        # Tests which depend on REG_INFTY

	#  Defaults assumed if this fails
	eval { require Config; };
        $::reg_infty   = $Config::Config{reg_infty} // ((1<<31)-1);
        $::reg_infty_m = $::reg_infty - 1;
        $::reg_infty_p = $::reg_infty + 1;
        $::reg_infty_m = $::reg_infty_m;   # Suppress warning.

        # As well as failing if the pattern matches do unexpected things, the
        # next three tests will fail if you should have picked up a lower-than-
        # default value for $reg_infty from Config.pm, but have not.
        SKIP: {
            skip "REG_INFTY too big to test ($::reg_infty)", 7
                if $::reg_infty > (1<<16);

            is(eval q{('aaa' =~ /(a{1,$::reg_infty_m})/)[0]}, 'aaa', $message);
            is($@, '', $message);
            is(eval q{('a' x $::reg_infty_m) =~ /a{$::reg_infty_m}/}, 1, $message);
            is($@, '', $message);
            isnt(q{('a' x ($::reg_infty_m - 1)) !~ /a{$::reg_infty_m}/}, 1, $message);
            is($@, '', $message);

            # It should be 'a' x 2147483647, but that exhausts memory on
            # reasonably sized modern machines
            like('a' x $::reg_infty_m, qr/a{1,}/,
                 "{1,} matches more times than REG_INFTY");
        }

        eval "'aaa' =~ /a{1,$::reg_infty}/";
        like($@, qr/^\QQuantifier in {,} bigger than/, $message);
        eval "'aaa' =~ /a{1,$::reg_infty_p}/";
        like($@, qr/^\QQuantifier in {,} bigger than/, $message);

    }

    {
        # Poke a couple more parse failures
        my $context = 'x' x 256;
        eval qq("${context}y" =~ /(?<=$context)y/);
        ok $@ =~ /^\QLookbehind longer than 255 not/, "Lookbehind limit";
    }

  SKIP:
    {   # Long Monsters

        my @trials = (125, 140, 250, 270, 300000, 30);

        skip('limited memory', @trials * 4) if $ENV{'PERL_SKIP_BIG_MEM_TESTS'};

        for my $l (@trials) { # Ordered to free memory
            my $a = 'a' x $l;
            # we do not use like() or unlike() here as the string
            # is very long and is not useful if the match fails,
            # the useful part
	    ok("ba$a=" =~ m/a$a=/, sprintf
                'Long monster: ("ba".("a" x %d)."=") =~ m/aa...a=/', $l);
            ok("b$a="  !~ m/a$a=/, sprintf
                'Long monster: ("b" .("a" x %d)."=") !~ m/aa...a=/', $l);
            ok("b$a="  =~ m/ba+=/, sprintf
                'Long monster: ("b" .("a" x %d)."=") =~ m/ba+=/', $l);
	    ok("ba$a=" =~ m/b(?:a|b)+=/, sprintf
                'Long monster: ("ba".("a" x %d)."=") =~ m/b(?:a|b)+=/', $l);
        }
    }

  SKIP:
    {   # 20000 nodes, each taking 3 words per string, and 1 per branch

        my %ans = ( 'ax13876y25677lbc' => 1,
                    'ax13876y25677mcb' => 0, # not b.
                    'ax13876y35677nbc' => 0, # Num too big
                    'ax13876y25677y21378obc' => 1,
                    'ax13876y25677y21378zbc' => 0,    # Not followed by [k-o]
                    'ax13876y25677y21378y21378kbc' => 1,
                    'ax13876y25677y21378y21378kcb' => 0, # Not b.
                    'ax13876y25677y21378y21378y21378kbc' => 0, # 5 runs
                  );

        skip('limited memory', 2 * scalar keys %ans) if $ENV{'PERL_SKIP_BIG_MEM_TESTS'};

        my $long_constant_len = join '|', 12120 .. 32645;
        my $long_var_len = join '|', 8120 .. 28645;

        for (keys %ans) {
	    my $message = "20000 nodes, const-len '$_'";
            ok !($ans{$_} xor /a(?=([yx]($long_constant_len)){2,4}[k-o]).*b./o), $message;

	    $message = "20000 nodes, var-len '$_'";
            ok !($ans{$_} xor /a(?=([yx]($long_var_len)){2,4}[k-o]).*b./o,), $message;
        }
    }

    {
        my $message = "Complicated backtracking";
        $_ = " a (bla()) and x(y b((l)u((e))) and b(l(e)e)e";
        my $expect = "(bla()) ((l)u((e))) (l(e)e)";

        our $c;
        sub matchit {
          m/
             (
               \(
               (?{ $c = 1 })    # Initialize
               (?:
                 (?(?{ $c == 0 })   # PREVIOUS iteration was OK, stop the loop
                   (?!
                   )        # Fail: will unwind one iteration back
                 )
                 (?:
                   [^()]+        # Match a big chunk
                   (?=
                     [()]
                   )        # Do not try to match subchunks
                 |
                   \(
                   (?{ ++$c })
                 |
                   \)
                   (?{ --$c })
                 )
               )+        # This may not match with different subblocks
             )
             (?(?{ $c != 0 })
               (?!
               )        # Fail
             )            # Otherwise the chunk 1 may succeed with $c>0
           /xg;
        }

        my @ans = ();
        my $res;
        push @ans, $res while $res = matchit;
        is("@ans", "1 1 1", $message);

        @ans = matchit;
        is("@ans", $expect, $message);

        $message = "Recursion with (??{ })";
        our $matched;
        $matched = qr/\((?:(?>[^()]+)|(??{$matched}))*\)/;

        @ans = my @ans1 = ();
        push (@ans, $res), push (@ans1, $&) while $res = m/$matched/g;

        is("@ans", "1 1 1", $message);
        is("@ans1", $expect, $message);

        @ans = m/$matched/g;
        is("@ans", $expect, $message);

    }

    {
        ok "abc" =~ /^(??{"a"})b/, '"abc" =~ /^(??{"a"})b/';
    }

    {
        my @ans = ('a/b' =~ m%(.*/)?(.*)%);    # Stack may be bad
        is("@ans", 'a/ b', "Stack may be bad");
    }

    {
        my $message = "Eval-group not allowed at runtime";
        my $code = '{$blah = 45}';
        our $blah = 12;
        eval { /(?$code)/ };
        ok($@ && $@ =~ /not allowed at runtime/ && $blah == 12, $message);

	$blah = 12;
	my $res = eval { "xx" =~ /(?$code)/o };
	{
	    no warnings 'uninitialized';
	    chomp $@; my $message = "$message '$@', '$res', '$blah'";
	    ok($@ && $@ =~ /not allowed at runtime/ && $blah == 12, $message);
	}

        $code = '=xx';
	$blah = 12;
	$res = eval { "xx" =~ /(?$code)/o };
	{
	    no warnings 'uninitialized';
	    my $message = "$message '$@', '$res', '$blah'";
	    ok(!$@ && $res, $message);
	}

        $code = '{$blah = 45}';
        $blah = 12;
        eval "/(?$code)/";
        is($blah, 45, $message);

        $blah = 12;
        /(?{$blah = 45})/;
        is($blah, 45, $message);
    }

    {
        my $message = "Pos checks";
        my $x = 'banana';
        $x =~ /.a/g;
        is(pos $x, 2, $message);

        $x =~ /.z/gc;
        is(pos $x, 2, $message);

        sub f {
            my $p = $_[0];
            return $p;
        }

        $x =~ /.a/g;
        is(f (pos $x), 4, $message);
    }

    {
        my $message = 'Checking $^R';
        our $x = $^R = 67;
        'foot' =~ /foo(?{$x = 12; 75})[t]/;
        is($^R, 75, $message);

        $x = $^R = 67;
        'foot' =~ /foo(?{$x = 12; 75})[xy]/;
        ok($^R eq '67' && $x eq '12', $message);

        $x = $^R = 67;
        'foot' =~ /foo(?{ $^R + 12 })((?{ $x = 12; $^R + 17 })[xy])?/;
        ok($^R eq '79' && $x eq '12', $message);
    }

    {
        is(qr/\b\v$/i,    '(?^i:\b\v$)', 'qr/\b\v$/i');
        is(qr/\b\v$/s,    '(?^s:\b\v$)', 'qr/\b\v$/s');
        is(qr/\b\v$/m,    '(?^m:\b\v$)', 'qr/\b\v$/m');
        is(qr/\b\v$/x,    '(?^x:\b\v$)', 'qr/\b\v$/x');
        is(qr/\b\v$/xism, '(?^msix:\b\v$)',  'qr/\b\v$/xism');
        is(qr/\b\v$/,     '(?^:\b\v$)', 'qr/\b\v$/');
    }

    {   # Test that charset modifier work, and are interpolated
        is(qr/\b\v$/, '(?^:\b\v$)', 'Verify no locale, no unicode_strings gives default modifier');
        is(qr/(?l:\b\v$)/, '(?^:(?l:\b\v$))', 'Verify infix l modifier compiles');
        is(qr/(?u:\b\v$)/, '(?^:(?u:\b\v$))', 'Verify infix u modifier compiles');
        is(qr/(?l)\b\v$/, '(?^:(?l)\b\v$)', 'Verify (?l) compiles');
        is(qr/(?u)\b\v$/, '(?^:(?u)\b\v$)', 'Verify (?u) compiles');

        my $dual = qr/\b\v$/;
        my $locale;

      SKIP: {
            skip 'Locales not available', 1 unless $has_locales;

            use locale;
            $locale = qr/\b\v$/;
            is($locale,    '(?^l:\b\v$)', 'Verify has l modifier when compiled under use locale');
            no locale;
        }

        use feature 'unicode_strings';
        my $unicode = qr/\b\v$/;
        is($unicode,    '(?^u:\b\v$)', 'Verify has u modifier when compiled under unicode_strings');
        is(qr/abc$dual/,    '(?^u:abc(?^:\b\v$))', 'Verify retains d meaning when interpolated under locale');

      SKIP: {
            skip 'Locales not available', 1 unless $has_locales;

            is(qr/abc$locale/,    '(?^u:abc(?^l:\b\v$))', 'Verify retains l when interpolated under unicode_strings');
        }

        no feature 'unicode_strings';
      SKIP: {
            skip 'Locales not available', 1 unless $has_locales;
            is(qr/abc$locale/,    '(?^:abc(?^l:\b\v$))', 'Verify retains l when interpolated outside locale and unicode strings');
        }

        is(qr/def$unicode/,    '(?^:def(?^u:\b\v$))', 'Verify retains u when interpolated outside locale and unicode strings');

      SKIP: {
            skip 'Locales not available', 2 unless $has_locales;

             use locale;
            is(qr/abc$dual/,    '(?^l:abc(?^:\b\v$))', 'Verify retains d meaning when interpolated under locale');
            is(qr/abc$unicode/,    '(?^l:abc(?^u:\b\v$))', 'Verify retains u when interpolated under locale');
        }
    }

    {
        my $message = "Look around";
        $_ = 'xabcx';
        foreach my $ans ('', 'c') {
            ok(/(?<=(?=a)..)((?=c)|.)/g, $message);
            is($1, $ans, $message);
        }
    }

    {
        my $message = "Empty clause";
        $_ = 'a';
        foreach my $ans ('', 'a', '') {
            ok(/^|a|$/g, $message);
            is($&, $ans, $message);
        }
    }

    {
        sub prefixify {
        my $message = "Prefixify";
            {
                my ($v, $a, $b, $res) = @_;
                ok($v =~ s/\Q$a\E/$b/, $message);
                is($v, $res, $message);
            }
        }

        prefixify ('/a/b/lib/arch', "/a/b/lib", 'X/lib', 'X/lib/arch');
        prefixify ('/a/b/man/arch', "/a/b/man", 'X/man', 'X/man/arch');
    }

    {
        $_ = 'var="foo"';
        /(\")/;
        ok $1 && /$1/, "Capture a quote";
    }

    {
        no warnings 'closure';
        my $message = '(?{ $var } refers to package vars';
        package aa;
        our $c = 2;
        $::c = 3;
        '' =~ /(?{ $c = 4 })/;
        main::is($c, 4, $message);
        main::is($::c, 3, $message);
    }

    {
        is(eval 'q(a:[b]:) =~ /[x[:foo:]]/', undef);
	like ($@, qr/POSIX class \[:[^:]+:\] unknown in regex/,
	      'POSIX class [: :] must have valid name');

        for my $d (qw [= .]) {
            is(eval "/[[${d}foo${d}]]/", undef);
	    like ($@, qr/\QPOSIX syntax [$d $d] is reserved for future extensions/,
		  "POSIX syntax [[$d $d]] is an error");
        }
    }

    {
        # test if failure of patterns returns empty list
        my $message = "Failed pattern returns empty list";
        $_ = 'aaa';
        @_ = /bbb/;
        is("@_", "", $message);

        @_ = /bbb/g;
        is("@_", "", $message);

        @_ = /(bbb)/;
        is("@_", "", $message);

        @_ = /(bbb)/g;
        is("@_", "", $message);
    }
    {
        my $message = 'ACCEPT and CLOSE - ';
        $_ = "aced";
        #12           3  4  5
        /((a?(*ACCEPT)())())()/
            or die "Failed to match";
        is($1,"a",$message . "buffer 1 is defined with expected value");
        is($2,"a",$message . "buffer 2 is defined with expected value");
        ok(!defined($3),$message . "buffer 3 is not defined");
        ok(!defined($4),$message . "buffer 4 is not defined");
        ok(!defined($5),$message . "buffer 5 is not defined");
        ok(!defined($6),$message . "buffer 6 is not defined");
        $message= 'NO ACCEPT and CLOSE - ';
        /((a?())())()/
            or die "Failed to match";
        is($1,"a",$message . "buffer 1 is defined with expected value");
        is($2,"a",$message . "buffer 2 is defined with expected value");
        is($3,"", $message . "buffer 3 is defined with expected value");
        is($4,"", $message . "buffer 4 is defined with expected value");
        is($5,"",$message . "buffer 5 is defined with expected value");
        ok(!defined($6),$message . "buffer 6 is not defined");
        #12           3  4  5
        $message = 'ACCEPT and CLOSE - ';
        /((a?(*ACCEPT)(c))(e))(d)/
            or die "Failed to match";
        is($1,"a",$message . "buffer 1 is defined with expected value");
        is($2,"a",$message . "buffer 2 is defined with expected value");
        ok(!defined($3),$message . "buffer 3 is not defined");
        ok(!defined($4),$message . "buffer 4 is not defined");
        ok(!defined($5),$message . "buffer 5 is not defined");
        ok(!defined($6),$message . "buffer 6 is not defined");
        $message= 'NO ACCEPT and CLOSE - ';
        /((a?(c))(e))(d)/
            or die "Failed to match";
        is($1,"ace", $message . "buffer 1 is defined with expected value");
        is($2,"ac", $message . "buffer 2 is defined with expected value");
        is($3,"c", $message . "buffer 3 is defined with expected value");
        is($4,"e", $message . "buffer 4 is defined with expected value");
        is($5,"d", $message . "buffer 5 is defined with expected value");
        ok(!defined($6),$message . "buffer 6 is not defined");
    }
    {
        my $message = '@- and @+ and @{^CAPTURE} tests';

        $_= "ace";
        /c(?=.$)/;
        is($#{^CAPTURE}, -1, $message);
        is($#+, 0, $message);
        is($#-, 0, $message);
        is($+ [0], 2, $message);
        is($- [0], 1, $message);
        ok(!defined $+ [1] && !defined $- [1] &&
           !defined $+ [2] && !defined $- [2], $message);

        /a(c)(e)/;
        is($#{^CAPTURE}, 1, $message); # one less than $#-
        is($#+, 2, $message);
        is($#-, 2, $message);
        is($+ [0], 3, $message);
        is($- [0], 0, $message);
        is(${^CAPTURE}[0], "c", $message);
        is($+ [1], 2, $message);
        is($- [1], 1, $message);
        is(${^CAPTURE}[1], "e", $message);
        is($+ [2], 3, $message);
        is($- [2], 2, $message);
        ok(!defined $+ [3] && !defined $- [3] &&
           !defined ${^CAPTURE}[2] && !defined ${^CAPTURE}[3] &&
           !defined $+ [4] && !defined $- [4], $message);

        # Exists has a special check for @-/@+ - bug 45147
        ok(exists $-[0], $message);
        ok(exists $+[0], $message);
        ok(exists ${^CAPTURE}[0], $message);
        ok(exists ${^CAPTURE}[1], $message);
        ok(exists $-[2], $message);
        ok(exists $+[2], $message);
        ok(!exists ${^CAPTURE}[2], $message);
        ok(!exists $-[3], $message);
        ok(!exists $+[3], $message);
        ok(exists ${^CAPTURE}[-1], $message);
        ok(exists ${^CAPTURE}[-2], $message);
        ok(exists $-[-1], $message);
        ok(exists $+[-1], $message);
        ok(exists $-[-3], $message);
        ok(exists $+[-3], $message);
        ok(!exists $-[-4], $message);
        ok(!exists $+[-4], $message);
        ok(!exists ${^CAPTURE}[-3], $message);


        /.(c)(b)?(e)/;
        is($#{^CAPTURE}, 2, $message); # one less than $#-
        is($#+, 3, $message);
        is($#-, 3, $message);
        is(${^CAPTURE}[0], "c", $message);
        is(${^CAPTURE}[2], "e", $message . "[$1 $3]");
        is($+ [1], 2, $message);
        is($- [1], 1, $message);
        is($+ [3], 3, $message);
        is($- [3], 2, $message);
        ok(!defined $+ [2] && !defined $- [2] &&
           !defined $+ [4] && !defined $- [4] &&
           !defined ${^CAPTURE}[1], $message);

        /.(c)/;
        is($#{^CAPTURE}, 0, $message); # one less than $#-
        is($#+, 1, $message);
        is($#-, 1, $message);
        is(${^CAPTURE}[0], "c", $message);
        is($+ [0], 2, $message);
        is($- [0], 0, $message);
        is($+ [1], 2, $message);
        is($- [1], 1, $message);
        ok(!defined $+ [2] && !defined $- [2] &&
           !defined $+ [3] && !defined $- [3] &&
           !defined ${^CAPTURE}[1], $message);

        /.(c)(ba*)?/;
        is($#{^CAPTURE}, 0, $message); # one less than $#-
        is($#+, 2, $message);
        is($#-, 1, $message);

        # Check that values don't stick
        "     "=~/()()()(.)(..)/;
        my($m,$p,$q) = (\$-[5], \$+[5], \${^CAPTURE}[4]);
        () = "$$_" for $m, $p, $q; # FETCH (or eqv.)
        " " =~ /()/;
        is $$m, undef, 'values do not stick to @- elements';
        is $$p, undef, 'values do not stick to @+ elements';
        is $$q, undef, 'values do not stick to @{^CAPTURE} elements';
    }

    foreach ('$+[0] = 13', '$-[0] = 13', '@+ = (7, 6, 5)',
             '${^CAPTURE}[0] = 13',
	     '@- = qw (foo bar)', '$^N = 42') {
	is(eval $_, undef);
        like($@, qr/^Modification of a read-only value attempted/,
	     '$^N, @- and @+ are read-only');
    }

    {
        my $message = '\G testing';
        $_ = 'aaa';
        pos = 1;
        my @a = /\Ga/g;
        is("@a", "a a", $message);

        my $str = 'abcde';
        pos $str = 2;
        unlike($str, qr/^\G/, $message);
        unlike($str, qr/^.\G/, $message);
        like($str, qr/^..\G/, $message);
        unlike($str, qr/^...\G/, $message);
        ok($str =~ /\G../ && $& eq 'cd', $message);
        ok($str =~ /.\G./ && $& eq 'bc', $message);

    }

    {
        my $message = '\G and intuit and anchoring';
	$_ = "abcdef";
	pos = 0;
	ok($_ =~ /\Gabc/, $message);
	ok($_ =~ /^\Gabc/, $message);

	pos = 3;
	ok($_ =~ /\Gdef/, $message);
	pos = 3;
	ok($_ =~ /\Gdef$/, $message);
	pos = 3;
	ok($_ =~ /abc\Gdef$/, $message);
	pos = 3;
	ok($_ =~ /^abc\Gdef$/, $message);
	pos = 3;
	ok($_ =~ /c\Gd/, $message);
	pos = 3;
	ok($_ =~ /..\GX?def/, $message);
    }

    {
        my $s = '123';
        pos($s) = 1;
        my @a = $s =~ /(\d)\G/g; # this infinitely looped up till 5.19.1
        is("@a", "1", '\G looping');
    }


    {
        my $message = 'pos inside (?{ })';
        my $str = 'abcde';
        our ($foo, $bar);
        like($str, qr/b(?{$foo = $_; $bar = pos})c/, $message);
        is($foo, $str, $message);
        is($bar, 2, $message);
        is(pos $str, undef, $message);

        undef $foo;
        undef $bar;
        pos $str = undef;
        ok($str =~ /b(?{$foo = $_; $bar = pos})c/g, $message);
        is($foo, $str, $message);
        is($bar, 2, $message);
        is(pos $str, 3, $message);

        $_ = $str;
        undef $foo;
        undef $bar;
        like($_, qr/b(?{$foo = $_; $bar = pos})c/, $message);
        is($foo, $str, $message);
        is($bar, 2, $message);

        undef $foo;
        undef $bar;
        ok(/b(?{$foo = $_; $bar = pos})c/g, $message);
        is($foo, $str, $message);
        is($bar, 2, $message);
        is(pos, 3, $message);

        undef $foo;
        undef $bar;
        pos = undef;
        1 while /b(?{$foo = $_; $bar = pos})c/g;
        is($foo, $str, $message);
        is($bar, 2, $message);
        is(pos, undef, $message);

        undef $foo;
        undef $bar;
        $_ = 'abcde|abcde';
        ok(s/b(?{$foo = $_; $bar = pos})c/x/g, $message);
        is($foo, 'abcde|abcde', $message);
        is($bar, 8, $message);
        is($_, 'axde|axde', $message);

        # List context:
        $_ = 'abcde|abcde';
        our @res;
        () = /([ace]).(?{push @res, $1,$2})([ce])(?{push @res, $1,$2})/g;
        @res = map {defined $_ ? "'$_'" : 'undef'} @res;
        is("@res", "'a' undef 'a' 'c' 'e' undef 'a' undef 'a' 'c'", $message);

        @res = ();
        () = /([ace]).(?{push @res, $`,$&,$'})([ce])(?{push @res, $`,$&,$'})/g;
        @res = map {defined $_ ? "'$_'" : 'undef'} @res;
        is("@res", "'' 'ab' 'cde|abcde' " .
                     "'' 'abc' 'de|abcde' " .
                     "'abcd' 'e|' 'abcde' " .
                     "'abcde|' 'ab' 'cde' " .
                     "'abcde|' 'abc' 'de'", $message);
    }

    {
        my $message = '\G anchor checks';
        my $foo = 'aabbccddeeffgg';
        pos ($foo) = 1;

	ok($foo =~ /.\G(..)/g, $message);
	is($1, 'ab', $message);

	pos ($foo) += 1;
	ok($foo =~ /.\G(..)/g, $message);
	is($1, 'cc', $message);

	pos ($foo) += 1;
	ok($foo =~ /.\G(..)/g, $message);
	is($1, 'de', $message);

	ok($foo =~ /\Gef/g, $message);

        undef pos $foo;
        ok($foo =~ /\G(..)/g, $message);
        is($1, 'aa', $message);

        ok($foo =~ /\G(..)/g, $message);
        is($1, 'bb', $message);

        pos ($foo) = 5;
        ok($foo =~ /\G(..)/g, $message);
        is($1, 'cd', $message);
    }

    {
        my $message = 'basic \G floating checks';
        my $foo = 'aabbccddeeffgg';
        pos ($foo) = 1;

	ok($foo =~ /a+\G(..)/g, "$message: a+\\G");
	is($1, 'ab', "$message: ab");

	pos ($foo) += 1;
	ok($foo =~ /b+\G(..)/g, "$message: b+\\G");
	is($1, 'cc', "$message: cc");

	pos ($foo) += 1;
	ok($foo =~ /d+\G(..)/g, "$message: d+\\G");
	is($1, 'de', "$message: de");

	ok($foo =~ /\Gef/g, "$message: \\Gef");

        pos ($foo) = 1;

	ok($foo =~ /(?=a+\G)(..)/g, "$message: (?a+\\G)");
	is($1, 'aa', "$message: aa");

        pos ($foo) = 2;

	ok($foo =~ /a(?=a+\G)(..)/g, "$message: a(?=a+\\G)");
	is($1, 'ab', "$message: ab");

    }

    {
        $_ = '123x123';
        my @res = /(\d*|x)/g;
        local $" = '|';
        is("@res", "123||x|123|", "0 match in alternation");
    }

    {
        my $message = "Match against temporaries (created via pp_helem())" .
                         " is safe";
        ok({foo => "bar\n" . $^X} -> {foo} =~ /^(.*)\n/g, $message);
        is($1, "bar", $message);
    }

    {
        my $message = 'package $i inside (?{ }), ' .
                         'saved substrings and changing $_';
        our @a = qw [foo bar];
        our @b = ();
        s/(\w)(?{push @b, $1})/,$1,/g for @a;
        is("@b", "f o o b a r", $message);
        is("@a", ",f,,o,,o, ,b,,a,,r,", $message);

        $message = 'lexical $i inside (?{ }), ' .
                         'saved substrings and changing $_';
        no warnings 'closure';
        my @c = qw [foo bar];
        my @d = ();
        s/(\w)(?{push @d, $1})/,$1,/g for @c;
        is("@d", "f o o b a r", $message);
        is("@c", ",f,,o,,o, ,b,,a,,r,", $message);
    }

    {
        my $message = 'Brackets';
        our $brackets;
        $brackets = qr {
            {  (?> [^{}]+ | (??{ $brackets }) )* }
        }x;

        ok("{{}" =~ $brackets, $message);
        is($&, "{}", $message);
        ok("something { long { and } hairy" =~ $brackets, $message);
        is($&, "{ and }", $message);
        ok("something { long { and } hairy" =~ m/((??{ $brackets }))/, $message);
        is($&, "{ and }", $message);
    }

    {
        $_ = "a-a\nxbb";
        pos = 1;
        ok(!m/^-.*bb/mg, '$_ = "a-a\nxbb"; m/^-.*bb/mg');
    }

    {
        my $message = '\G anchor checks';
        my $text = "aaXbXcc";
        pos ($text) = 0;
        ok($text !~ /\GXb*X/g, $message);
    }

    {
        $_ = "xA\n" x 500;
        unlike($_, qr/^\s*A/m, '$_ = "xA\n" x 500; /^\s*A/m"');

        my $text = "abc dbf";
        my @res = ($text =~ /.*?(b).*?\b/g);
        is("@res", "b b", '\b is not special');
    }

    {
        my $message = '\S, [\S], \s, [\s]';
        my @a = map chr, 0 .. 255;
        my @b = grep m/\S/, @a;
        my @c = grep m/[^\s]/, @a;
        is("@b", "@c", $message);

        @b = grep /\S/, @a;
        @c = grep /[\S]/, @a;
        is("@b", "@c", $message);

        @b = grep /\s/, @a;
        @c = grep /[^\S]/, @a;
        is("@b", "@c", $message);

        @b = grep /\s/, @a;
        @c = grep /[\s]/, @a;
        is("@b", "@c", $message);

        # Test an inverted posix class with a char also in the class.
        my $nbsp = chr utf8::unicode_to_native(0xA0);
        my $non_s = chr utf8::unicode_to_native(0xA1);
        my $pat_string = "[^\\S ]";
        unlike(" ", qr/$pat_string/, "Verify ' ' !~ /$pat_string/");
        like("\t", qr/$pat_string/, "Verify '\\t =~ /$pat_string/");
        unlike($nbsp, qr/$pat_string/, "Verify non-utf8-NBSP !~ /$pat_string/");
        utf8::upgrade($nbsp);
        like($nbsp, qr/$pat_string/, "Verify utf8-NBSP =~ /$pat_string/");
        unlike($non_s, qr/$pat_string/, "Verify non-utf8-inverted-bang !~ /$pat_string/");
        utf8::upgrade($non_s);
        unlike($non_s, qr/$pat_string/, "Verify utf8-inverted-bang !~ /$pat_string/");
    }
    {
        my $message = '\D, [\D], \d, [\d]';
        my @a = map chr, 0 .. 255;
        my @b = grep /\D/, @a;
        my @c = grep /[^\d]/, @a;
        is("@b", "@c", $message);

        @b = grep /\D/, @a;
        @c = grep /[\D]/, @a;
        is("@b", "@c", $message);

        @b = grep /\d/, @a;
        @c = grep /[^\D]/, @a;
        is("@b", "@c", $message);

        @b = grep /\d/, @a;
        @c = grep /[\d]/, @a;
        is("@b", "@c", $message);
    }
    {
        my $message = '\W, [\W], \w, [\w]';
        my @a = map chr, 0 .. 255;
        my @b = grep /\W/, @a;
        my @c = grep /[^\w]/, @a;
        is("@b", "@c", $message);

        @b = grep /\W/, @a;
        @c = grep /[\W]/, @a;
        is("@b", "@c", $message);

        @b = grep /\w/, @a;
        @c = grep /[^\W]/, @a;
        is("@b", "@c", $message);

        @b = grep /\w/, @a;
        @c = grep /[\w]/, @a;
        is("@b", "@c", $message);
    }

    {
        # see if backtracking optimization works correctly
        my $message = 'Backtrack optimization';
        like("\n\n", qr/\n   $ \n/x, $message);
        like("\n\n", qr/\n*  $ \n/x, $message);
        like("\n\n", qr/\n+  $ \n/x, $message);
        like("\n\n", qr/\n?  $ \n/x, $message);
        like("\n\n", qr/\n*? $ \n/x, $message);
        like("\n\n", qr/\n+? $ \n/x, $message);
        like("\n\n", qr/\n?? $ \n/x, $message);
        unlike("\n\n", qr/\n*+ $ \n/x, $message);
        unlike("\n\n", qr/\n++ $ \n/x, $message);
        like("\n\n", qr/\n?+ $ \n/x, $message);
    }

    {
        package S;
        use overload '""' => sub {'Object S'};
        sub new {bless []}

        my $message  = "Ref stringification";
      ::ok(do { \my $v} =~ /^SCALAR/,   "Scalar ref stringification") or diag($message);
      ::ok(do {\\my $v} =~ /^REF/,      "Ref ref stringification") or diag($message);
      ::ok([]           =~ /^ARRAY/,    "Array ref stringification") or diag($message);
      ::ok({}           =~ /^HASH/,     "Hash ref stringification") or diag($message);
      ::ok('S' -> new   =~ /^Object S/, "Object stringification") or diag($message);
    }

    {
        my $message = "Test result of match used as match";
        ok('a1b' =~ ('xyz' =~ /y/), $message);
        is($`, 'a', $message);
        ok('a1b' =~ ('xyz' =~ /t/), $message);
        is($`, 'a', $message);
    }

    {
        my $message = '"1" is not \s';
        warning_is(sub {unlike("1\n" x 102, qr/^\s*\n/m, $message)},
		   undef, "$message (did not warn)");
    }

    {
        my $message = '\s, [[:space:]] and [[:blank:]]';
        my %space = (spc   => " ",
                     tab   => "\t",
                     cr    => "\r",
                     lf    => "\n",
                     ff    => "\f",
        # There's no \v but the vertical tabulator seems miraculously
        # be 11 both in ASCII and EBCDIC.
                     vt    => chr(11),
                     false => "space");

        my @space0 = sort grep {$space {$_} =~ /\s/         } keys %space;
        my @space1 = sort grep {$space {$_} =~ /[[:space:]]/} keys %space;
        my @space2 = sort grep {$space {$_} =~ /[[:blank:]]/} keys %space;

        is("@space0", "cr ff lf spc tab vt", $message);
        is("@space1", "cr ff lf spc tab vt", $message);
        is("@space2", "spc tab", $message);
    }

    {
        my $n= 50;
        # this must be a high number and go from 0 to N, as the bug we are looking for doesn't
        # seem to be predictable. Slight changes to the test make it fail earlier or later.
        foreach my $i (0 .. $n)
        {
            my $str= "\n" x $i;
            ok $str=~/.*\z/, "implicit MBOL check string disable does not break things length=$i";
        }
    }
    {
        # we are actually testing that we dont die when executing these patterns
        use utf8;
        my $e = "Böck";
        ok(utf8::is_utf8($e),"got a unicode string - rt75680");

        ok($e !~ m/.*?[x]$/, "unicode string against /.*?[x]\$/ - rt75680");
        ok($e !~ m/.*?\p{Space}$/i, "unicode string against /.*?\\p{space}\$/i - rt75680");
        ok($e !~ m/.*?[xyz]$/, "unicode string against /.*?[xyz]\$/ - rt75680");
        ok($e !~ m/(.*?)[,\p{isSpace}]+((?:\p{isAlpha}[\p{isSpace}\.]{1,2})+)\p{isSpace}*$/, "unicode string against big pattern - rt75680");
    }
    {
        # we are actually testing that we dont die when executing these patterns
        my $e = "B" . uni_to_native("\x{f6}") . "ck";
        ok(!utf8::is_utf8($e), "got a latin string - rt75680");

        ok($e !~ m/.*?[x]$/, "latin string against /.*?[x]\$/ - rt75680");
        ok($e !~ m/.*?\p{Space}$/i, "latin string against /.*?\\p{space}\$/i - rt75680");
        ok($e !~ m/.*?[xyz]$/,"latin string against /.*?[xyz]\$/ - rt75680");
        ok($e !~ m/(.*?)[,\p{isSpace}]+((?:\p{isAlpha}[\p{isSpace}\.]{1,2})+)\p{isSpace}*$/,"latin string against big pattern - rt75680");
    }

    {
        #
        # Tests for bug 77414.
        #

        my $message = '\p property after empty * match';
        {
            like("1", qr/\s*\pN/, $message);
            like("-", qr/\s*\p{Dash}/, $message);
            like(" ", qr/\w*\p{Blank}/, $message);
        }

        like("1", qr/\s*\pN+/, $message);
        like("-", qr/\s*\p{Dash}{1}/, $message);
        like(" ", qr/\w*\p{Blank}{1,4}/, $message);

    }

    {   # Some constructs with Latin1 characters cause a utf8 string not
        # to match itself in non-utf8
        my $c = uni_to_native("\xc0");
        my $pattern = my $utf8_pattern = qr/(($c)+,?)/;
        utf8::upgrade($utf8_pattern);
        ok $c =~ $pattern, "\\xc0 =~ $pattern; Neither pattern nor target utf8";
        ok $c =~ /$pattern/i, "\\xc0 =~ /$pattern/i; Neither pattern nor target utf8";
        ok $c =~ $utf8_pattern, "\\xc0 =~ $pattern; pattern utf8, target not";
        ok $c =~ /$utf8_pattern/i, "\\xc0 =~ /$pattern/i; pattern utf8, target not";
        utf8::upgrade($c);
        ok $c =~ $pattern, "\\xc0 =~ $pattern; target utf8, pattern not";
        ok $c =~ /$pattern/i, "\\xc0 =~ /$pattern/i; target utf8, pattern not";
        ok $c =~ $utf8_pattern, "\\xc0 =~ $pattern; Both target and pattern utf8";
        ok $c =~ /$utf8_pattern/i, "\\xc0 =~ /$pattern/i; Both target and pattern utf8";
    }

    {   # Make sure can override the formatting
        use feature 'unicode_strings';
        ok uni_to_native("\xc0") =~ /\w/, 'Under unicode_strings: "\xc0" =~ /\w/';
        ok uni_to_native("\xc0") !~ /(?d:\w)/, 'Under unicode_strings: "\xc0" !~ /(?d:\w)/';
    }

    {
        my $str= "\x{100}";
        chop $str;
        my $qr= qr/$str/;
        is("$qr", "(?^:)", "Empty pattern qr// stringifies to (?^:) with unicode flag enabled - Bug #80212");
        $str= "";
        $qr= qr/$str/;
        is("$qr", "(?^:)", "Empty pattern qr// stringifies to (?^:) with unicode flag disabled - Bug #80212");

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
	# RT #3516: \G in a m//g expression causes problems
	my $count = 0;
	while ("abc" =~ m/(\G[ac])?/g) {
	    last if $count++ > 10;
	}
	ok($count < 10, 'RT #3516 A');

	$count = 0;
	while ("abc" =~ m/(\G|.)[ac]/g) {
	    last if $count++ > 10;
	}
	ok($count < 10, 'RT #3516 B');

	$count = 0;
	while ("abc" =~ m/(\G?[ac])?/g) {
	    last if $count++ > 10;
	}
	ok($count < 10, 'RT #3516 C');
    }
    {
        # RT #84294: Is this a bug in the simple Perl regex?
        #          : Nested buffers and (?{...}) dont play nicely on partial matches
        our @got= ();
        ok("ab" =~ /((\w+)(?{ push @got, $2 })){2}/,"RT #84294: Pattern should match");
        my $want= "'ab', 'a', 'b'";
        my $got= join(", ", map { defined($_) ? "'$_'" : "undef" } @got);
        is($got,$want,'RT #84294: check that "ab" =~ /((\w+)(?{ push @got, $2 })){2}/ leaves @got in the correct state');
    }

    {
        # Suppress warnings, as the non-unicode one comes out even if turn off
        # warnings here (because the execution is done in another scope).
        local $SIG{__WARN__} = sub {};
        my $str = "\x{110000}";

        unlike($str, qr/\p{ASCII_Hex_Digit=True}/, "Non-Unicode doesn't match \\p{AHEX=True}");
        like($str, qr/\p{ASCII_Hex_Digit=False}/, "Non-Unicode matches \\p{AHEX=False}");
        like($str, qr/\P{ASCII_Hex_Digit=True}/, "Non-Unicode matches \\P{AHEX=True}");
        unlike($str, qr/\P{ASCII_Hex_Digit=False}/, "Non-Unicode matches \\P{AHEX=FALSE}");
    }

    {
        # Test that IDstart works, but because the author (khw) knows
        # regexes much better than the rest of the core, it is being done here
        # in the context of a regex which relies on buffer names beginng with
        # IDStarts.
        use utf8;
        my $str = "abc";
        like($str, qr/(?<a>abc)/, "'a' is legal IDStart");
        like($str, qr/(?<_>abc)/, "'_' is legal IDStart");
        like($str, qr/(?<ß>abc)/, "U+00DF is legal IDStart");
        like($str, qr/(?<ℕ>abc)/, "U+2115' is legal IDStart");

        # This test works on Unicode 6.0 in which U+2118 and U+212E are legal
        # IDStarts there, but are not Word characters, and therefore Perl
        # doesn't allow them to be IDStarts.  But there is no guarantee that
        # Unicode won't change things around in the future so that at some
        # future Unicode revision these tests would need to be revised.
        foreach my $char ("%", "×", chr(0x2118), chr(0x212E)) {
            my $prog = <<"EOP";
use utf8;;
"abc" =~ qr/(?<$char>abc)/;
EOP
            utf8::encode($prog);
            fresh_perl_like($prog, qr!Group name must start with a non-digit word character!, {},
                        sprintf("'U+%04X not legal IDFirst'", ord($char)));
        }
    }

    { # [perl #101710]
        my $pat = "b";
        utf8::upgrade($pat);
        like("\xffb", qr/$pat/i, "/i: utf8 pattern, non-utf8 string, latin1-char preceding matching char in string");
    }

    { # Crash with @a =~ // warning
	local $SIG{__WARN__} = sub {
             pass 'no crash for @a =~ // warning'
        };
	eval ' sub { my @a =~ // } ';
    }

    { # Concat overloading and qr// thingies
	my @refs;
	my $qr = qr//;
        package Cat {
            require overload;
            overload->import(
		'""' => sub { ${$_[0]} },
		'.' => sub {
		    push @refs, ref $_[1] if ref $_[1];
		    bless $_[2] ? \"$_[1]${$_[0]}" : \"${$_[0]}$_[1]"
		}
            );
	}
	my $s = "foo";
	my $o = bless \$s, Cat::;
	/$o$qr/;
	is "@refs", "Regexp", '/$o$qr/ passes qr ref to cat overload meth';
    }

    {
        my $count=0;
        my $str="\n";
        $count++ while $str=~/.*/g;
        is $count, 2, 'test that ANCH_MBOL works properly. We should get 2 from $count++ while "\n"=~/.*/g';
        my $class_count= 0;
        $class_count++ while $str=~/[^\n]*/g;
        is $class_count, $count, 'while "\n"=~/.*/g and while "\n"=~/[^\n]*/g should behave the same';
        my $anch_count= 0;
        $anch_count++ while $str=~/^.*/mg;
        is $anch_count, 1, 'while "\n"=~/^.*/mg should match only once';
    }

    { # [perl #111174]
        use re '/u';
        my $A_grave = uni_to_native("\xc0");
        like uni_to_native("\xe0"), qr/(?i:$A_grave)/, "(?i: shouldn't lose the passed in /u";
        use re '/a';
        unlike "\x{100}", qr/(?i:\w)/, "(?i: shouldn't lose the passed in /a";
        use re '/aa';
        unlike 'k', qr/(?i:\N{KELVIN SIGN})/, "(?i: shouldn't lose the passed in /aa";
        unlike 'k', qr'(?i:\N{KELVIN SIGN})', "(?i: shouldn't lose the passed in /aa";
    }

    {
	# the test for whether the pattern should be re-compiled should
	# consider the UTF8ness of the previous and current pattern
	# string, as well as the physical bytes of the pattern string

	for my $s (byte_utf8a_to_utf8n("\xc4\x80"), "\x{100}") {
	    ok($s =~ /^$s$/, "re-compile check is UTF8-aware");
	}
    }

    #  #113682 more overloading and qr//
    # when doing /foo$overloaded/, if $overloaded returns
    # a qr/(?{})/ via qr or "" overloading, then 'use re 'eval'
    # shouldn't be required. Via '.', it still is.
    {
        package Qr0;
	use overload 'qr' => sub { qr/(??{50})/ };

        package Qr1;
	use overload '""' => sub { qr/(??{51})/ };

        package Qr2;
	use overload '.'  => sub { $_[1] . qr/(??{52})/ };

        package Qr3;
	use overload '""' => sub { qr/(??{7})/ },
		     '.'  => sub { $_[1] . qr/(??{53})/ };

        package Qr_indirect;
	use overload '""'  => sub { $_[0][0] };

	package main;

	for my $i (0..3) {
	    my $o = bless [], "Qr$i";
	    if ((0,0,1,1)[$i]) {
		eval { "A5$i" =~ /^A$o$/ };
		like($@, qr/Eval-group not allowed/, "Qr$i");
		eval { "5$i" =~ /$o/ };
		like($@, ($i == 3 ? qr/^$/ : qr/no method found,/),
			"Qr$i bare");
		{
		    use re 'eval';
		    ok("A5$i" =~ /^A$o$/, "Qr$i - with use re eval");
		    eval { "5$i" =~ /$o/ };
		    like($@, ($i == 3 ? qr/^$/ : qr/no method found,/),
			    "Qr$i bare - with use re eval");
		}
	    }
	    else {
		ok("A5$i" =~ /^A$o$/, "Qr$i");
		ok("5$i" =~ /$o/, "Qr$i bare");
	    }
	}

	my $o = bless [ bless [], "Qr1" ], 'Qr_indirect';
	ok("A51" =~ /^A$o/, "Qr_indirect");
	ok("51" =~ /$o/, "Qr_indirect bare");
    }

    {   # Various flags weren't being set when a [] is optimized into an
        # EXACTish node
        ok("\x{017F}\x{017F}" =~ qr/^[$sharp_s]?$/i, "[] to EXACTish optimization");
    }

    {   # Test that it avoids spllitting a multi-char fold across nodes.
        # These all fold to things that are like 'ss', which, if split across
        # nodes could fail to match a single character that folds to the
        # combination.  1F0 byte expands when folded;
        my $utf8_locale = find_utf8_ctype_locale();
        for my $char('F', $sharp_s, "\x{1F0}", "\x{FB00}") {
            my $length = 260;    # Long enough to overflow an EXACTFish regnode
            my $p = $char x $length;
            my $s = ($char eq $sharp_s) ? 'ss'
                                        : $char eq "\x{1F0}"
                                          ? "j\x{30c}"
                                          : 'ff';
            $s = $s x $length;
            for my $charset (qw(u d l aa)) {
                for my $utf8 (0..1) {
                    for my $locale ('C', $utf8_locale) {
                      SKIP:
                        {
                            skip "test skipped for non-C locales", 2
                                    if $charset ne 'l'
                                    && (! defined $locale || $locale ne 'C');
                            if ($charset eq 'l') {
                                skip 'Locales not available', 2
                                                            unless $has_locales;
                                if (! defined $locale) {
                                    skip "No UTF-8 locale", 2;
                                }
                                skip "Can't test in miniperl",2
                                  if is_miniperl();

                                require POSIX;
                                POSIX::setlocale(&LC_CTYPE, $locale);
                            }

                            my $pat = $p;
                            utf8::upgrade($pat) if $utf8;
                            my $should_pass =
                              (    $charset eq 'u'
                               || ($charset eq 'd' && $utf8)
                               || ($charset eq 'd' && (   $char =~ /[[:ascii:]]/
                                                       || ord $char > 255))
                               || ($charset eq 'aa' && $char =~ /[[:ascii:]]/)
                               || ($charset eq 'l' && $locale ne 'C')
                               || ($charset eq 'l' && $char =~ /[[:ascii:]]/)
                              );
                            my $name = "(?i$charset), utf8=$utf8, locale=$locale,"
                              . " char=" . sprintf "%x", ord $char;
                            no warnings 'locale';
                            is (eval " '$s' =~ qr/(?i$charset)$pat/;",
                                $should_pass, $name);
                            fail "$name: $@" if $@;
                            is (eval " 'a$s' =~ qr/(?i$charset)a$pat/;",
                                $should_pass, "extra a, $name");
                            fail "$name: $@" if $@;
                        }
                    }
                }
            }
        }
    }

    SKIP:
    {
        skip "no re debug", 5 if is_miniperl;
        my $s = ("0123456789" x 26214) x 2; # Should fill 2 LEXACTS, plus
                                            # small change
        my $pattern_prefix = "use utf8; use re qw(Debug COMPILE)";
        my $pattern = "$pattern_prefix; qr/$s/;";
        my $result = fresh_perl($pattern);
        if ($? != 0) {  # Re-run so as to display STDERR.
            fail($pattern);
            fresh_perl($pattern, { stderr => 0, verbose => 1 });
        }
        like($result, qr/Final program[^X]*\bLEXACT\b[^X]*\bLEXACT\b[^X]*\bEXACT\b[^X]*\bEND\b/s,
             "Check that LEXACT nodes are generated");
        like($s, qr/$s/, "Check that LEXACT nodes match");
        like("a$s", qr/a$s/, "Previous test preceded by an 'a'");
        substr($s, 260000, 1) = "\x{100}";
        $pattern = "$pattern_prefix; qr/$s/;";
        $result = fresh_perl($pattern, { 'wide_chars' => 1 } );
        if ($? != 0) {  # Re-run so as to display STDERR.
            fail($pattern);
            fresh_perl($pattern, { stderr => 0, verbose => 1 });
        }
        like($result, qr/Final program[^X]*\bLEXACT_REQ8\b[^X]*\bLEXACT\b[^X]*\bEXACT\b[^X]*\bEND\b/s,
             "Check that an LEXACT_ONLY node is generated with a \\x{100}");
        like($s, qr/$s/, "Check that LEXACT_REQ8 nodes match");
    }

    {
        for my $char (":", uni_to_native("\x{f7}"), "\x{2010}") {
            my $utf8_char = $char;
            utf8::upgrade($utf8_char);
            my $display = $char;
            $display = display($display);
            my $utf8_display = "utf8::upgrade(\"$display\")";

            like($char, qr/^$char?$/, "\"$display\" =~ /^$display?\$/");
            like($char, qr/^$utf8_char?$/, "my \$p = \"$display\"; utf8::upgrade(\$p); \"$display\" =~ /^\$p?\$/");
            like($utf8_char, qr/^$char?$/, "my \$c = \"$display\"; utf8::upgrade(\$c); \"\$c\" =~ /^$display?\$/");
            like($utf8_char, qr/^$utf8_char?$/, "my \$c = \"$display\"; utf8::upgrade(\$c); my \$p = \"$display\"; utf8::upgrade(\$p); \"\$c\" =~ /^\$p?\$/");
        }
    }

    {
	# #116148: Pattern utf8ness sticks around globally
	# the utf8 in the first match was sticking around for the second
	# match

	use feature 'unicode_strings';

	my $x = "\x{263a}";
	$x =~ /$x/;

	my $text = "Perl";
	ok("Perl" =~ /P.*$/i, '#116148');
    }

    { # 118297: Mixing up- and down-graded strings in regex
        utf8::upgrade(my $u = "\x{e5}");
        utf8::downgrade(my $d = "\x{e5}");
        my $warned;
        local $SIG{__WARN__} = sub { $warned++ if $_[0] =~ /\AMalformed UTF-8/ };
        my $re = qr/$u$d/;
        ok(!$warned, "no warnings when interpolating mixed up-/downgraded strings in pattern");
        my $c = "\x{e5}\x{e5}";
        utf8::downgrade($c);
        like($c, $re, "mixed up-/downgraded pattern matches downgraded string");
        utf8::upgrade($c);
        like($c, $re, "mixed up-/downgraded pattern matches upgraded string");
    }

    {
        # if we have 87 capture buffers defined then \87 should refer to the 87th.
        # test that this is true for 1..100
        # Note that this test causes the engine to recurse at runtime, and
        # hence use a lot of C stack.

        # Compiling for all 100 nested captures blows the stack under
        # clang and ASan; reduce.
        my $max_captures = $Config{ccflags} =~ /sanitize/ ? 20 : 100;

        for my $i (1..100) {
            if ($i > $max_captures) {
                pass("skipping $i buffers under ASan aa");
                pass("skipping $i buffers under ASan aba");
                next;
            }
            my $capture= "a";
            $capture= "($capture)" for 1 .. $i;
            for my $mid ("","b") {
                my $str= "a${mid}a";
                my $backref= "\\$i";
                eval {
                    ok($str=~/$capture$mid$backref/,"\\$i works with $i buffers '$str'=~/...$mid$backref/");
                    1;
                } or do {
                    is("$@","","\\$i works with $i buffers works with $i buffers '$str'=~/...$mid$backref/");
                };
            }
        }
    }

    # this mixture of readonly (not COWable) and COWable strings
    # messed up the capture buffers under COW. The actual test results
    # are incidental; the issue is was an AddressSanitizer failure
    {
	my $c ='AB';
	my $res = '';
	for ($c, 'C', $c, 'DE') {
	    ok(/(.)/, "COWable match");
	    $res .= $1;
	}
	is($res, "ACAD");
    }


    {
	# RT #45667
	# /[#$x]/x didn't interpolate the var $x.
	my $b = 'cd';
	my $s = 'abcd$%#&';
	$s =~ s/[a#$b%]/X/g;
	is ($s, 'XbXX$XX&', 'RT #45667 without /x');
	$s = 'abcd$%#&';
	$s =~ s/[a#$b%]/X/gx;
	is ($s, 'XbXX$XX&', 'RT #45667 with /x');
    }

    {
	no warnings "uninitialized";
	my @a;
	$a[1]++;
	/@a/;
	pass('no crash with /@a/ when array has nonexistent elems');
    }

    {
	is runperl(prog => 'delete $::{qq-\cR-}; //; print qq-ok\n-'),
	   "ok\n",
	   'deleting *^R does not result in crashes';
	no warnings 'once';
	*^R = *caretRglobwithnoscalar;
	"" =~ /(?{42})/;
	is $^R, 42, 'assigning to *^R does not result in a crash';
	is runperl(
	     stderr => 1,
	     prog => 'eval q|'
	            .' q-..- =~ /(??{undef *^R;q--})(?{42})/; '
                    .' print qq-$^R\n-'
	            .'|'
	   ),
	   "42\n",
	   'undefining *^R within (??{}) does not result in a crash';
    }

    SKIP: {   # Test literal range end point special handling
        unless ($::IS_EBCDIC) {
            skip "Valid only for EBCDIC", 24;
        }

        like("\x89", qr/[i-j]/, '"\x89" should match [i-j]');
        unlike("\x8A", qr/[i-j]/, '"\x8A" shouldnt match [i-j]');
        unlike("\x90", qr/[i-j]/, '"\x90" shouldnt match [i-j]');
        like("\x91", qr/[i-j]/, '"\x91" should match [i-j]');

        like("\x89", qr/[i-\N{LATIN SMALL LETTER J}]/, '"\x89" should match [i-\N{LATIN SMALL LETTER J}]');
        unlike("\x8A", qr/[i-\N{LATIN SMALL LETTER J}]/, '"\x8A" shouldnt match [i-\N{LATIN SMALL LETTER J}]');
        unlike("\x90", qr/[i-\N{LATIN SMALL LETTER J}]/, '"\x90" shouldnt match [i-\N{LATIN SMALL LETTER J}]');
        like("\x91", qr/[i-\N{LATIN SMALL LETTER J}]/, '"\x91" should match [i-\N{LATIN SMALL LETTER J}]');

        like("\x89", qr/[i-\N{U+6A}]/, '"\x89" should match [i-\N{U+6A}]');
        unlike("\x8A", qr/[i-\N{U+6A}]/, '"\x8A" shouldnt match [i-\N{U+6A}]');
        unlike("\x90", qr/[i-\N{U+6A}]/, '"\x90" shouldnt match [i-\N{U+6A}]');
        like("\x91", qr/[i-\N{U+6A}]/, '"\x91" should match [i-\N{U+6A}]');

        like("\x89", qr/[\N{U+69}-\N{U+6A}]/, '"\x89" should match [\N{U+69}-\N{U+6A}]');
        unlike("\x8A", qr/[\N{U+69}-\N{U+6A}]/, '"\x8A" shouldnt match [\N{U+69}-\N{U+6A}]');
        unlike("\x90", qr/[\N{U+69}-\N{U+6A}]/, '"\x90" shouldnt match [\N{U+69}-\N{U+6A}]');
        like("\x91", qr/[\N{U+69}-\N{U+6A}]/, '"\x91" should match [\N{U+69}-\N{U+6A}]');

        like("\x89", qr/[i-\x{91}]/, '"\x89" should match [i-\x{91}]');
        like("\x8A", qr/[i-\x{91}]/, '"\x8A" should match [i-\x{91}]');
        like("\x90", qr/[i-\x{91}]/, '"\x90" should match [i-\x{91}]');
        like("\x91", qr/[i-\x{91}]/, '"\x91" should match [i-\x{91}]');

        # Need to use eval, because tries to compile on ASCII platforms even
        # though the tests are skipped, and fails because 0x89-j is an illegal
        # range there.
        like("\x89", eval 'qr/[\x{89}-j]/', '"\x89" should match [\x{89}-j]');
        like("\x8A", eval 'qr/[\x{89}-j]/', '"\x8A" should match [\x{89}-j]');
        like("\x90", eval 'qr/[\x{89}-j]/', '"\x90" should match [\x{89}-j]');
        like("\x91", eval 'qr/[\x{89}-j]/', '"\x91" should match [\x{89}-j]');
    }

    # These are based on looking at the code in regcomp.c
    # We don't look for specific code, just the existence of an SSC
    foreach my $re (qw(     qr/a?c/
                            qr/a?c/i
                            qr/[ab]?c/
                            qr/\R?c/
                            qr/\d?c/d
                            qr/\w?c/l
                            qr/\s?c/a
                            qr/[[:lower:]]?c/u
    )) {
      SKIP: {
        skip "no re-debug under miniperl" if is_miniperl;
        my $prog = <<"EOP";
use re qw(Debug COMPILE);
$re;
EOP
        fresh_perl_like($prog, qr/synthetic stclass/, { stderr=>1 }, "$re generates a synthetic start class");
      }
    }

    {
        like "\x{AA}", qr/a?[\W_]/d, "\\W with /d synthetic start class works";
    }

    SKIP: {
        skip("Tests are ASCII-centric, some would fail on EBCDIC", 12) if $::IS_EBCDIC;

        # Verify that the very last Latin-1 U+00FF
        # (LATIN SMALL LETTER Y WITH DIAERESIS)
        # and its UPPER counterpart (U+0178 which is pure Unicode),
        # and likewise for the very first pure Unicode
        # (LATIN CAPITAL LETTER A WITH MACRON) fold-match properly,
        # and there are no off-by-one logic errors in the transition zone.

        ok("\xFF" =~ /\xFF/i, "Y WITH DIAERESIS l =~ l");
        ok("\xFF" =~ /\x{178}/i, "Y WITH DIAERESIS l =~ u");
        ok("\x{178}" =~ /\xFF/i, "Y WITH DIAERESIS u =~ l");
        ok("\x{178}" =~ /\x{178}/i, "Y WITH DIAERESIS u =~ u");

        # U+00FF with U+05D0 (non-casing Hebrew letter).
        ok("\xFF\x{5D0}" =~ /\xFF\x{5D0}/i, "Y WITH DIAERESIS l =~ l");
        ok("\xFF\x{5D0}" =~ /\x{178}\x{5D0}/i, "Y WITH DIAERESIS l =~ u");
        ok("\x{178}\x{5D0}" =~ /\xFF\x{5D0}/i, "Y WITH DIAERESIS u =~ l");
        ok("\x{178}\x{5D0}" =~ /\x{178}\x{5D0}/i, "Y WITH DIAERESIS u =~ u");

        # U+0100.
        ok("\x{100}" =~ /\x{100}/i, "A WITH MACRON u =~ u");
        ok("\x{100}" =~ /\x{101}/i, "A WITH MACRON u =~ l");
        ok("\x{101}" =~ /\x{100}/i, "A WITH MACRON l =~ u");
        ok("\x{101}" =~ /\x{101}/i, "A WITH MACRON l =~ l");
    }

    {
        use utf8;
        ok("abc" =~ /abc/x, "NEL is white-space under /x");
    }

    {
        ok('a(b)c' =~ qr(a\(b\)c), "'\\(' is a literal in qr(...)");
        ok('a[b]c' =~ qr[a\[b\]c], "'\\[' is a literal in qr[...]");
        ok('a{3}c' =~ qr{a\{3\}c},  # Only failed when { could be a meta
              "'\\{' is a literal in qr{...}, where it could be a quantifier");

        # This one is for completeness
        ok('a<b>c' =~ qr<a\<b\>c>, "'\\<' is a literal in qr<...>)");
    }

    {   # Was getting optimized into EXACT (non-folding node)
        my $x = qr/[x]/i;
        utf8::upgrade($x);
        like("X", qr/$x/, "UTF-8 of /[x]/i matches upper case");
    }

    {   # Special handling of literal-ended ranges in [...] was breaking this
        use utf8;
        like("ÿ", qr/[ÿ-ÿ]/, "\"ÿ\" should match [ÿ-ÿ]");
    }

    {	# [perl #123539]
        like("TffffffffffffTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT5TTTTTTTTTTTTTTTTTTTTTTTTT3TTgTTTTTTTTTTTTTTTTTTTTT2TTTTTTTTTTTTTTTTTTTTTTTHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHiHHHHHHHfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff&ffff", qr/TffffffffffffTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT5TTTTTTTTTTTTTTTTTTTTTTTTT3TTgTTTTTTTTTTTTTTTTTTTTT2TTTTTTTTTTTTTTTTTTTTTTTHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHiHHHHHHHfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff&ffff/il, "");
        like("TffffffffffffT\x{100}TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT5TTTTTTTTTTTTTTTTTTTTTTTTT3TTgTTTTTTTTTTTTTTTTTTTTT2TTTTTTTTTTTTTTTTTTTTTTTHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHiHHHHHHHfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff&ffff", qr/TffffffffffffT\x{100}TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT5TTTTTTTTTTTTTTTTTTTTTTTTT3TTgTTTTTTTTTTTTTTTTTTTTT2TTTTTTTTTTTTTTTTTTTTTTTHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHiHHHHHHHfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff&ffff/il, "");
    }

	{	# [perl #123604]
		my($s, $x, @x) = ('abc', 'a', 'd');
		my $long = 'b' x 2000;
		my $eval = q{$s =~ m{$x[bbb]c} ? 1 : 0};
		$eval =~ s{bbb}{$long};
		my $match = eval $eval;
		ok(1, "did not crash");
		ok($match, "[bbb...] resolved as character class, not subscript");
	}

	{	# [perl #123755]
		for my $pat ('(??', '(?P', '(?i-') {
			eval qq{ qr/$pat/ };
			ok(1, "qr/$pat/ did not crash");
			eval qq{ qr/${pat}\x{123}/ };
			my $e = $@;
			like($e, qr{\x{123}},
				"qr/${pat}x/ shows x in error even if it's a wide character");
		}
	}

	{
		# Expect one of these sizes to cause overflow and wrap to negative
		for my $bits (32, 64) {
			my $wrapneg = 2 ** ($bits - 2) * 3;
			for my $sign ('', '-') {
				my $pat = sprintf "qr/(?%s%u)/", $sign, $wrapneg;
				eval $pat;
				ok(1, "big backref $pat did not crash");
			}
		}
	}
        {
            # Test that we handle qr/\8888888/ and variants without an infinite loop,
            # we use a test within a test so we can todo it, and make sure we don't
            # infinite loop our tests.
            # NOTE - Do not put quotes in the code!
            # NOTE - We have to triple escape the backref in the pattern below.
            my $code='
                BEGIN{require q(./test.pl);}
                watchdog(3);
                for my $len (1 .. 20) {
                    my $eights= q(8) x $len;
                    eval qq{ qr/\\\\$eights/ };
                }
                print q(No infinite loop here!);
            ';
            fresh_perl_is($code, "No infinite loop here!", {},
                "test that we handle things like m/\\888888888/ without infinite loops" );
        }

        SKIP:
        {   # Test that we handle some malformed UTF-8 without looping [perl
            # #123562]
            skip "no Encode", 1 if is_miniperl;
            my $code='
                BEGIN{require q(./test.pl);}
                use Encode qw(_utf8_on);
                # \x80 and \x41 are continuation bytes in their respective
                # character sets
                my $malformed = (ord("A") == 65) ? "a\x80\n" : "a\x41\n";
                utf8::downgrade($malformed);
                _utf8_on($malformed);
                watchdog(3);
                $malformed =~ /(\n\r|\r)$/;
                print q(No infinite loop here!);
            ';
            fresh_perl_like($code, qr/Malformed UTF-8 character/, {},
                "test that we handle some UTF-8 malformations without looping" );
        }

	{
		# [perl #123843] hits SEGV trying to compile this pattern
		my $match;
		eval q{ ($match) = ("xxyxxyxy" =~ m{(x+(y(?1))*)}) };
		ok(1, "compiled GOSUB in CURLYM ok");
		is($match, 'xxyxxyx', "matched GOSUB in CURLYM");
	}

	{
		# [perl #123852] doesn't avoid all the capture-related work with
		# //n, leading to possible memory corruption
		eval q{ qr{()(?1)}n };
		my $error = $@;
		ok(1, "qr{()(?1)}n didn't crash");
		like($error, qr{Reference to nonexistent group},
				'gave appropriate error for qr{()(?1)}n');
	}

	{
            # [perl #126406] panic with unmatchable quantifier
            my $code='
                no warnings "regexp";
                "" =~ m/(.0\N{6,0}0\N{6,0}000000000000000000000000000000000)/;
            ';
            fresh_perl_is($code, "", {},
                            "perl [#126406] panic");
	}
        {
            my $bug="[perl #126182]"; # test for infinite pattern recursion
            for my $tuple (
                    [ 'q(a)=~/(.(?2))((?<=(?=(?1)).))/', "died", "look ahead left recursion fails fast" ],
                    [ 'q(aa)=~/(?R)a/', "died", "left-recursion fails fast", ],
                    [ 'q(bbaa)=~/(?&x)(?(DEFINE)(?<x>(?&y)*a)(?<y>(?&x)*b))/',
                        "died", "inter-cyclic optional left recursion dies" ],
                    [ 'q(abc) =~ /a((?1)?)c/', "died", "optional left recursion dies" ],
                    [ 'q(abc) =~ /a((?1)??)c/', "died", "min mod left recursion dies" ],
                    [ 'q(abc) =~ /a((?1)*)c/', "died", "* left recursion dies" ],
                    [ 'q(abc) =~ /a((?1)+)c/', "died", "+ left recursion dies" ],
                    [ 'q(abc) =~ /a((?1){0,3})c/', "died", "{0,3} left recursion fails fast" ],

                    [ 'q(aaabbb)=~/a(?R)?b/', "matched", "optional self recursion works" ],
                    [ '"((5maa-maa)(maa-3maa))" =~ /(\\\\((?:[^()]++|(?0))*+\\\\))/', "matched",
                        "recursion and possessive captures", "((5maa-maa)(maa-3maa))"],
                    [ '"((5maa-maa)(maa-3maa))" =~ /(\\\\((?:[^()]++|(?1))*+\\\\))/', "matched",
                        "recursion and possessive captures", "((5maa-maa)(maa-3maa))"],
                    [ '"((5maa-maa)(maa-3maa))" =~ /(\\\\((?:[^()]+|(?0))*\\\\))/', "matched",
                        "recursion and possessive captures", "((5maa-maa)(maa-3maa))"],
                    [ '"((5maa-maa)(maa-3maa))" =~ /(\\\\((?:[^()]+|(?1))*\\\\))/', "matched",
                        "recursion and possessive captures", "((5maa-maa)(maa-3maa))"],
            ) {
                my ($expr, $expect, $test_name, $cap1)= @$tuple;
                # avoid quotes in this code!
                my $code='
                    BEGIN{require q(./test.pl);}
                    watchdog(3);
                    my $status= eval(q{ !(' . $expr . ') ? q(failed) : ' .
                        ($cap1 ? '($1 ne q['.$cap1.']) ? qq(badmatch:$1) : ' : '') .
                        ' q(matched) })
                                || ( ( $@ =~ /Infinite recursion/ ) ? qq(died) : q(strange-death) );
                    print $status;
                ';
                fresh_perl_is($code, $expect, {}, "$bug - $test_name" );
            }
        }
        {
            my $is_cygwin = $^O eq "cygwin";
            local $::TODO = "this flaps on github cygwin vm, but not on cygwin iron #18129"
              if $is_cygwin;
            my $expected = "Timeout";
            my $code = '
                BEGIN{require q(test.pl);}
                watchdog(3);
                $SIG{ALRM} = sub {print "'.$expected.'\n"; exit(1)};
                alarm 1;
                $_ = "a" x 1000 . "b" x 1000 . "c" x 1000;
                /.*a.*b.*c.*[de]/;
                print "increase the multipliers in the regex above to run the regex longer";
            ';
            # this flaps on github cygwin vm, but not on cygwin iron #18129
            # so on cygwin it's run for 50 seconds to see if it fails eventually
            my $max = $is_cygwin ? 50 : 1;
            my ($iter, $result, $status);
            for my $i (1..$max) {
                $iter = $i;
                $result = fresh_perl($code,{});
                $status = $?;
                last if $result ne $expected;
            }
            is($result, $expected, "Test Perl 73464")
              or diag "PROG:", $code, "STATUS:", $status, "failed on iteration: $iter";
        }

        {   # [perl #128686], crashed the the interpreter
            my $AE = chr utf8::unicode_to_native(0xC6);
            my $ae = chr utf8::unicode_to_native(0xE6);
            my $re = qr/[$ae\s]/i;
            ok($AE !~ $re, '/[\xE6\s]/i doesn\'t match \xC6 when not in UTF-8');
            utf8::upgrade $AE;
            ok($AE =~ $re, '/[\xE6\s]/i matches \xC6 when in UTF-8');
        }

        {
            is(0+("\n" =~ m'\n'), 1, q|m'\n' should interpolate escapes|);
        }

        {
            my $str = "a\xB6";
            ok( $str =~ m{^(a|a\x{b6})$}, "fix [perl #129950] - latin1 case" );
            utf8::upgrade($str);
            ok( $str =~ m{^(a|a\x{b6})$}, "fix [perl #129950] - utf8 case" );
        }
        {
            my $got= run_perl( switches => [ '-l' ], prog => <<'EOF_CODE' );
            my $died= !eval {
                $_=qq(ab);
                print;
                my $p=qr/(?{ s!!x! })/;
                /$p/;
                print;
                /a/;
                /$p/;
                print;
                /b/;
                /$p/;
                print;
                //;
                1;
            };
            $error = $died ? ($@ || qq(Zombie)) : qq(none);
            print $died ? qq(died) : qq(lived);
            print qq(Error: $@);
EOF_CODE
            my @got= split /\n/, $got;
            is($got[0],"ab","empty pattern in regex codeblock: got expected start string");
            is($got[1],"xab",
                "empty pattern in regex codeblock: first subst with no last-match worked right");
            is($got[2],"xxb","empty pattern in regex codeblock: second subst worked right");
            is($got[3],"xxx","empty pattern in regex codeblock: third subst worked right");
            is($got[4],"died","empty pattern in regex codeblock: died as expected");
            like($got[5],qr/Error: Infinite recursion via empty pattern/,
           "empty pattern in regex codeblock: produced the right exception message" );
        }

    # This test is based on the one directly above, which happened to
    # leak. Repeat the test, but stripped down to the bare essentials
    # of the leak, which is to die while executing a regex which is
    # already the current regex, thus causing the saved outer set of
    # capture offsets to leak. The test itself doesn't do anything
    # except sit around hoping not to be triggered by ASan
    {
        eval {
            my $s = "abcd";
            $s =~ m{([abcd]) (?{ die if $1 eq 'd'; })}gx;
            $s =~ //g;
            $s =~ //g;
            $s =~ //g;
        };
        pass("call to current regex doesn't leak");
    }



    {
        # [perl #130495] /x comment skipping stopped a byte short, leading
        # to assertion failure or 'malformed utf-8 character" warning
        fresh_perl_is(
            "use utf8; m{a#\x{124}}x", '', {wide_chars => 1},
            '[perl #130495] utf-8 character at end of /x comment should not misparse',
        );
    }
    {
        # [perl #130522] causes out-of-bounds read detected by clang with
        # address=sanitized when length of the STCLASS string is greater than
        # length of target string.
        my $re = qr{(?=\0z)\0?z?$}i;
        my($yes, $no) = (1, "");
        for my $test (
            [ $no,  undef,   '<undef>' ],
            [ $no,  '',      '' ],
            [ $no,  "\0",    '\0' ],
            [ $yes, "\0z",   '\0z' ],
            [ $no,  "\0z\0", '\0z\0' ],
            [ $yes, "\0z\n", '\0z\n' ],
        ) {
            my($result, $target, $disp) = @$test;
            no warnings qw/uninitialized/;
            is($target =~ $re, $result, "[perl #130522] with target '$disp'");
        }
    }
    {
	# [perl #129377] backref to an unmatched capture should not cause
	# reading before start of string.
	SKIP: {
	    skip "no re-debug under miniperl" if is_miniperl;
	    my $prog = <<'EOP';
use re qw(Debug EXECUTE);
"x" =~ m{ () y | () \1 }x;
EOP
	    fresh_perl_like($prog, qr{
		\A (?! .* ^ \s+ - )
	    }msx, { stderr => 1 }, "Offsets in debug output are not negative");
	}
    }
    {
        # buffer overflow

        # This test also used to leak - fixed by the commit which added
        # this line.

        fresh_perl_is("BEGIN{\$^H=0x200000}\ns/[(?{//xx",
                      "Unmatched [ in regex; marked by <-- HERE in m/[ <-- HERE (?{/ at (eval 1) line 1.\n",
                      {}, "buffer overflow for regexp component");
    }
    {
        # [perl #129281] buffer write overflow, detected by ASAN, valgrind
        fresh_perl_is('/0(?0)|^*0(?0)|^*(^*())0|/', '', {}, "don't bump whilem_c too much");
    }
    {
        # RT #131893 - fails with ASAN -fsanitize=undefined
        fresh_perl_is('qr/0(0?(0||00*))|/', '', {}, "integer overflow during compilation");
    }

    {
        # RT #131575 intuit skipping back from the end to find the highest
        # possible start point, was potentially hopping back beyond pos()
        # and crashing by calling fbm_instr with a negative length

        my $text = "=t=\x{5000}";
        pos($text) = 3;
        ok(scalar($text !~ m{(~*=[a-z]=)}g), "RT #131575");
    }
    {
        fresh_perl_is('"AA" =~ m/AA{1,0}/','',{},"handle OPFAIL insert properly");
    }
    {
        fresh_perl_is('$_="0\x{1000000}";/^000?\0000/','',{},"dont throw assert errors trying to fbm past end of string");
    }
    {   # [perl $132227]
        fresh_perl_is("('0ba' . ('ss' x 300)) =~ m/0B\\N{U+41}" . $sharp_s x 150 . '/i and print "1\n"',  1,{},"Use of sharp s under /di that changes to /ui");

        # A variation, but as far as khw knows not part of 132227
        fresh_perl_is("'0bssa' =~ m/0B" . $sharp_s . "\\N{U+41}" . '/i and print "1\n"',  1,{},"Use of sharp s under /di that changes to /ui");
    }
    {   # [perl $132164]
        fresh_perl_is('m m0*0+\Rm', "",{},"Undefined behavior in address sanitizer");
    }
    {   # [perl #133642]
        fresh_perl_is('no warnings "experimental::vlb";
                      m/((?<=(0?)))/', "",{},"Was getting 'Double free'");
    }
    {   # [perl #133782]
        # this would panic on DEBUGGING builds
        fresh_perl_is(<<'CODE', "ok\nok\n",{}, 'Bad length magic was left on $^R');
while( "\N{U+100}bc" =~ /(..?)(?{$^N})/g ) {
  print "ok\n" if length($^R)==length("$^R");
}
CODE
    }
    {   # [perl #133871], ASAN/valgrind out-of-bounds access
        fresh_perl_like('qr/(?|(())|())|//', qr/syntax error/, {}, "[perl #133871]");
    }
    {   # [perl #133871], ASAN/valgrind out-of-bounds access
        fresh_perl_like('qr/\p{nv:NAnq}/', qr/Can't find Unicode property definition/, {}, "GH #17367");
    }
    {   # GH #17370, ASAN/valgrind out-of-bounds access
        fresh_perl_like('qr/\p{nv:qnan}/', qr/Can't find Unicode property definition/, {}, "GH #17370");
    }
    {   # GH #17371, segfault
        fresh_perl_like('qr/\p{nv=\\\\\}(?0)|\337ss|\337ss//', qr/Unicode property wildcard not terminated/, {}, "GH #17371");
    }
    {   # GH #17384, ASAN/valgrind out-of-bounds access
        fresh_perl_like('"q0" =~ /\p{__::Is0}/', qr/Unknown user-defined property name \\p\{__::Is0}/, {}, "GH #17384");
    }

  SKIP:
    {   # [perl #133921], segfault
        skip "Not valid for EBCDIC", 5 if $::IS_EBCDIC;

        fresh_perl_is('qr0||ß+p00000F00000ù\Q00000ÿ00000x00000x0c0e0\Qx0\Qx0\x{0c!}\;\;î0\x        fresh_perl_is('|ß+W0ü0r0\Qx0\Qx0x0c0G00000000000000000O000000000x0x0x0c!}\;îçÿù\Q0 \x
fresh_perl_is('s|ß+W0ü0f0\Qx0\Qx0x0c0G0xgive0000000000000O0h000x0 \xòÿÿÿ



	ç















x{0c!}\;\;çÿ 
        fresh_perl_is('a aú




	ç















x{1c!}\;\;îçÿp 
    fresh_perl_is('s|ß+W0ü0f0\Qx0\Qx0x0c0g0c 000n0000000000000O0h000x0 \xòÿÿÿ




	ç















x{0c!}\;\;îçÿ     }

    {   # perl #133998]
        fresh_perl_is('print "\x{110000}" =~ qr/(?l)|[^\S\pC\s]/', 1, {},
        '/[\S\s]/l works');
    }

    {   # perl #133995]
        use utf8;
        fresh_perl_is('"έδωσαν ελληνικήვე" =~ m/[^0](?=0)0?/', "",
                      {wide_chars => 1},
                      '[^0] doesnt crash on UTF-8 target string');
    }

    {   # [perl #133992]  This is a tokenizer bug of parsing a pattern
        fresh_perl_is(q:$z = do {
                                use utf8;
                                "q!ÑÐµÑÑ! =~ m'"
                        };
                        $z .= 'è(?#';
                        $z .= "'";
                        eval $z;:, "", {}, 'foo');
    }

    {   # [perl #134325]
        my $quote="\\Q";
        my $back="\\\\";
        my $ff="\xff";
        my $s = sprintf "/\\1|(|%s)%s%s   /i",
                        $quote x 8 . $back x 69,
                        $quote x 5 . $back x 4,
                        $ff x 48;
        like(fresh_perl("$s", { stderr => 1, }), qr/Unmatched \(/);
   }

   {    # GitHub #17196, caused assertion failure
        fresh_perl_like('("0" x 258) =~ /(?l)0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000/',
                        qr/^$/,
                        {},
                        "Assertion failure with /l exact string longer than a single node");
    }

SKIP:
    {   # [perl #134334], Assertion failure
        my $utf8_locale = find_utf8_ctype_locale();
        skip "no UTF-8 locale available" unless $utf8_locale;
        fresh_perl_like("use POSIX; POSIX::setlocale(&LC_CTYPE, '$utf8_locale'); 'ssss' =~ /\xDF+?sX/il;",
                        qr/^$/,
                        {},
                        "Assertion failure matching /il on single char folding to multi");
    }

    {   # Test ANYOFHs
        my $pat = qr/[\x{4000001}\x{4000003}\x{4000005}]+/;
        unlike("\x{4000000}", $pat, "4000000 isn't in pattern");
        like("\x{4000001}", $pat, "4000001 is in pattern");
        unlike("\x{4000002}", $pat, "4000002 isn't in pattern");
        like("\x{4000003}", $pat, "4000003 is in pattern");
        unlike("\x{4000004}", $pat, "4000004 isn't in pattern");
        like("\x{4000005}", $pat, "4000005 is in pattern");
        unlike("\x{4000006}", $pat, "4000006 isn't in pattern");

        # gh #17319
        $pat = qr/[\N{U+200D}\N{U+2000}]()/;
        unlike("\x{1FFF}", $pat, "1FFF isn't in pattern");
        like("\x{2000}", $pat, "2000 is in pattern");
        unlike("\x{2001}", $pat, "2001 isn't in pattern");
        unlike("\x{200C}", $pat, "200C isn't in pattern");
        like("\x{200D}", $pat, "200 is in pattern");
        unlike("\x{200E}", $pat, "200E isn't in pattern");
    }

    # gh17490: test recursion check
    {
        my $eval = '(?{1})';
        my $re = sprintf '(?&FOO)(?(DEFINE)(?<FOO>%sfoo))', $eval x 20;
        my $result = eval qq{"foo" =~ /$re/};
        is($@ // '', '', "many evals did not die");
        ok($result, "regexp correctly matched");
    }

    # gh16947: test regexp corruption (GOSUB)
    {
        fresh_perl_is(q{
            'xy' =~ /x(?0)|x(?|y|y)/ && print 'ok'
        }, 'ok', {}, 'gh16947: test regexp corruption (GOSUB)');
    }
    # gh16947: test fix doesn't break SUSPEND
    {
        fresh_perl_is(q{ 'sx' =~ m{ss++}i; print 'ok' },
                'ok', {}, "gh16947: test fix doesn't break SUSPEND");
    }

    # gh17730: should not crash
    {
        fresh_perl_is(q{
            "q00" =~ m{(((*ACCEPT)0)*00)?0(?1)}; print "ok"
        }, 'ok', {}, 'gh17730: should not crash');
    }

    # gh17743: more regexp corruption via GOSUB
    {
        fresh_perl_is(q{
            "0" =~ /((0(?0)|000(?|0000|0000)(?0))|)/; print "ok"
        }, 'ok', {}, 'gh17743: test regexp corruption (1)');

        fresh_perl_is(q{
            "000000000000" =~ /(0(())(0((?0)())|000(?|\x{ef}\x{bf}\x{bd}|\x{ef}\x{bf}\x{bd}))|)/;
            print "ok"
        }, 'ok', {}, 'gh17743: test regexp corruption (2)');
    }

    {
        # Test branch reset (?|...|...) in list context. This was reported
        # in GH Issue #20710, in relation to breaking App::pl. See
        # https://github.com/Perl/perl5/issues/20710#issuecomment-1404549785
        my $ok = 0;
        my ($w,$x,$y,$z);
        $ok = ($x,$y) = "ab"=~/(?|(p)(q)|(x)(y)|(a)(b))/;
        ok($ok,"Branch reset pattern 1 matched as expected");
        is($x,"a","Branch reset in list context check 1 (a)");
        is($y,"b","Branch reset in list context check 2 (b)");

        $ok = ($x,$y,$z) = "xyz"=~/(?|(p)(q)|(x)(y)|(a)(b))(z)/;
        ok($ok,"Branch reset pattern 2 matched as expected");
        is($x,"x","Branch reset in list context check 3 (x)");
        is($y,"y","Branch reset in list context check 4 (y)");
        is($z,"z","Branch reset in list context check 5 (z)");

        $ok = ($w,$x,$y) = "wpq"=~/(w)(?|(p)(q)|(x)(y)|(a)(b))/;
        ok($ok,"Branch reset pattern 3 matched as expected");
        is($w,"w","Branch reset in list context check 6 (w)");
        is($x,"p","Branch reset in list context check 7 (p)");
        is($y,"q","Branch reset in list context check 8 (q)");

        $ok = ($w,$x,$y,$z) = "wabz"=~/(w)(?|(p)(q)|(x)(y)|(a)(b))(z)/;
        ok($ok,"Branch reset pattern 4 matched as expected");
        is($w,"w","Branch reset in list context check 9  (w)");
        is($x,"a","Branch reset in list context check 10 (a)");
        is($y,"b","Branch reset in list context check 11 (b)");
        is($z,"z","Branch reset in list context check 12 (z)");
    }
    {
        # Test for GH Issue #20826. Save stack overflow introduced in
        # 92373dea9d7bcc0a017f20cb37192c1d8400767f PR #20530.
        # Note this test depends on an assert so it will only fail
        # under DEBUGGING.
        fresh_perl_is(q{
            $_ = "x" x 1000;
            my $pat = '(.)' x 200;
            $pat = qr/($pat)+/;
            m/$pat/;
            print "ok";
        }, 'ok', {}, 'gh20826: test regex save stack overflow');
    }
    {
        my ($x, $y);
        ok( "aaa" =~ /(?:(a)?\1)+/,
            "GH Issue #18865 'aaa' - pattern matches");
        $x = "($-[0],$+[0])";
        ok( "aaa" =~ /(?:((?{})a)?\1)+/,
            "GH Issue #18865 'aaa' - deoptimized pattern matches");
        $y = "($-[0],$+[0])";
        {
            local $::TODO = "Not Yet Implemented";
            is( $y, $x,
                "GH Issue #18865 'aaa' - test optimization");
        }
        ok( "ababab" =~ /(?:(?:(ab))?\1)+/,
            "GH Issue #18865 'ababab' - pattern matches");
        $x = "($-[0],$+[0])";
        ok( "ababab" =~ /(?:(?:((?{})ab))?\1)+/,
            "GH Issue #18865 'ababab' - deoptimized pattern matches");
        $y = "($-[0],$+[0])";
        {
            local $::TODO = "Not Yet Implemented";
            is( $y, $x,
                "GH Issue #18865 'ababab' - test optimization");
        }
        ok( "XaaXbbXb" =~ /(?:X([ab])?\1)+/,
            "GH Issue #18865 'XaaXbbXb' - pattern matches");
        $x = "($-[0],$+[0])";
        ok( "XaaXbbXb" =~ /(?:X((?{})[ab])?\1)+/,
            "GH Issue #18865 'XaaXbbXb' - deoptimized pattern matches");
        $y = "($-[0],$+[0])";
        {
            local $::TODO = "Not Yet Implemented";
            is( $y, $x,
                "GH Issue #18865 'XaaXbbXb' - test optimization");
        }
    }
    {
        # Test that ${^LAST_SUCCESSFUL_PATTERN} works as expected.
        # It should match like the empty pattern does, and it should be dynamic
        # in the same was as $1 is dynamic.
        my ($str,$pat);
        $str = "ABCD";
        $str =~/(D)/;
        is("$1", "D", '$1 is "D"');
        $pat = "${^LAST_SUCCESSFUL_PATTERN}";
        is($pat, "(?^:(D))", 'Outer ${^LAST_SUCCESSFUL_PATTERN} is as expected');
        {
            if ($str=~/BX/ || $str=~/(BC)/) {
                is("$1", "BC",'$1 is now "BC"');
                $pat = "${^LAST_SUCCESSFUL_PATTERN}";
                ok($str =~ s//ZZ/, "Empty pattern matched as expected");
                is($str, "AZZD", "Empty pattern in s/// has result we expected");
            }
        }
        is("$1", "D", '$1 should now be "D" again');
        is($pat, "(?^:(BC))", 'inner ${^LAST_SUCCESSFUL_PATTERN} is as expected');
        ok($str=~s//Q/, 'Empty pattern to "Q" was successful');
        is($str, "AZZQ", "Empty pattern in s/// has result we expected (try2)");
        $pat = "${^LAST_SUCCESSFUL_PATTERN}";
        is($pat, "(?^:(D))", 'Outer ${^LAST_SUCCESSFUL_PATTERN} restored to its previous value as expected');

        $str = "ABCD";
        {
            if ($str=~/BX/ || $str=~/(BC)/) {
                is("$1", "BC",'$1 is now "BC"');
                $pat = "${^LAST_SUCCESSFUL_PATTERN}";
                ok($str=~s/${^LAST_SUCCESSFUL_PATTERN}/ZZ/, '${^LAST_SUCCESSFUL_PATTERN} matched as expected');
                is($str, "AZZD", '${^LAST_SUCCESSFUL_PATTERN} in s/// has result we expected');
            }
        }
        is("$1", "D", '$1 should now be "D" again');
        is($pat, "(?^:(BC))", 'inner ${^LAST_SUCCESSFUL_PATTERN} is as expected');
        is($str, "AZZD", 'Using ${^LAST_SUCCESSFUL_PATTERN} as a pattern has same result as empty pattern');
        ok($str=~s/${^LAST_SUCCESSFUL_PATTERN}/Q/, '${^LAST_SUCCESSFUL_PATTERN} to "Q" was successful');
        is($str, "AZZQ", '${^LAST_SUCCESSFUL_PATTERN} in s/// has result we expected');
        ok($str=~/ZQ/, "/ZQ/ matched as expected");
        $pat = "${^LAST_SUCCESSFUL_PATTERN}";
        is($pat, "(?^:ZQ)", '${^LAST_SUCCESSFUL_PATTERN} changed as expected');

        $str = "foobarfoo";
        ok($str =~ s/foo//, "matched foo");
        my $copy= ${^LAST_SUCCESSFUL_PATTERN};
        ok(defined($copy), '$copy is defined');
        ok($str =~ s/bar//,"matched bar");
        ok($str =~ s/$copy/PQR/, 'replaced $copy with PQR');
        is($str, "PQR", 'final string should be PQR');
    }
} # End of sub run_tests

1;

#
# ex: set ts=8 sts=4 sw=4 et:
#
