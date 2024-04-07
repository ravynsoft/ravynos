#!./perl

# Checks if the parser behaves correctly in edge cases
# (including weird syntax errors)

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    require './charset_tools.pl';
    skip_all_without_unicode_tables();
}

plan (tests => 58);

use utf8;
use open qw( :utf8 :std );

is *tèst, "*main::tèst", "sanity check.";
ok $::{"tèst"}, "gets the right glob in the stash.";

my $glob_by_sub = sub { *ｍａｉｎ::method }->();

is *ｍａｉｎ::method, "*ｍａｉｎ::method", "glob stringy works";
is "" . *ｍａｉｎ::method, "*ｍａｉｎ::method", "glob stringify-through-concat works";
is $glob_by_sub, "*ｍａｉｎ::method", "glob stringy works";
is "" . $glob_by_sub, "*ｍａｉｎ::method", "";

sub gimme_glob {
    no strict 'refs';
    is *{$_[0]}, "*main::$_[0]";
    *{$_[0]};
}

is "" . gimme_glob("下郎"), "*main::下郎";
$a = *下郎;
is "" . $a, "*main::下郎";

*{gimme_glob("下郎")} = sub {};

{
    ok defined *{"下郎"}{CODE};
    ok !defined *{"\344\270\213\351\203\216"}{CODE};
}

$Lèon = 1;
is ${*Lèon{SCALAR}}, 1, "scalar define in the right glob,";
ok !${*{"L\303\250on"}{SCALAR}}, "..and nothing in the wrong one.";

my $a = "foo" . chr(190);
my $b = $a    . chr(256);
chop $b; # $b is $a with utf8 on

is $a, $b, '$a equals $b';

*$b = sub { 5 };

is eval { main->$a }, 5, q!$a can call $b's sub!;
ok !$@, "..and there's no error.";

my $c = $b;
utf8::encode($c);
ok $b ne $c, '$b unequal $c';
eval { main->$c };
ok $@, q!$c can't call $b's sub.!;

# Now define another sub under the downgraded name:
*$a = sub { 6 };
# Call it:
is eval { main->$a }, 6, "Adding a new sub to *a and calling it works,";
ok !$@, "..without errors.";
eval { main->$c };
ok $@, "but it's still unreachable through *c";

*$b = \10;
is ${*$a{SCALAR}}, 10;
is ${*$b{SCALAR}}, 10;
is ${*$c{SCALAR}}, undef;

opendir FÒÒ, ".";
closedir FÒÒ;
::ok($::{"FÒÒ"}, "Bareword generates the right glob.");
::ok(!$::{"F\303\222\303\222"});

sub участники { 1 }

ok $::{"участники"}, "non-const sub declarations generate the right glob";
is $::{"участники"}->(), 1;

sub 原 () { 1 }

is grep({ $_ eq "\x{539f}"     } keys %::), 1, "Constant subs generate the right glob.";
is grep({ $_ eq "\345\216\237" } keys %::), 0;

#These should probably go elsewhere.
eval q{ sub wròng1 (_$); wròng1(1,2) };
like( $@, qr/Malformed prototype for main::wròng1/, 'Malformed prototype croak is clean.' );

eval q{ sub ча::ики ($__); ча::ики(1,2) };
like( $@, qr/Malformed prototype for ча::ики/ );

our $問 = 10;
is $問, 10, "our works";
is $main::問, 10, "...as does getting the same variable through the fully qualified name";
is ${"main::\345\225\217"}, undef, "..and using the encoded form doesn't";

{
    use charnames qw( :full );

    eval qq! my \$\x{30cb} \N{DROMEDARY CAMEL} !;
    $@ =~ s/eval \d+/eval 11/;
    is $@, 'Unrecognized character \x{1f42a}; marked by <-- HERE after  my $ニ <-- HERE near column 8 at (eval 11) line 1.
', "'Unrecognized character' croak is UTF-8 clean";

    eval "q\0foobar\0 \x{FFFF}+1";
    $@ =~ s/eval \d+/eval 11/;
    is(
        $@,
       "Unrecognized character \\x{ffff}; marked by <-- HERE after q\0foobar\0 <-- HERE near column 11 at (eval 11) line 1.\n",
       "...and nul-clean"
    );

    {
        use re 'eval';
        my $f = qq{(?{\$ネ+ 1; \x{1F42A} })};
        eval { "a" =~ /^a$f/ };
        my $e = $@;
        $e =~ s/eval \d+/eval 11/;
        is(
            $e,
            "Unrecognized character \\x{1f42a}; marked by <-- HERE after (?{\$ネ+ 1; <-- HERE near column 13 at (eval 11) line 1.\n",
            "Messages from a re-eval are UTF-8 clean"
        );

        $f = qq{(?{q\0foobar\0 \x{FFFF}+1 })};
        eval { "a" =~ /^a$f/ };
        my $e = $@;
        $e =~ s/eval \d+/eval 11/;
        is(
            $e,
            "Unrecognized character \\x{ffff}; marked by <-- HERE after q\x{0}foobar\x{0} <-- HERE near column 16 at (eval 11) line 1.\n",
           "...and nul-clean"
        );
    }
    
    {
        eval qq{\$ネ+ 1; \x{1F42A}};
        $@ =~ s/eval \d+/eval 11/;
        is(
            $@,
            "Unrecognized character \\x{1f42a}; marked by <-- HERE after \$ネ+ 1; <-- HERE near column 8 at (eval 11) line 1.\n",
            "Unrecognized character error doesn't cut off in the middle of characters"
        )
    }

}

{
    use feature 'state';
    for ( qw( my state our ) ) {
        local $@;
        eval "$_ Ｆｏｏ $x = 1;";
        like $@, qr/No such class Ｆｏｏ/u, "'No such class' warning for $_ is UTF-8 clean";
    }
}

{
    local $@;
    eval "our \$main::\x{30cb};";
    like $@, qr!No package name allowed for variable \$main::\x{30cb} in "our"!, "'No such package name allowed for variable' is UTF-8 clean";
}

{
    use feature 'state';
    local $@;
    for ( qw( my state ) ) {
        eval "$_ \$::\x{30cb};";
        like $@, qr!"$_" variable \$::\x{30cb} can't be in a package!, qq!'"$_" variable %s can't be in a package' is UTF-8 clean!;
    }
}

{
    local $@;
    eval qq!print \x{30cb}, "comma""!;
    like $@, qr/No comma allowed after filehandle/, "No comma allowed after filehandle triggers correctly for UTF-8 filehandles.";
}

# tests for "Bad name"
eval q{ Ｆｏｏ::$bar };
like( $@, qr/Bad name after Ｆｏｏ::/, 'Bad name after Ｆｏｏ::' );
eval q{ Ｆｏｏ''bar };
like( $@, qr/Bad name after Ｆｏｏ'/, 'Bad name after Ｆｏｏ\'' );

{
    no warnings 'utf8';
    local $SIG{__WARN__} = sub { }; # The eval will also output a warning,
                                    # which we ignore
    my $malformed_to_be = ($::IS_EBCDIC)   # Overlong sequence
                           ? "\x{74}\x{41}"
                           : "\x{c0}\x{a0}";
    CORE::evalbytes "use charnames ':full'; use utf8; my \$x = \"\\N{abc$malformed_to_be}\"";
    like( $@, qr/Malformed UTF-8 character \(fatal\) at /, 'Malformed UTF-8 input to \N{}');
}

# RT# 124216: Perl_sv_clear: Assertion
# If a parsing error occurred during a forced token within an interpolated
# context, the stack unwinding failed to restore PL_lex_defer and so after
# error recovery the state restored after the forced token was processed
# was the wrong one, resulting in the lexer thinking we're still inside a
# quoted string and things getting freed multiple times.
#
# The \x{3030} char isn't a legal var name, and this triggers the error.
#
# NB: this only failed if the closing quote of the interpolated string is
# the last char of the file (i.e. no trailing \n).

{
    my $bad = "\x{3030}";
    # Write out the individual utf8 bytes making up \x{3030}. This
    # avoids 'Wide char in print' warnings from test.pl. (We may still
    # get that warning when compiling the prog itself, since the
    # error it prints to stderr contains a wide char.)
    utf8::encode($bad);

    fresh_perl_like(qq{use utf8; "\$$bad"},
        qr/
            \A
            ( \QWide character in print at - line 1.\E\n )?
            \Qsyntax error at - line 1, near \E"\$.*"\n
            \QExecution of - aborted due to compilation errors.\E\z
        /xm,

        {stderr => 1}, "RT# 124216");
}

SKIP: {

    use Config;
    if ($Config{uvsize} < 8) {
        skip("test is only valid on 64-bit ints", 4);
    }
    else {
        my $a;
        my $b;

        # This caused a memory fault [perl #128738]
        $b = byte_utf8a_to_utf8n("\xFE\x82\x80\x80\x80\x80\x80"); # 0x80000000
        eval "\$a = q ${b}abc${b}";
        is $@, "",
               "No errors in eval'ing a string with large code point delimiter";
        is $a, 'abc',
               "Got expected result in eval'ing a string with a large code point"
            . " delimiter";

        $b = byte_utf8a_to_utf8n("\xFE\x83\xBF\xBF\xBF\xBF\xBF"); # 0xFFFFFFFF
        eval "\$a = q ${b}Hello, \\\\whirled!${b}";
        is $@, "",
               "No errors in eval'ing a string with large code point delimiter";
        is $a, 'Hello, \whirled!',
               "Got expected result in eval'ing a string with a large code point"
            . " delimiter";
    }
}

fresh_perl_is(<<'EOS', <<'EXPECT', {}, 'no panic in pad_findmy_pvn (#134061)');
use utf8;
eval "sort \x{100}%";
die $@;
EOS
syntax error at (eval 1) line 1, at EOF
Execution of (eval 1) aborted due to compilation errors.
EXPECT

# New tests go here ^^^^^

# Keep this test last, as it will mess up line number reporting for any
# subsequent tests.

<<END;
${
#line 57
qq ϟϟ }
END
is __LINE__, 59, '#line directive and qq with uni delims inside heredoc';

# Put new tests above the line number tests.
