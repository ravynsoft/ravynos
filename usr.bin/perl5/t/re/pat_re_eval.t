#!./perl
#
# This is a home for regular expression tests that don't fit into
# the format supported by re/regexp.t.  If you want to add a test
# that does fit that format, add it to re/re_tests, not here.

use strict;
use warnings;
use Config;
use 5.010;


sub run_tests;

$| = 1;


BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
	require './charset_tools.pl';
}

our @global;


plan tests => 527;  # Update this when adding/deleting tests.

run_tests() unless caller;

# test that runtime code without 'use re eval' is trapped

sub norun {
    like($@, qr/Eval-group not allowed at runtime/, @_);
}

#
# Tests start here.
#
sub run_tests {
    {
        my $message =  "Call code from qr //";
        local $_ = 'var="foo"';
        $a = qr/(?{++$b})/;
        $b = 7;
        ok(/$a$a/ && $b eq '9', $message);

        my $c="$a";
        ok(/$a$a/ && $b eq '11', $message);

        undef $@;
        eval {/$c/};
	norun("$message norun 1");


        {
	    eval {/$a$c$a/};
	    norun("$message norun 2");
	    use re "eval";
	    /$a$c$a/;
	    is($b, '14', $message);
	}

        our $lex_a = 43;
        our $lex_b = 17;
        our $lex_c = 27;
        my $lex_res = ($lex_b =~ qr/$lex_b(?{ $lex_c = $lex_a++ })/);

        is($lex_res, 1, $message);
        is($lex_a, 44, $message);
        is($lex_c, 43, $message);

        undef $@;
        my $d = '(?{1})';
        my $match = eval { /$a$c$a$d/ };
        ok($@ && $@ =~ /Eval-group not allowed/ && !$match, $message);
        is($b, '14', $message);

        $lex_a = 2;
        $lex_a = 43;
        $lex_b = 17;
        $lex_c = 27;
        $lex_res = ($lex_b =~ qr/17(?{ $lex_c = $lex_a++ })/);

        is($lex_res, 1, $message);
        is($lex_a, 44, $message);
        is($lex_c, 43, $message);

    }

    {
        our $a = bless qr /foo/ => 'Foo';
        ok 'goodfood' =~ $a,     "Reblessed qr // matches";
        is($a, '(?^:foo)', "Reblessed qr // stringifies");
        my $x = "\x{3fe}";
        my $z = my $y = byte_utf8a_to_utf8n("\317\276");  # Byte representation
                                                          # of $x
        $a = qr /$x/;
        ok $x =~ $a, "UTF-8 interpolation in qr //";
        ok "a$a" =~ $x, "Stringified qr // preserves UTF-8";
        ok "a$x" =~ /^a$a\z/, "Interpolated qr // preserves UTF-8";
        ok "a$x" =~ /^a(??{$a})\z/,
                        "Postponed interpolation of qr // preserves UTF-8";


        is(length qr /##/x, 9, "## in qr // doesn't corrupt memory; Bug 17776");

        {
            ok "$x$x" =~ /^$x(??{$x})\z/,
               "Postponed UTF-8 string in UTF-8 re matches UTF-8";
            ok "$y$x" =~ /^$y(??{$x})\z/,
               "Postponed UTF-8 string in non-UTF-8 re matches UTF-8";
            ok "$y$x" !~ /^$y(??{$y})\z/,
               "Postponed non-UTF-8 string in non-UTF-8 re doesn't match UTF-8";
            ok "$x$x" !~ /^$x(??{$y})\z/,
               "Postponed non-UTF-8 string in UTF-8 re doesn't match UTF-8";
            ok "$y$y" =~ /^$y(??{$y})\z/,
               "Postponed non-UTF-8 string in non-UTF-8 re matches non-UTF8";
            ok "$x$y" =~ /^$x(??{$y})\z/,
               "Postponed non-UTF-8 string in UTF-8 re matches non-UTF8";

            $y = $z;  # Reset $y after upgrade.
            ok "$x$y" !~ /^$x(??{$x})\z/,
               "Postponed UTF-8 string in UTF-8 re doesn't match non-UTF-8";
            ok "$y$y" !~ /^$y(??{$x})\z/,
               "Postponed UTF-8 string in non-UTF-8 re doesn't match non-UTF-8";
        }
    }
    {
        our $this_counter;
        ok( "ABDE" =~ /(A(A|B(*ACCEPT)|C)+D)(E)(?{ $this_counter++ })/,
            "ACCEPT/CURLYX/EVAL - pattern should match");
        is( "$1-$2", "AB-B",
            "Make sure that ACCEPT works in CURLYX by using EVAL");
    }
    {
        ok( "AB"=~/(A)(?(*{ 1 })B|C)/, "(?(*{ ... })yes|no) works as expected");
        ok( "AC"=~/(A)(?(*{ 0 })B|C)/, "(?(*{ ... })yes|no) works as expected");
    }

    {
        # Test if $^N and $+ work in (*{ }) (optimistic eval)
        our @ctl_n = ();
        our @plus = ();
        my $nested_tags = qr{
          (?<nested_tags>
            <
                ((\w)+)
                (*{
                       push @ctl_n, (defined $^N ? $^N : "undef");
                       push @plus, (defined $+ ? $+ : "undef");
                })
            >
            (?&nested_tags)*
            </\s* \w+ \s*>
          )
        }x;

        # note the results of this may change from perl to perl as different optimisations
        # are added or enabled. It is testing that things *work*, not that they produce
        # a specific output. The whole idea of optimistic eval is to have an eval that
        # does not disable optimizations in the way a normal eval does.
        my $c = 0;
        for my $test (
            # Test structure:
            #  [ Expected result, Regex, Expected value(s) of $^N, Expected value(s) of $+, "note" ]
            [ 1, qr#^$nested_tags$#, "bla blubb <bla><blubb></blubb></bla>", "a b a" ],
            [ 1, qr#^($nested_tags)$#, "bla blubb <bla><blubb></blubb></bla>", "a b a" ],
            [ 1, qr#^(|)$nested_tags$#, "bla blubb <bla><blubb></blubb></bla>", "a b a" ],
            [ 1, qr#^(?:|)$nested_tags$#, "bla blubb <bla><blubb></blubb></bla>", "a b a" ],
            [ 1, qr#^<(bl|bla)>$nested_tags<(/\1)>$#, "blubb /bla", "b /bla" ],
        ) { #"#silence vim highlighting
            $c++;
            @ctl_n = ();
            @plus = ();
            my $match = (("<bla><blubb></blubb></bla>" =~ $test->[1]) ? 1 : 0);
            push @ctl_n, (defined $^N ? $^N : "undef");
            push @plus, (defined $+ ? $+ : "undef");
            ok($test->[0] == $match, "(*{ ... }) match $c");
            if ($test->[0] != $match) {
              # unset @ctl_n and @plus
              @ctl_n = @plus = ();
            }
            my $note = $test->[4] ? " - $test->[4]" : "";
            is("@ctl_n", $test->[2], "(*{ ... }) ctl_n $c$note");
            is("@plus", $test->[3], "(*{ ... }) plus $c$note");
        }
    }

    {
        # Test if $^N and $+ work in (?{})
        our @ctl_n = ();
        our @plus = ();
        our $nested_tags;
        $nested_tags = qr{
            <
               ((\w)+)
               (?{
                       push @ctl_n, (defined $^N ? $^N : "undef");
                       push @plus, (defined $+ ? $+ : "undef");
               })
            >
            (??{$nested_tags})*
            </\s* \w+ \s*>
        }x;


        my $c = 0;
        for my $test (
            # Test structure:
            #  [ Expected result, Regex, Expected value(s) of $^N, Expected value(s) of $+ ]
            [ 1, qr#^$nested_tags$#, "bla blubb bla", "a b a" ],
            [ 1, qr#^($nested_tags)$#, "bla blubb <bla><blubb></blubb></bla>", "a b a" ],
            [ 1, qr#^(|)$nested_tags$#, "bla blubb bla", "a b a" ],
            [ 1, qr#^(?:|)$nested_tags$#, "bla blubb bla", "a b a" ],
            [ 1, qr#^<(bl|bla)>$nested_tags<(/\1)>$#, "blubb /bla", "b /bla" ],
            [ 1, qr#(??{"(|)"})$nested_tags$#, "bla blubb bla", "a b a" ],
            [ 1, qr#^(??{"(bla|)"})$nested_tags$#, "bla blubb bla", "a b a" ],
            [ 1, qr#^(??{"(|)"})(??{$nested_tags})$#, "bla blubb undef", "a b undef" ],
            [ 1, qr#^(??{"(?:|)"})$nested_tags$#, "bla blubb bla", "a b a" ],
            [ 1, qr#^((??{"(?:bla|)"}))((??{$nested_tags}))$#, "bla blubb <bla><blubb></blubb></bla>", "a b <bla><blubb></blubb></bla>" ],
            [ 1, qr#^((??{"(?!)?"}))((??{$nested_tags}))$#, "bla blubb <bla><blubb></blubb></bla>", "a b <bla><blubb></blubb></bla>" ],
            [ 1, qr#^((??{"(?:|<(/?bla)>)"}))((??{$nested_tags}))\1$#, "bla blubb <bla><blubb></blubb></bla>", "a b <bla><blubb></blubb></bla>" ],
            [ 0, qr#^((??{"(?!)"}))?((??{$nested_tags}))(?!)$#, # changed in perl 5.37.7
                 "bla blubb blub blu bl b bl b undef",
                 "a b b u l b l b undef" ],

        ) { #"#silence vim highlighting
            $c++;
            @ctl_n = ();
            @plus = ();
            my $match = (("<bla><blubb></blubb></bla>" =~ $test->[1]) ? 1 : 0);
            push @ctl_n, (defined $^N ? $^N : "undef");
            push @plus, (defined $+ ? $+ : "undef");
            ok($test->[0] == $match, "match $c");
            if ($test->[0] != $match) {
              # unset @ctl_n and @plus
              @ctl_n = @plus = ();
            }
            is("@ctl_n", $test->[2], "ctl_n $c");
            is("@plus", $test->[3], "plus $c");
        }
    }

    {
        our $f;
        local $f;
        $f = sub {
            defined $_[0] ? $_[0] : "undef";
        };

        like("123", qr/^(\d)(((??{1 + $^N})))+$/, 'Bug 56194');

        our @ctl_n;
        our @plus;

        my $re  = qr#(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1}))*(?{$^N})#;
        my $re2 = qr#(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1}))*(?{$^N})(|a(b)c|def)(??{"$^R"})#;
        my $re3 = qr#(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1})){2}(?{$^N})(|a(b)c|def)(??{"$^R"})#;
        our $re5;
        local $re5 = qr#(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1})){2}(?{$^N})#;
        my $re6 = qr#(??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1})#;
        my $re7 = qr#(??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1})#;
        my $re8 = qr/(\d+)/;
        my $c = 0;
        for my $test (
             # Test structure:
             #  [
             #    String to match
             #    Regex too match
             #    Expected values of $^N
             #    Expected values of $+
             #    Expected values of $1, $2, $3, $4 and $5
             #  ]
             [
                  "1233",
                  qr#^(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1}))+(??{$^N})$#,
                  "1 2 3 3",
                  "1 2 3 3",
                  "\$1 = 1, \$2 = 3, \$3 = undef, \$4 = undef, \$5 = undef",
             ],
             [
                  "1233",
                  qr#^(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1}))+(abc|def|)?(??{$+})$#,
                  "1 2 3 3",
                  "1 2 3 3",
                  "\$1 = 1, \$2 = 3, \$3 = undef, \$4 = undef, \$5 = undef",
             ],
             [
                  "1233",
                  qr#^(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1}))+(|abc|def)?(??{$+})$#,
                  "1 2 3 3",
                  "1 2 3 3",
                  "\$1 = 1, \$2 = 3, \$3 = undef, \$4 = undef, \$5 = undef",
             ],
             [
                  "1233",
                  qr#^(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1}))+(abc|def|)?(??{$^N})$#,
                  "1 2 3 3",
                  "1 2 3 3",
                  "\$1 = 1, \$2 = 3, \$3 = undef, \$4 = undef, \$5 = undef",
             ],
             [
                  "1233",
                  qr#^(1)((??{ push @ctl_n, $f->($^N); push @plus, $f->($+); $^N + 1}))+(|abc|def)?(??{$^N})$#,
                  "1 2 3 3",
                  "1 2 3 3",
                  "\$1 = 1, \$2 = 3, \$3 = undef, \$4 = undef, \$5 = undef",
              ],
              [
                  "123abc3",
                   qr#^($re)(|a(b)c|def)(??{$^R})$#,
                   "1 2 3 abc",
                   "1 2 3 b",
                   "\$1 = 123, \$2 = 1, \$3 = 3, \$4 = abc, \$5 = b",
              ],
              [
                  "123abc3",
                   qr#^($re2)$#,
                   "1 2 3 123abc3",
                   "1 2 3 b",
                   "\$1 = 123abc3, \$2 = 1, \$3 = 3, \$4 = abc, \$5 = b",
              ],
              [
                  "123abc3",
                   qr#^($re3)$#,
                   "1 2 123abc3",
                   "1 2 b",
                   "\$1 = 123abc3, \$2 = 1, \$3 = 3, \$4 = abc, \$5 = b",
              ],
              [
                  "123abc3",
                   qr#^(??{$re5})(|abc|def)(??{"$^R"})$#,
                   "1 2 abc",
                   "1 2 abc",
                   "\$1 = abc, \$2 = undef, \$3 = undef, \$4 = undef, \$5 = undef",
              ],
              [
                  "123abc3",
                   qr#^(??{$re5})(|a(b)c|def)(??{"$^R"})$#,
                   "1 2 abc",
                   "1 2 b",
                   "\$1 = abc, \$2 = b, \$3 = undef, \$4 = undef, \$5 = undef",
              ],
              [
                  "1234",
                   qr#^((\d+)((??{push @ctl_n, $f->($^N); push @plus, $f->($+);$^N + 1}))((??{push @ctl_n, $f->($^N); push @plus, $f->($+);$^N + 1}))((??{push @ctl_n, $f->($^N); push @plus, $f->($+);$^N + 1})))$#,
                   "1234 123 12 1 2 3 1234",
                   "1234 123 12 1 2 3 4",
                   "\$1 = 1234, \$2 = 1, \$3 = 2, \$4 = 3, \$5 = 4",
              ],
              [
                   "1234556",
                   qr#^(\d+)($re6)($re6)($re6)$re6(($re6)$re6)$#,
                   "1234556 123455 12345 1234 123 12 1 2 3 4 4 5 56",
                   "1234556 123455 12345 1234 123 12 1 2 3 4 4 5 5",
                   "\$1 = 1, \$2 = 2, \$3 = 3, \$4 = 4, \$5 = 56",
              ],
              [
                  "12345562",
                   qr#^((??{$re8}))($re7)($re7)($re7)$re7($re7)($re7(\2))$#,
                   "12345562 1234556 123455 12345 1234 123 12 1 2 3 4 4 5 62",
                   "12345562 1234556 123455 12345 1234 123 12 1 2 3 4 4 5 2",
                   "\$1 = 1, \$2 = 2, \$3 = 3, \$4 = 4, \$5 = 5",
              ],
        ) {
            $c++;
            @ctl_n = ();
            @plus = ();
            undef $^R;
            my $match = $test->[0] =~ $test->[1];
            my $str = join(", ", '$1 = '.$f->($1), '$2 = '.$f->($2), '$3 = '.$f->($3), '$4 = '.$f->($4),'$5 = '.$f->($5));
            push @ctl_n, $f->($^N);
            push @plus, $f->($+);
            ok($match, "match $c; Bug 56194");
            if (not $match) {
                # unset $str, @ctl_n and @plus
                $str = "";
                @ctl_n = @plus = ();
            }
            is("@ctl_n", $test->[2], "ctl_n $c; Bug 56194");
            is("@plus", $test->[3], "plus $c; Bug 56194");
            is($str, $test->[4], "str $c; Bug 56194");
        }

        {
            @ctl_n = ();
            @plus = ();

            our $re4;
            local $re4 = qr#(1)((??{push @ctl_n, $f->($^N); push @plus, $f->($+);$^N + 1})){2}(?{$^N})(|abc|def)(??{"$^R"})#;
            undef $^R;
            my $match = "123abc3" =~ m/^(??{$re4})$/;
            my $str = join(", ", '$1 = '.$f->($1), '$2 = '.$f->($2), '$3 = '.$f->($3), '$4 = '.$f->($4),'$5 = '.$f->($5),'$^R = '.$f->($^R));
            push @ctl_n, $f->($^N);
            push @plus, $f->($+);
            ok($match, 'Bug 56194');
            if (not $match) {
                # unset $str
                @ctl_n = ();
                @plus = ();
                $str = "";
            }
            is("@ctl_n", "1 2 undef", 'Bug 56194');
            is("@plus", "1 2 undef", 'Bug 56194');
            is($str,
               "\$1 = undef, \$2 = undef, \$3 = undef, \$4 = undef, \$5 = undef, \$^R = 3",
               'Bug 56194 ($^R tweaked by 121070)');
       }
       {
            undef $^R;
            "abcd"=~/(?<Char>.)(?&Char)(?{ 42 })/;
            is("$^R", 42, 'Bug 121070 - use of (?&Char) should not clobber $^R');
            "abcd"=~/(?<Char>.)(?&Char)(?{ 42 })(?{ 43 })/;
            is("$^R", 43, 'related to 121070 - use of (?&Char) should not clobber $^R');
       }
    }

    {
	# re evals within \U, \Q etc shouldn't be seen by the lexer
	local our $a  = "i";
	local our $B  = "J";
	ok('(?{1})' =~ /^\Q(?{1})\E$/,   '\Q(?{1})\E');
	ok('(?{1})' =~ /^\Q(?{\E1\}\)$/, '\Q(?{\E1\}\)');
	eval {/^\U(??{"$a\Ea"})$/ }; norun('^\U(??{"$a\Ea"})$ norun');
	eval {/^\L(??{"$B\Ea"})$/ }; norun('^\L(??{"$B\Ea"})$ norun');
	use re 'eval';
	ok('Ia' =~ /^\U(??{"$a\Ea"})$/,  '^\U(??{"$a\Ea"})$');
	ok('ja' =~ /^\L(??{"$B\Ea"})$/,  '^\L(??{"$B\Ea"})$');
    }

    {
	# Comprehensive (hopefully) tests of closure behaviour:
	# i.e. when do (?{}) blocks get (re)compiled, and what instances
	# of lexical vars do they close over?

	# if the pattern string gets utf8 upgraded while concatenating,
	# make sure a literal code block is still detected (by still
	# compiling in the absence of use re 'eval')

	{
	    my $s1 = "\x{80}";
	    my $s2 = "\x{100}";
	    ok("\x{80}\x{100}" =~ /^$s1(?{1})$s2$/, "utf8 upgrade");
	}

	my ($cr1, $cr2, $cr3, $cr4);

	for my $x (qw(a b c)) {
	    my $bc = ($x ne 'a');
	    my $c80 = chr(0x80);

	    # the most basic: literal code should be in same scope
	    # as the parent

	    ok("A$x"       =~ /^A(??{$x})$/,       "[$x] literal code");
	    ok("\x{100}$x" =~ /^\x{100}(??{$x})$/, "[$x] literal code UTF8");

	    # the "don't recompile if pattern unchanged" mechanism
	    # shouldn't apply to code blocks - recompile every time
	    # to pick up new instances of variables

	    my $code1  = 'B(??{$x})';
	    my $code1u = $c80 . "\x{100}" . '(??{$x})';

	    eval {/^A$code1$/};
	    norun("[$x] unvarying runtime code AA norun");
	    eval {/^A$code1u$/};
	    norun("[$x] unvarying runtime code AU norun");
	    eval {/^$c80\x{100}$code1$/};
	    norun("[$x] unvarying runtime code UA norun");
	    eval {/^$c80\x{101}$code1u$/};
	    norun("[$x] unvarying runtime code UU norun");

	    {
		use re 'eval';
		ok("AB$x" =~ /^A$code1$/, "[$x] unvarying runtime code AA");
		ok("A$c80\x{100}$x" =~ /^A$code1u$/,
					    "[$x] unvarying runtime code AU");
		ok("$c80\x{100}B$x" =~ /^$c80\x{100}$code1$/,
					    "[$x] unvarying runtime code UA");
		ok("$c80\x{101}$c80\x{100}$x" =~ /^$c80\x{101}$code1u$/,
					    "[$x] unvarying runtime code UU");
	    }

	    # mixed literal and run-time code blocks

	    my $code2  = 'B(??{$x})';
	    my $code2u = $c80 . "\x{100}" . '(??{$x})';

	    eval {/^A(??{$x})-$code2$/};
	    norun("[$x] literal+runtime AA norun");
	    eval {/^A(??{$x})-$code2u$/};
	    norun("[$x] literal+runtime AU norun");
	    eval {/^$c80\x{100}(??{$x})-$code2$/};
	    norun("[$x] literal+runtime UA norun");
	    eval {/^$c80\x{101}(??{$x})-$code2u$/};
	    norun("[$x] literal+runtime UU norun");

	    {
		use re 'eval';
		ok("A$x-B$x" =~ /^A(??{$x})-$code2$/,
					    "[$x] literal+runtime AA");
		ok("A$x-$c80\x{100}$x" =~ /^A(??{$x})-$code2u$/,
					    "[$x] literal+runtime AU");
		ok("$c80\x{100}$x-B$x" =~ /^$c80\x{100}(??{$x})-$code2$/,
					    "[$x] literal+runtime UA");
		ok("$c80\x{101}$x-$c80\x{100}$x"
					    =~ /^$c80\x{101}(??{$x})-$code2u$/,
					    "[$x] literal+runtime UU");
	    }

	    # literal qr code only created once, naked

	    $cr1 //= qr/^A(??{$x})$/;
	    ok("Aa" =~ $cr1, "[$x] literal qr once naked");

	    # literal qr code only created once, embedded with text

	    $cr2 //= qr/B(??{$x})$/;
	    ok("ABa" =~ /^A$cr2/, "[$x] literal qr once embedded text");

	    # literal qr code only created once, embedded with text + lit code

	    $cr3 //= qr/C(??{$x})$/;
	    ok("A$x-BCa" =~ /^A(??{$x})-B$cr3/,
			    "[$x] literal qr once embedded text + lit code");

	    # literal qr code only created once, embedded with text + run code

	    $cr4 //= qr/C(??{$x})$/;
	    my $code3 = 'A(??{$x})';

	    eval {/^$code3-B$cr4/};
	    norun("[$x] literal qr once embedded text + run code norun");
	    {
		use re 'eval';
		ok("A$x-BCa" =~ /^$code3-B$cr4/,
			    "[$x] literal qr once embedded text + run code");
	    }

	    # literal qr code, naked

	    my $r1 = qr/^A(??{$x})$/;
	    ok("A$x" =~ $r1, "[$x] literal qr naked");

	    # literal qr code, embedded with text

	    my $r2 = qr/B(??{$x})$/;
	    ok("AB$x" =~ /^A$r2/, "[$x] literal qr embedded text");

	    # literal qr code, embedded with text + lit code

	    my $r3 = qr/C(??{$x})$/;
	    ok("A$x-BC$x" =~ /^A(??{$x})-B$r3/,
				"[$x] literal qr embedded text + lit code");

	    # literal qr code, embedded with text + run code

	    my $r4 = qr/C(??{$x})$/;
	    my $code4 = '(??{$x})';

	    eval {/^A$code4-B$r4/};
	    norun("[$x] literal qr embedded text + run code");
	    {
		use re 'eval';
		ok("A$x-BC$x" =~ /^A$code4-B$r4/,
				"[$x] literal qr embedded text + run code");
	    }

	    # nested qr in different scopes

	    my $code5 = '(??{$x})';
	    my $r5 = qr/C(??{$x})/;

	    my $r6;
	    eval {qr/$code5-C(??{$x})/}; norun("r6 norun");
	    {
		use re 'eval';
		$r6 = qr/$code5-C(??{$x})/;
	    }

	    my @rr5;
	    my @rr6;

	    for my $y (qw(d e f)) {

		my $rr5 = qr/^A(??{"$x$y"})-$r5/;
		push @rr5, $rr5;
		ok("A$x$y-C$x" =~ $rr5,
				"[$x-$y] literal qr + r5");

		my $rr6 = qr/^A(??{"$x$y"})-$r6/;
		push @rr6, $rr6;
		ok("A$x$y-$x-C$x" =~ $rr6,
				"[$x-$y] literal qr + r6");
	    }

	    for my $i (0,1,2) {
		my $y = 'Y';
		my $yy = (qw(d e f))[$i];
		my $rr5 = $rr5[$i];
		ok("A$x$yy-C$x" =~ $rr5, "[$x-$yy] literal qr + r5, outside");
		ok("A$x$yy-C$x-D$x" =~ /$rr5-D(??{$x})$/,
				"[$x-$yy] literal qr + r5 + lit, outside");


		my $rr6 = $rr6[$i];
		push @rr6, $rr6;
		ok("A$x$yy-$x-C$x" =~ $rr6,
				"[$x-$yy] literal qr + r6, outside");
		ok("A$x$yy-$x-C$x-D$x" =~ /$rr6-D(??{$x})/,
				"[$x-$yy] literal qr + r6 +lit, outside");
	    }
	}

	# recursive subs should get lexical from the correct pad depth

	sub recurse {
	    my ($n) = @_;
	    return if $n > 2;
	    ok("A$n" =~ /^A(??{$n})$/, "recurse($n)");
	    recurse($n+1);
	}
	recurse(0);

	# for qr// containing run-time elements but with a compile-time
	# code block, make sure the run-time bits are executed in the same
	# pad they were compiled in
	{
	    my $a = 'a'; # ensure outer and inner pads don't align
	    my $b = 'b';
	    my $c = 'c';
	    my $d = 'd';
	    my $r = qr/^$b(??{$c})$d$/;
	    ok("bcd" =~ $r, "qr with run-time elements and code block");
	}

	# check that cascaded embedded regexes all see their own lexical
	# environment

	{
	    my ($r1, $r2, $r3, $r4);
	    my ($x1, $x2, $x3, $x4) = (5,6,7,8);
	    { my $x1 = 1; $r1 = qr/A(??{$x1})/; }
	    { my $x2 = 2; $r2 = qr/$r1(??{$x2})/; }
	    { my $x3 = 3; $r3 = qr/$r2(??{$x3})/; }
	    { my $x4 = 4; $r4 = qr/$r3(??{$x4})/; }
	    ok("A1234" =~ /^$r4$/, "cascaded qr");
	}

	# and again, but in a loop, with no external references
	# being maintained to the qr's

	{
	    my $r = 'A';
	    for my $x (1..4) {
		$r = qr/$r(??{$x})/;
	    }
	    my $x = 5;
	    ok("A1234" =~ /^$r$/, "cascaded qr loop");
	}


	# and again, but compiling the qrs in an eval so there
	# aren't even refs to the qrs from any ops

	{
	    my $r = 'A';
	    for my $x (1..4) {
		$r = eval q[ qr/$r(??{$x})/; ];
	    }
	    my $x = 5;
	    ok("A1234" =~ /^$r$/, "cascaded qr loop");
	}

	# have qrs with either literal code blocks or only embedded
	# code blocks, but not both

	{
	    my ($r1, $r2, $r3, $r4);
	    my ($x1, $x3) = (7,8);
	    { my $x1 = 1; $r1 = qr/A(??{$x1})/; }
	    {             $r2 = qr/${r1}2/; }
	    { my $x3 = 3; $r3 = qr/$r2(??{$x3})/; }
	    {             $r4 = qr/${r3}4/; }
	    ok("A1234"  =~   /^$r4$/,    "cascaded qr mix 1");
	    ok("A12345" =~   /^${r4}5$/, "cascaded qr mix 2");
	    ok("A1234"  =~ qr/^$r4$/   , "cascaded qr mix 3");
	    ok("A12345" =~ qr/^${r4}5$/, "cascaded qr mix 4");
	}

	# and make sure things are freed at the right time
	{
	    sub Foo99::DESTROY { $Foo99::d++ }
	    $Foo99::d = 0;
	    my $r1;
	    {
	        my $x = bless [1], 'Foo99';
	        $r1 = eval 'qr/(??{$x->[0]})/';
	    }
	    my $r2 = eval 'qr/a$r1/';
	    my $x = 2;
	    ok(eval '"a1" =~ qr/^$r2$/', "match while in scope");
	    # make sure PL_reg_curpm isn't holding on to anything
	    "a" =~ /a(?{1})/;
	    is($Foo99::d, 0, "before scope exit");
	}
	::is($Foo99::d, 1, "after scope exit");

	# forward declared subs should Do The Right Thing with any anon CVs
	# within them (i.e. pad_fixup_inner_anons() should work)

	sub forward;
	sub forward {
	    my $x = "a";
	    my $A = "A";
	    ok("Aa" =~ qr/^A(??{$x})$/,  "forward qr compiletime");
	    ok("Aa" =~ qr/^$A(??{$x})$/, "forward qr runtime");
	}
	forward;
    }

    # test that run-time embedded code, when re-fed into toker,
    # does all the right escapes

    {
	my $enc;
        $enc = eval 'use Encode; find_encoding("ascii")' unless $::IS_EBCDIC;

	my $x = 0;
	my $y = 'bad';

	# note that most of the strings below are single-quoted, and the
	# things within them, like '$y', *aren't* intended to interpolate

	my $s1 =
	    'a\\$y(?# (??{BEGIN{$x=1} "X1"})b(?# \Ux2\E)c\'d\\\\e\\\\Uf\\\\E';

	ok(q{a$ybc'd\e\Uf\E} =~ /^$s1$/, "reparse");
	is($x, 0, "reparse no BEGIN");

	my $s2 = 'g\\$y# (??{{BEGIN{$x=2} "X3"}) \Ux3\E'  . "\nh";

	ok(q{a$ybc'd\\e\\Uf\\Eg$yh} =~ /^$s1$s2$/x, "reparse /x");
	is($x, 0, "reparse /x no BEGIN");

	my $b = '\\';
	my $q = '\'';

	#  non-ascii in string as "<0xNNN>"
	sub esc_str {
	    my $s = shift;
	    $s =~ s{(.)}{
			my $c = ord($1);
			(utf8::native_to_unicode($c)< 32
                         || utf8::native_to_unicode($c) > 127)
                        ? sprintf("<0x%x>", $c) : $1;
		}ge;
	    $s;
	}
	sub  fmt { sprintf "hairy backslashes %s [%s] =~ /^%s/",
			$_[0], esc_str($_[1]), esc_str($_[2]);
	}


	for my $u (
	    [ '',  '', 'blank ' ],
	    [ "\x{100}", '\x{100}', 'single' ],
	    [ "\x{100}", "\x{100}", 'double' ])
	{
	    for my $pair (
		    [ "$b",        "$b$b"               ],
		    [ "$q",        "$q"                 ],
		    [ "$b$q",      "$b$b$b$q"           ],
		    [ "$b$b$q",    "$b$b$b$b$q"         ],
		    [ "$b$b$b$q",  "$b$b$b$b$b$b$q"     ],
		    [ "$b$b$b$b$q","$b$b$b$b$b$b$b$b$q" ],
	    ) {
		my ($s, $r) = @$pair;
		$s = "9$s";
		my $ss = "$u->[0]$s";

		my $c = '9' . $r;
		my $cc = "$u->[1]$c";

		ok($ss =~ /^$cc/, fmt("plain      $u->[2]", $ss, $cc));

		no strict;
		$nine = $nine = "bad";
                $ss = "$u->[0]\t${q}\x41${b}x42$s" if $::IS_ASCII;
                $ss = "$u->[0]\t${q}\xC1${b}xC2$s" if $::IS_EBCDIC;
		for my $use_qr ('', 'qr') {
		    $c =  qq[(??{my \$z='{';]
			. (($::IS_ASCII)
                           ? qq[$use_qr"$b${b}t$b$q$b${b}x41$b$b$b${b}x42"]
                           : qq[$use_qr"$b${b}t$b$q$b${b}xC1$b$b$b${b}xC2"])
			. qq[. \$nine})];
		    # (??{ qr/str/ }) goes through one less interpolation
		    # stage than  (??{ qq/str/ })
		    $c =~ s{\\\\}{\\}g if ($use_qr eq 'qr');
		    $c .= $r;
		    $cc = "$u->[1]$c";
		    my $nine = 9;

		    eval {/^$cc/}; norun(fmt("code   norun $u->[2]", $ss, $cc));
		    {
			use re 'eval';
			ok($ss =~ /^$cc/, fmt("code         $u->[2]", $ss, $cc));
		    }
		}
	    }
	}

	my $code1u = "(??{qw(\x{100})})";
	eval {/^$code1u$/}; norun("reparse embedded unicode norun");
	{
	    use re 'eval';
	    ok("\x{100}" =~ /^$code1u$/, "reparse embedded unicode");
	}
    }

    # a non-pattern literal won't get code blocks parsed at compile time;
    # but they must get parsed later on if 'use re eval' is in scope
    # also check that unbalanced {}'s are parsed ok

    {
	eval q["a{" =~ '^(??{"a{"})$'];
	norun("non-pattern literal code norun");
	eval {/^${\'(??{"a{"})'}$/};
	norun("runtime code with unbalanced {} norun");

	use re 'eval';
	ok("a{" =~ '^a(??{"{"})$', "non-pattern literal code");
	ok("a{" =~ /^a${\'(??{"{"})'}$/, "runtime code with unbalanced {}");
    }

    # make sure warnings come from the right place

    {
	use warnings;
	my ($s, $t, $w);
	local $SIG{__WARN__} = sub { $w .= "@_" };

	$w = ''; $s = 's';
	my $r = qr/(?{$t=$s+1})/;
	"a" =~ /a$r/;
	like($w, qr/pat_re_eval/, "warning main file");

	# do it in an eval to get predictable line numbers
	eval q[

	    $r = qr/(?{$t=$s+1})/;
	];
	$w = ''; $s = 's';
	"a" =~ /a$r/;
	like($w, qr/ at \(eval \d+\) line 3/, "warning eval A");

	$w = ''; $s = 's';
	eval q[
	    use re 'eval';
	    my $c = '(?{$t=$s+1})';
	    "a" =~ /a$c/;
	    1;
	];
	like($w, qr/ at \(eval \d+\) line 1/, "warning eval B");
    }

    # jumbo test for:
    # * recursion;
    # * mixing all the different types of blocks (literal, qr/literal/,
    #   runtime);
    # * backtracking (the Z+ alternation ensures CURLYX and full
    #   scope popping on backtracking)

    {
        sub recurse2 {
            my ($depth)= @_;
	    return unless $depth;
            my $s1 = '3-LMN';
            my $r1 = qr/(??{"$s1-$depth"})/;

	    my $s2 = '4-PQR';
            my $c1 = '(??{"$s2-$depth"})';
            use re 'eval';
	    ok(   "<12345-ABC-$depth-123-LMN-$depth-1234-PQR-$depth>"
	        . "<12345-ABC-$depth-123-LMN-$depth-1234-PQR-$depth>"
		=~
		  /^<(\d|Z+)+(??{"45-ABC-$depth-"})(\d|Z+)+$r1-\d+$c1>
		    <(\d|Z+)+(??{"45-ABC-$depth-"})(\d|Z+)+$r1-\d+$c1>$/x,
		"recurse2($depth)");
	    recurse2($depth-1);
	}
	recurse2(5);
    }

    # nested (??{}) called from various levels of a recursive function

    {
	sub recurse3 {
	    my ($n) = @_;
	    return if $n > 3;
	    ok("A$n" =~ m{^A(??{ "0123" =~ /((??{$n}))/; $1 })$},
		"recurse3($n)");
	    ok("A$n" !~ m{^A(??{ "0123" =~ /((??{$n}))/; "X" })$},
		"recurse3($n) nomatch");
	    recurse3($n+1);
	}
	recurse3(0);
    }

    # nested (??{}) being invoked recursively via a function

    {
	my $s = '';
	our $recurse4;
	my @alpha = qw(A B C D E);
	$recurse4 = sub {
	    my ($n) = @_;
	    $s .= "(n=$n:";
	    if ($n < 4) {
		my $m = ("$alpha[$n]" . substr("0123", 0, $n+1)) =~
		    m{^([A-Z])
		      (??{
			    $s .= "1=$1:";
			    "$n-0123" =~ m{^(\d)-(((??{$recurse4->($n+1)})))};
			    $s .= "i1=$1:<=[$2]";
			    $3; # NB - not stringified
		       })
		       $
		     }x;
		$s .= "1a=$1:";
		$s .= $m ? 'M' : '!M';
	    }
	    my $ret =  '.*?' . ($n-1);
	    $s .= "<=[$ret])";
	    return $ret;
	};
	$recurse4->(0);
	my $exp =   '(n=0:1=A:(n=1:1=B:(n=2:1=C:(n=3:1=D:(n=4:<=[.*?3])'
		  . 'i1=3:<=[0123]1a=D:M<=[.*?2])i1=2:<=[012]1a=C:M<=[.*?1])'
		  . 'i1=1:<=[01]1a=B:M<=[.*?0])i1=0:<=[0]1a=A:M<=[.*?-1])';
	is($s, $exp, 'recurse4');
    }

    # single (??{}) being invoked recursively via a function

    {
	my $s = '';
	our $recurse5;
	my @alpha = qw(A B C D E);
	$recurse5 = sub {
	    my ($n) = @_;
	    $s .= "(n=$n:";
	    if ($n < 4) {
		my $m = ("$alpha[$n]" . substr("0123", 0, $n+1)) =~
		    m{^([A-Z])
		      ((??{
			    $s .= "1=$1:";
			    $recurse5->($n+1);
		       }))
		       $
		     }x;
		$s .= "1a=$1:2=$2:";
		$s .= $m ? 'M' : '!M';
	    }
	    my $ret =  '.*?' . ($n-1);
	    $s .= "<=[$ret])";
	    return $ret;
	};
	$recurse5->(0);
	my $exp =   '(n=0:1=A:(n=1:1=B:(n=2:1=C:(n=3:1=D:(n=4:<=[.*?3])'
		  . '1a=D:2=0123:M<=[.*?2])1a=C:2=012:M<=[.*?1])'
		  . '1a=B:2=01:M<=[.*?0])1a=A:2=0:M<=[.*?-1])';
	is($s, $exp, 'recurse5');
    }


    # make sure that errors during compiling run-time code get trapped

    {
	use re 'eval';

	my $code = '(?{$x=})';
	eval { "a" =~ /^a$code/ };
	like($@, qr/syntax error at \(eval \d+\) line \d+/, 'syntax error');

	$code = '(?{BEGIN{die})';
	eval { "a" =~ /^a$code/ };
	like($@,
	    qr/BEGIN failed--compilation aborted at \(eval \d+\) line \d+/,
	    'syntax error');
        
        use utf8;
        $code = '(?{Ｆｏｏ::$bar})';
        eval { "a" =~ /^a$code/ };
        like($@, qr/Bad name after Ｆｏｏ:: at \(eval \d+\) line \d+/, 'UTF8 sytax error');
    }

    # make sure that 'use re eval' is propagated into compiling the
    # pattern returned by (??{})

    {
	use re 'eval';
	my $pat = 'B(??{1})C';
	my $A = 'A';
	# compile-time outer code-block
	ok("AB1CD" =~ /^A(??{$pat})D$/, "re eval propagated compile-time");
	# run-time outer code-block
	ok("AB1CD" =~ /^$A(??{$pat})D$/, "re eval propagated run-time");
    }

    # returning a ref to something that had set magic but wasn't
    # PERL_MAGIC_qr triggered a false positive assertion failure
    # The test is not so much concerned with it not matching,
    # as with not failing the assertion

    {
	ok("a" !~ /^(a)(??{ \$1 })/, '(??{ ref })');
    }

    # make sure the uninit warning from returning an undef var
    # sees the right var

    {
	my ($u1, $u2);
	my $warn = '';
	local $SIG{__WARN__} = sub {  $warn .= $_[0] };
	$u1 =~ /(??{$u2})/ or die;
	like($warn, qr/value \$u1 in pattern match.*\n.*value at/, 'uninit');
    }

    # test that code blocks are called in scalar context

    {
	my @a = (0);
	ok("" =~ /^(?{@a})$/, '(?{}) in scalar context');
	is($^R, 1, '(?{}) in scalar context: $^R');
	ok("1" =~ /^(??{@a})$/, '(??{}) in scalar context');
	ok("foo" =~ /^(?(?{@a})foo|bar)$/, '(?(?{})|) in scalar context');
    }

    # BEGIN in compiled blocks shouldn't mess with $1 et al

    {
	use re 'eval';
	my $code1 = '(B)(??{ BEGIN { "X" =~ /X/ } $1})(C)';
	ok("ABBCA" =~ /^(.)(??{$code1})\1$/, '(?{}) BEGIN and $1');
	my $code2 = '(B)(??{ BEGIN { "X" =~ /X/ } $1 =~ /(.)/ ? $1 : ""})(C)';
	ok("ABBCA" =~ /^(.)(??{$code2})\1$/, '(?{}) BEGIN and $1 mark 2');
    }

    # check that the optimiser is applied to code blocks: see if aelem has
    # been converted to aelemfast

    {
	my $out;
	for my $prog (
	    '/(?{$a[0]})/',
	    'q() =~ qr/(?{$a[0]})/',
	    'use re q(eval); q() =~ q{(?{$a[0]})}',
	    'use re q(eval); $c = q{(?{$a[0]})}; /$c/',
	    'use re q(eval); $c = q{(?{$a[0]})}; /(?{1;})$c/',
	) {
	    $out = runperl(switches => ["-Dt"], prog => $prog, stderr => 1);
	    like($out, qr/aelemfast|Recompile perl with -DDEBUGGING/,
		"optimise: '$prog'");
	}
    }

    #  [perl #115080]
    #  Ensure that ?pat? matches exactly once, even when the run-time
    #  pattern changes, and even when the presence of run-time (?{}) affects
    #  how and when patterns are recompiled

    {
	my $m;

	$m = '';
	for (qw(a a a)) {
	    $m .= $_ if m?$_?;
	}
	is($m, 'a', '?pat? with a,a,a');

	$m = '';
	for (qw(a b c)) {
	    $m .= $_ if m?$_?;
	}
	is($m, 'a', '?pat? with a,b,c');

	use re 'eval';

	$m = '';
	for (qw(a a a)) {
	my $e = qq[(??{"$_"})];
	    $m .= $_ if m?$e?;
	}
	is($m, 'a', '?pat? with (??{a,a,a})');

	$m = '';
	for (qw(a b c)) {
	my $e = qq[(??{"$_"})];
	    $m .= $_ if m?$e?;
	}
	is($m, 'a', '?pat? with (??{a,b,c})');
    }

    {
	# this code won't actually fail, but it used to fail valgrind,
	# so its here just to make sure valgrind doesn't fail again
	# While examining the ops of the secret anon sub wrapped around
	# the qr//, the pad of the sub was in scope, so cSVOPo_sv
	# got the const from the wrong pad. By having lots of $s's
	# (aka gvsv(*s), this forces the targs of the consts which have
	# been moved to the pad, to have high indices.

	sub {
	    local our $s = "abc";
	    my $qr = qr/^(?{1})$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s$s/;
	}->();
	pass("cSVOPo_sv");
    }

    # [perl #115004]
    # code blocks in qr objects that are interpolated in arrays need
    # handling the same as if they were interpolated from scalar vars
    # (before this code would need 'use re "eval"')

    {
	use Tie::Array;

	local @global;
	my @array;
	my @refs = (0, \@array, 2);
	my @tied;
	tie @tied, 'Tie::StdArray';
	{
	    my $bb = 'B';
	    my $dd = 'D';
	    @array = ('A', qr/(??{$bb})/, 'C', qr/(??{$dd})/, 'E');
	    @tied  = @array;
	    @global = @array;
	}
	my $bb = 'X';
	my $dd = 'Y';
	ok("A B C D E=" =~ /@array/, 'bare interpolated array match');
	ok("A B C D E=" =~ qr/@array/, 'qr bare interpolated array match');
	ok("A B C D E=" =~ /@global/, 'bare interpolated global array match');
	ok("A B C D E=" =~ qr/@global/,
				    'qr bare interpolated global array match');
	ok("A B C D E=" =~ /@{$refs[1]}/, 'bare interpolated ref array match');
	ok("A B C D E=" =~ qr/@{$refs[1]}/,
					'qr bare interpolated ref array match');
	ok("A B C D E=" =~ /@tied/,  'bare interpolated tied array match');
	ok("A B C D E=" =~ qr/@tied/,  'qr bare interpolated tied array match');
	ok("aA B C D E=" =~ /^a@array=$/, 'interpolated array match');
	ok("aA B C D E=" =~ qr/^a@array=$/, 'qr interpolated array match');
	ok("aA B C D E=" =~ /^a@global=$/, 'interpolated global array match');
	ok("aA B C D E=" =~ qr/^a@global=$/,
					'qr interpolated global array match');
	ok("aA B C D E=" =~ /^a@{$refs[1]}=$/, 'interpolated ref array match');
	ok("aA B C D E=" =~ qr/^a@{$refs[1]}=$/,
					    'qr interpolated ref array match');
	ok("aA B C D E=" =~ /^a@tied=$/,  'interpolated tied array match');
	ok("aA B C D E=" =~ qr/^a@tied=$/,  'qr interpolated tied array match');

	{
	    local $" = '-';
	    ok("aA-B-C-D-E=" =~ /^a@{array}=$/,
			'interpolated array match with local sep');
	    ok("aA-B-C-D-E=" =~ qr/^a@{array}=$/,
			'qr interpolated array match with local sep');
	    ok("aA-B-C-D-E=" =~ /^a@{global}=$/,
			'interpolated global array match with local sep');
	    ok("aA-B-C-D-E=" =~ qr/^a@{global}=$/,
			'qr interpolated global array match with local sep');
	    ok("aA-B-C-D-E=" =~ /^a@{tied}=$/,
			'interpolated tied array match with local sep');
	    ok("aA-B-C-D-E=" =~ qr/^a@{tied}=$/,
			'qr interpolated tied array match with local sep');
	}

	# but don't handle the array ourselves in the presence of \Q etc

	@array  = ('A', '(?{})');
	@global = @array;
	@tied   = @array;
	ok("aA (?{})=" =~ /^a\Q@{array}\E=$/,
				'interpolated array match with \Q');
	ok("aA (?{})=" =~ qr/^a\Q@{array}\E=$/,
				'qr interpolated array match with \Q');
	ok("aA (?{})=" =~ /^a\Q@{global}\E=$/,
				'interpolated global array match with \Q');
	ok("aA (?{})=" =~ qr/^a\Q@{global}\E=$/,
				'qr interpolated global array match with \Q');
	ok("aA (?{})=" =~ /^a\Q@{$refs[1]}\E=$/,
				'interpolated ref array match with \Q');
	ok("aA (?{})=" =~ qr/^a\Q@{$refs[1]}\E=$/,
				'qr interpolated ref array match with \Q');
	ok("aA (?{})=" =~ /^a\Q@{tied}\E=$/,
				'interpolated tied array match with \Q');
	ok("aA (?{})=" =~ qr/^a\Q@{tied}\E=$/,
				'qr interpolated tied array match with \Q');

	# and check it works with an empty array

	@array = ();
	@global = ();
	@tied = ();
	ok("a=" =~ /^a@array=$/, 'empty array match');
	ok("a=" =~ qr/^a@array=$/, 'qr empty array match');
	ok("a=" =~ /^a@global=$/, 'empty global array match');
	ok("a=" =~ qr/^a@global=$/, 'qr empty global array match');
	ok("a=" =~ /^a@tied=$/,  'empty tied array match');
	ok("a=" =~ qr/^a@tied=$/,  'qr empty tied array match');
	ok("a=" =~ /^a\Q@{array}\E=$/, 'empty array match with \Q');
	ok("a=" =~ /^a\Q@{array}\E=$/, 'empty array match with \Q');
	ok("a=" =~ qr/^a\Q@{global}\E=$/,
				    'qr empty global array match with \Q');
	ok("a=" =~ /^a\Q@{tied}\E=$/, 'empty tied array match with \Q');
	ok("a=" =~ qr/^a\Q@{tied}\E=$/, 'qr empty tied array match with \Q');

	# NB: these below are empty patterns, so they happen to use the
	# successful match from the line above

	ok("a=" =~ /@array/, 'empty array pattern');
	ok("a=" =~ qr/@array/, 'qr empty array pattern');
	ok("a=" =~ /@global/, 'empty global array pattern');
	ok("a=" =~ qr/@global/, 'qr empty global array pattern');
	ok("a=" =~ /@tied/, 'empty tied pattern');
	ok("a=" =~ qr/@tied/, 'qr empty tied pattern');
	ok("a=" =~ /\Q@array\E/, 'empty array pattern with \Q');
	ok("a=" =~ qr/\Q@array\E/, 'qr empty array pattern with \Q');
	ok("a=" =~ /\Q@global\E/, 'empty global array pattern with \Q');
	ok("a=" =~ qr/\Q@global\E/, 'qr empty global array pattern with \Q');
	ok("a=" =~ /\Q@tied\E/, 'empty tied pattern with \Q');
	ok("a=" =~ qr/\Q@tied\E/, 'qr empty tied pattern with \Q');
	ok("a=" =~ //, 'completely empty pattern');
	ok("a=" =~ qr//, 'qr completely empty pattern');
    }

    {
	{ package o; use overload '""'=>sub { "abc" } }
	my $x = bless [],"o";
	my $y = \$x;
	(my $y_addr = "$y") =~ y/()//d; # REF(0x7fcb9c02) -> REF0x7fcb9c02
	# $y_addr =~ $y should be true, as should $y_addr =~ /(??{$y})/
	"abc$y_addr" =~ /(??{$x})(??{$y})/;
	is "$&", "abc$y_addr",
	   '(??{$x}) does not leak cached qr to (??{\$x}) (match)';
	is scalar "abcabc" =~ /(??{$x})(??{$y})/, "",
	   '(??{$x}) does not leak cached qr to (??{\$x}) (no match)';
    }

    {
	sub ReEvalTieTest::TIESCALAR {bless[], "ReEvalTieTest"}
	sub ReEvalTieTest::STORE{}
	sub ReEvalTieTest::FETCH { "$1" }
	tie my $t, "ReEvalTieTest";
	$t = bless [], "o";
	"aab" =~ /(a)((??{"b" =~ m|(.)|; $t}))/;
	is "[$1 $2]", "[a b]",
	   '(??{$tied_former_overload}) sees the right $1 in FETCH';
    }

    {
	my @matchsticks;
	my $ref = bless \my $o, "o";
	my $foo = sub { push @matchsticks, scalar "abc" =~ /(??{$ref})/ };
	&$foo;
	bless \$o;
	() = "$ref"; # flush AMAGIC flag on main
	&$foo;
	is "@matchsticks", "1 ", 'qr magic is not cached on refs';
    }

    {
	my ($foo, $bar) = ("foo"x1000, "bar"x1000);
	"$foo$bar" =~ /(??{".*"})/;
	is "$&", "foo"x1000 . "bar"x1000,
	    'padtmp swiping does not affect "$a$b" =~ /(??{})/'
    }

    {
        # [perl #129140]
        # this used to cause a double-free of the code_block struct
        # when re-running the compilation after spotting utf8.
        # This test doesn't catch it, but might panic, or fail under
        # valgrind etc

        my $s = '';
        /$s(?{})\x{100}/ for '', '';
        pass "RT #129140";
    }

    # RT #130650 code blocks could get double-freed during a pattern
    # compilation croak

    {
        # this used to panic or give ASAN errors
        eval 'qr/(?{})\6/';
        like $@, qr/Reference to nonexistent group/, "RT #130650";
    }

    # RT #129881
    # on exit from a pattern with multiple code blocks from different
    # CVs, PL_comppad wasn't being restored correctly

    sub {
        # give first few pad slots known values
        my ($x1, $x2, $x3, $x4, $x5) = 101..105;
        # these vars are in a separate pad
        my $r = qr/((?{my ($y1, $y2) = 201..202; 1;})A){2}X/;
        # the first alt fails, causing a switch to this anon
        # sub's pad
        "AAA" =~ /$r|(?{my ($z1, $z2) = 301..302; 1;})A/;
        is $x1, 101, "RT #129881: x1";
        is $x2, 102, "RT #129881: x2";
        is $x3, 103, "RT #129881: x3";
    }->();


    # RT #126697
    # savestack wasn't always being unwound on EVAL failure
    {
        local our $i = 0;
        my $max = 0;

        'ABC' =~ m{
            \A
            (?:
                (?: AB | A | BC )
                (?{
                    local $i = $i + 1;
                    $max = $i if $max < $i;
                })
            )*
            \z
        }x;
        is $max, 2, "RT #126697";
    }

    # RT #132772
    #
    # Ensure that optimisation of OP_CONST into OP_MULTICONCAT doesn't
    # leave any freed ops in the execution path. This is associated
    # with rpeep() being called before optimize_optree(), which causes
    # gv/rv2sv to be prematurely optimised into gvsv, confusing
    # S_maybe_multiconcat when it tries to reorganise a concat subtree
    # into a multiconcat list

    {
        my $a = "a";
        local $b = "b"; # not lexical, so optimised to OP_GVSV
        local $_ = "abc";
        ok /^a(??{ $b."c" })$/,  "RT #132772 - compile time";
        ok /^$a(??{ $b."c" })$/, "RT #132772 - run time";
        my $qr = qr/^a(??{ $b."c" })$/;
        ok /$qr/,  "RT #132772 - compile time qr//";
        $qr = qr/(??{ $b."c" })$/;
        ok /^a$qr$/,  "RT #132772 -  compile time qr// compound";
        $qr = qr/$a(??{ $b."c" })$/;
        ok /^$qr$/,  "RT #132772 -  run time qr//";
    }

    # RT #133687
    # mixing compile-time (?(?{code})) with run-time code blocks
    # was failing, because the second pass through the parser
    # (which compiles the runtime code blocks) was failing to adequately
    # mask the compile-time code blocks to shield them from a second
    # compile: /X(?{...})Y/ was being correctly masked as /X________Y/
    # but /X(?(?{...}))Y/ was being incorrectly masked as
    # /X(?________)Y/

    {
        use re 'eval';
        my $runtime_re = '(??{ "A"; })';
        ok "ABC" =~ /^ $runtime_re (?(?{ 1; })BC)    $/x, 'RT #133687 yes';
        ok "ABC" =~ /^ $runtime_re (?(?{ 0; })xy|BC) $/x, 'RT #133687 yes|no';
    }

    # RT #134208
    # when the string being matched was an SvTEMP and the re_eval died,
    # the SV's magic was being restored after the SV was freed.
    # Give ASan something to play with.

    {
        my $a;
        no warnings 'uninitialized';
        eval { "$a $1" =~ /(?{ die })/ };
        pass("SvTEMP 1");
        eval { sub { " " }->() =~ /(?{ die })/ };
        pass("SvTEMP 2");
    }

    # GH #19680 "panic: restartop in perl_run"
    # The eval block embedded within the (?{}) - but with no more code
    # following it - causes the next op after the OP_LEAVETRY to be NULL
    # (not even an OP_LEAVE). This confused the exception-catching and
    # rethrowing code: it was incorrectly rethrowing the exception rather
    # than just stopping at that point.

    ok("test" =~ m{^ (?{eval {die "boo!"}}) test $}x, "GH #19680");

    # GH #19390 Segmentation fault with use re 'eval'
    # Similar to  GH #19680 above, but exiting the eval via a syntax error
    # rather than throwing an exception

    ok("" =~ m{^ (?{eval q{$x=}})}x, "GH #19390");

} # End of sub run_tests

1;
