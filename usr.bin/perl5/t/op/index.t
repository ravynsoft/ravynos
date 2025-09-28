#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './charset_tools.pl';
}

use strict;
plan( tests => 415 );

run_tests() unless caller;

sub run_tests {

    my $foo = 'Now is the time for all good men to come to the aid of their country.';

    my $first = substr($foo,0,index($foo,'the'));
    is($first, "Now is ");

    my $last = substr($foo,rindex($foo,'the'),100);
    is($last, "their country.");

    $last = substr($foo,index($foo,'Now'),2);
    is($last, "No");

    $last = substr($foo,rindex($foo,'Now'),2);
    is($last, "No");

    $last = substr($foo,index($foo,'.'),100);
    is($last, ".");

    $last = substr($foo,rindex($foo,'.'),100);
    is($last, ".");

    is(index("ababa","a",-1), 0);
    is(index("ababa","a",0), 0);
    is(index("ababa","a",1), 2);
    is(index("ababa","a",2), 2);
    is(index("ababa","a",3), 4);
    is(index("ababa","a",4), 4);
    is(index("ababa","a",5), -1);

    is(rindex("ababa","a",-1), -1);
    is(rindex("ababa","a",0), 0);
    is(rindex("ababa","a",1), 0);
    is(rindex("ababa","a",2), 2);
    is(rindex("ababa","a",3), 2);
    is(rindex("ababa","a",4), 4);
    is(rindex("ababa","a",5), 4);

    # tests for empty search string
    is(index("abc", "", -1), 0);
    is(index("abc", "", 0), 0);
    is(index("abc", "", 1), 1);
    is(index("abc", "", 2), 2);
    is(index("abc", "", 3), 3);
    is(index("abc", "", 4), 3);
    is(rindex("abc", "", -1), 0);
    is(rindex("abc", "", 0), 0);
    is(rindex("abc", "", 1), 1);
    is(rindex("abc", "", 2), 2);
    is(rindex("abc", "", 3), 3);
    is(rindex("abc", "", 4), 3);

    $a = "foo \x{1234}bar";

    is(index($a, "\x{1234}"), 4);
    is(index($a, "bar",    ), 5);

    is(rindex($a, "\x{1234}"), 4);
    is(rindex($a, "foo",    ), 0);

    {
        my $needle = "\x{1230}\x{1270}";
        my @needles = split ( //, $needle );
        my $haystack = "\x{1228}\x{1228}\x{1230}\x{1270}";
        foreach ( @needles ) {
            my $a = index ( "\x{1228}\x{1228}\x{1230}\x{1270}", $_ );
            my $b = index ( $haystack, $_ );
            is($a, $b, q{[perl #22375] 'split'/'index' problem for utf8});
        }
        $needle = "\x{1270}\x{1230}"; # Transpose them.
        @needles = split ( //, $needle );
        foreach ( @needles ) {
            my $a = index ( "\x{1228}\x{1228}\x{1230}\x{1270}", $_ );
            my $b = index ( $haystack, $_ );
            is($a, $b, q{[perl #22375] 'split'/'index' problem for utf8});
        }
    }

    {
        my $search;
        my $text;
        $search = "foo " . uni_to_native("\xc9") . " bar";
        $text = "a" . uni_to_native("\xa3\xa3") . "a $search    $search quux";

        my $text_utf8 = $text;
        utf8::upgrade($text_utf8);
        my $search_utf8 = $search;
        utf8::upgrade($search_utf8);

        is (index($text, $search), 5);
        is (rindex($text, $search), 18);
        is (index($text, $search_utf8), 5);
        is (rindex($text, $search_utf8), 18);
        is (index($text_utf8, $search), 5);
        is (rindex($text_utf8, $search), 18);
        is (index($text_utf8, $search_utf8), 5);
        is (rindex($text_utf8, $search_utf8), 18);

        my $text_octets = $text_utf8;
        utf8::encode ($text_octets);
        my $search_octets = $search_utf8;
        utf8::encode ($search_octets);

        is (index($text_octets, $search_octets), 7, "index octets, octets")
            or _diag ($text_octets, $search_octets);
        is (rindex($text_octets, $search_octets), 21, "rindex octets, octets");
        is (index($text_octets, $search_utf8), -1);
        is (rindex($text_octets, $search_utf8), -1);
        is (index($text_utf8, $search_octets), -1);
        is (rindex($text_utf8, $search_octets), -1);

        is (index($text_octets, $search), -1);
        is (rindex($text_octets, $search), -1);
        is (index($text, $search_octets), -1);
        is (rindex($text, $search_octets), -1);
    }

    SKIP: {
        skip("Not a 64-bit machine", 3) if length sprintf("%x", ~0) <= 8;
        my $a = eval q{"\x{80000000}"};
        my $s = $a.'defxyz';
        is(index($s, 'def'), 1, "0x80000000 is a single character");

        my $b = eval q{"\x{fffffffd}"};
        my $t = $b.'pqrxyz';
        is(index($t, 'pqr'), 1, "0xfffffffd is a single character");

        local ${^UTF8CACHE} = -1;
        is(index($t, 'xyz'), 4, "0xfffffffd and utf8cache");
    }


    # Tests for NUL characters.
    {
        my @tests = (
            ["",            -1, -1, -1],
            ["foo",         -1, -1, -1],
            ["\0",           0, -1, -1],
            ["\0\0",         0,  0, -1],
            ["\0\0\0",       0,  0,  0],
            ["foo\0",        3, -1, -1],
            ["foo\0foo\0\0", 3,  7, -1],
        );
        foreach my $l (1 .. 3) {
            my $q = "\0" x $l;
            my $i = 0;
            foreach my $test (@tests) {
                $i ++;
                my $str = $$test [0];
                my $res = $$test [$l];

                {
                    is (index ($str, $q), $res, "Find NUL character(s)");
                }

                #
                # Bug #53746 shows a difference between variables and literals,
                # so test literals as well.
                #
                my $test_str = qq {is (index ("$str", "$q"), $res, } .
                               qq {"Find NUL character(s)")};
                   $test_str =~ s/\0/\\0/g;

                eval $test_str;
                die $@ if $@;
            }
        }
    }

    {
        # RT#75898
        is(eval { utf8::upgrade($_ = " "); index $_, " ", 72 }, -1,
           'UTF-8 cache handles offset beyond the end of the string');
        $_ = "\x{100}BC";
        is(index($_, "C", 4), -1,
           'UTF-8 cache handles offset beyond the end of the string');
    }

    # RT #89218
    use constant {PVBM => 'galumphing', PVBM2 => 'bang'};

    sub index_it {
        is(index('galumphing', PVBM), 0,
           "index isn't confused by format compilation");
    }
     
    index_it();
    is($^A, '', '$^A is empty');
    formline PVBM;
    is($^A, 'galumphing', "formline isn't confused by index compilation");
    index_it();

    $^A = '';
    # must not do index here before formline.
    is($^A, '', '$^A is empty');
    formline PVBM2;
    is($^A, 'bang', "formline isn't confused by index compilation");
    is(index('bang', PVBM2), 0, "index isn't confused by format compilation");

    {
        use constant perl => "rules";
        is(index("perl rules", perl), 5, 'first index of a constant works');
        is(index("rules 1 & 2", perl), 0, 'second index of the same constant works');
    }

    # PVBM compilation should not flatten ref constants
    use constant riffraff => \our $referent;
    index "foo", riffraff;
    is ref riffraff, 'SCALAR', 'index does not flatten ref constants';

    package o { use overload '""' => sub { "foo" } }
    bless \our $referent, o::;
    is index("foo", riffraff), 0,
        'index respects changes in ref stringification';

    use constant quire => ${qr/(?{})/}; # A REGEXP, not a reference to one
    index "foo", quire;
    eval ' "" =~ quire ';
    is $@, "", 'regexp constants containing code blocks are not flattened';

    use constant bang => $! = 8;
    index "foo", bang;
    cmp_ok bang, '==', 8, 'dualvar constants are not flattened';

    use constant u => undef;
    {
        my $w;
        local $SIG{__WARN__} = sub { $w .= shift };
        eval '
            use warnings;
            sub { () = index "foo", u; }
        ';
        is $w, undef, 'no warnings from compiling index($foo, undef_constant)';
    }
    is u, undef, 'undef constant is still undef';

    is index('the main road', __PACKAGE__), 4,
        '[perl #119169] __PACKAGE__ as 2nd argument';

    utf8::upgrade my $substr = "\x{a3}a";

    is index($substr, 'a'), 1, 'index reply reflects characters not octets';

    # op_eq, op_const optimised away in (index() == -1) and variants

    for my $test (
          # expect:
          #    F: always false regardless of the expression
          #    T: always true  regardless of the expression
          #    f: expect false if the string is found
          #    t: expect true  if the string is found
          #
          # op  const  expect
        [ '<',    -1,      'F' ],
        [ '<',     0,      'f' ],

        [ '<=',   -1,      'f' ],
        [ '<=',    0,      'f' ],

        [ '==',   -1,      'f' ],
        [ '==',    0,      'F' ],

        [ '!=',   -1,      't' ],
        [ '!=',    0,      'T' ],

        [ '>=',   -1,      'T' ],
        [ '>=',    0,      't' ],

        [ '>',    -1,      't' ],
        [ '>',     0,      't' ],
    ) {
        my ($op, $const, $expect0) = @$test;

        my $s = "abcde";
        my $r;

        for my $substr ("e", "z") {
            my $expect =
                $expect0 eq 'T' ? 1 == 1 :
                $expect0 eq 'F' ? 0 == 1 :
                $expect0 eq 't' ? ($substr eq "e") :
                                  ($substr ne "e");

            for my $rindex ("", "r") {
                for my $reverse (0, 1) {
                    my $rop = $op;
                    if ($reverse) {
                        $rop =~ s/>/</ or  $rop =~ s/</>/;
                    }
                    for my $targmy (0, 1) {
                        my $index = "${rindex}index(\$s, '$substr')";
                        my $expr = $reverse ? "$const $rop $index" : "$index $rop $const";
                        # OPpTARGET_MY variant: the '$r = ' is optimised away too
                        $expr = "\$r = ($expr)" if $targmy;

                        my $got = eval $expr;
                        die "eval of <$expr> gave: $@\n" if $@ ne "";

                        is !!$got, $expect, $expr;
                        if ($targmy) {
                            is !!$r, $expect, "$expr - r value";
                        }
                    }
                }
            }
        }
    }

    {
        # RT #131823
        # index with OPpTARGET_MY shouldn't do the '== -1' optimisation
        my $s = "abxyz";
        my $r;

        ok(!(($r = index($s,"z")) == -1),  "(r = index(a)) == -1");
        is($r, 4,                          "(r = index(a)) == -1 - r value");


    }

    {
        my $store = 100;
        package MyTie {
            require Tie::Scalar;
            our @ISA = qw(Tie::StdScalar);
            sub STORE {
                my ($self, $value) = @_;

                $store = $value;
            }
        };
        my $x;
        tie $x, "MyTie";
        $x = (index("foo", "o") == -1);
        ok(!$store, 'magic called on $lexical = (index(...) == -1)');
    }
    {
        is(eval <<'EOS', "a", 'optimized $lex = (index(...) == -1) is an lvalue');
my $y = "foo";
my $z = "o";
my $x;
($x = (index($y, $z) == -1)) =~ s/^/a/;
$x;
EOS
    }

    {
        my $s = "abc";
        my $len = length($s);
        utf8::upgrade($s);
        length($s);
        is(index($s, "", $len+1), 3, 'Overlong index doesn\'t confuse utf8 cache');
    }

} # end of sub run_tests
