#!perl

# Tests too complex for t/base/lex.t

use strict;
use warnings;

BEGIN {
    chdir "t" if -d "t";
    require './test.pl';
    @INC= "../lib";
}

plan(tests => 53);

{
    print <<'';   # Yow!
ok 1

    # previous line intentionally left blank.

    my $yow = "ok 2";
    print <<"";   # Yow!
$yow

    # previous line intentionally left blank.
}

curr_test(3);


{
    my %foo = (aap => "monkey");
    my $foo = '';
    is("@{[$foo{'aap'}]}", 'monkey', 'interpolation of hash lookup with space between lexical variable and subscript');
    is("@{[$foo {'aap'}]}", 'monkey', 'interpolation of hash lookup with space between lexical variable and subscript - test for [perl #70091]');

# Original bug report [perl #70091]
#  #!perl
#  use warnings;
#  my %foo;
#  my $foo = '';
#  (my $tmp = $foo) =~ s/^/$foo {$0}/e;
#  __END__
#
#  This program causes a segfault with 5.10.0 and 5.10.1.
#
#  The space between '$foo' and '{' is essential, which is why piping
#  it through perl -MO=Deparse "fixes" it.
#

}

{
 delete local $ENV{PERL_UNICODE};
 fresh_perl_is(
  'BEGIN{ ++$_ for @INC{"charnames.pm","_charnames.pm"} } "\N{a}"',
  'Constant(\N{a}) unknown at - line 1, within string' . "\n"
 ."Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  'correct output (and no crash) when charnames cannot load for \N{...}'
 );
}
fresh_perl_is(
  'BEGIN{ ++$_ for @INC{"charnames.pm","_charnames.pm"};
          $^H{charnames} = "foo" } "\N{a}"',
  "Undefined subroutine &main::foo called at - line 2.\n"
 ."Propagated at - line 2, within string\n"
 ."Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  'no crash when charnames cannot load and %^H holds string'
);
fresh_perl_is(
  'BEGIN{ ++$_ for @INC{"charnames.pm","_charnames.pm"};
          $^H{charnames} = \"foo" } "\N{a}"',
  "Not a CODE reference at - line 2.\n"
 ."Propagated at - line 2, within string\n"
 ."Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  'no crash when charnames cannot load and %^H holds string reference'
);

# not fresh_perl_is, as it seems to hide the error
is runperl(
    nolib => 1, # -Ilib may also hide the error
    progs => [
      '*{',
      '         XS::APItest::gv_fetchmeth_type()',
      '}'
    ],
    stderr => 1,
   ),
  "Undefined subroutine &XS::APItest::gv_fetchmeth_type called at -e line "
 ."2.\n",
  'no buffer corruption with multiline *{...expr...}'
;

$_ = "rhubarb";
is ${no strict; \$_}, "rhubarb", '${no strict; ...}';
is join("", map{no strict; "rhu$_" } "barb"), 'rhubarb',
  'map{no strict;...}';

# [perl #123753]
fresh_perl_is(
  '$eq = "ok\n"; print $' . "\0eq\n",
  "ok\n",
   { stderr => 1 },
  '$ <null> ident'
);
fresh_perl_is(
  '@eq = "ok\n"; print @' . "\0eq\n",
  "ok\n",
   { stderr => 1 },
  '@ <null> ident'
);
fresh_perl_is(
  '%eq = ("o"=>"k\n"); print %' . "\0eq\n",
  "ok\n",
   { stderr => 1 },
  '% <null> ident'
);
fresh_perl_is(
  'sub eq { "ok\n" } print &' . "\0eq\n",
  "ok\n",
   { stderr => 1 },
  '& <null> ident'
);
fresh_perl_is(
  '$eq = "ok\n"; print ${*' . "\0eq{SCALAR}}\n",
  "ok\n",
   { stderr => 1 },
  '* <null> ident'
);
SKIP: {
    skip "Different output on EBCDIC (presumably)", 3 if $::IS_EBCDIC;
    fresh_perl_is(
      qq'"ab}"ax;&\0z\x8Ao}\x82x;', <<gibberish,
Bareword found where operator expected (Missing operator before "ax"?) at - line 1, near ""ab}"ax"
syntax error at - line 1, near ""ab}"ax"
Execution of - aborted due to compilation errors.
gibberish
       { stderr => 1 },
      'gibberish containing &\0z - used to crash [perl #123753]'
    );
    fresh_perl_is(
      qq'"ab}"ax;&{+z}\x8Ao}\x82x;', <<gibberish,
Bareword found where operator expected (Missing operator before "ax"?) at - line 1, near ""ab}"ax"
syntax error at - line 1, near ""ab}"ax"
Execution of - aborted due to compilation errors.
gibberish
       { stderr => 1 },
      'gibberish containing &{+z} - used to crash [perl #123753]'
    );
    fresh_perl_is(
      "\@{\327\n", <<\gibberisi,
Unrecognized character \xD7; marked by <-- HERE after @{<-- HERE near column 3 at - line 1.
gibberisi
       { stderr => 1 },
      '@ { \327 \n - used to garble output (or fail asan) [perl #128951]'
    );
}

fresh_perl_is(
  '/$a[/<<a',
  "Missing right curly or square bracket at - line 1, within pattern\n" .
  "syntax error at - line 1, at EOF\n" .
  "Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  '/$a[/<<a with no newline [perl #123712]'
);
fresh_perl_is(
  '/$a[m||/<<a',
  "Missing right curly or square bracket at - line 1, within pattern\n" .
  "syntax error at - line 1, at EOF\n" .
  "Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  '/$a[m||/<<a with no newline [perl #123712]'
);

fresh_perl_is(
  '"@{"',
  "Missing right curly or square bracket at - line 1, within string\n" .
  "syntax error at - line 1, at EOF\n" .
  "Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  '"@{" [perl #123712]'
);

fresh_perl_is(
  '/$0{}/',
  'syntax error at - line 1, near "{}"' . "\n" .
  "Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  '/$0{}/ with no newline [perl #123802]'
);
fresh_perl_is(
  '"\L\L"',
  'syntax error at - line 1, near "\L\L"' . "\n" .
  "Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  '"\L\L" with no newline [perl #123802]'
);
fresh_perl_is(
  '<\L\L>',
  'syntax error at - line 1, near "\L\L"' . "\n" .
  "Execution of - aborted due to compilation errors.\n",
   { stderr => 1 },
  '<\L\L> with no newline [perl #123802]'
);

is eval "qq'@\x{ff13}'", "\@\x{ff13}",
  '"@<fullwidth digit>" [perl #123963]';

fresh_perl_is(
  "s;\@{<<a;\n",
  "Can't find string terminator \"a\" anywhere before EOF at - line 1.\n",
   { stderr => 1 },
  's;@{<<a; [perl #123995]'
);

fresh_perl_is(
  '$_ = q-strict.pm-; 1 ? require : die;'
 .' print qq-ok\n- if $INC{q-strict.pm-}',
  "ok\n",
  {},
  'foo ? require : bar [perl #128307]'
);

like runperl(prog => 'sub ub(){0} ub ub', stderr=>1), qr/Bareword found/,
 '[perl #126482] Assert failure when mentioning a constant twice in a row';

fresh_perl_is(
    "do\0"."000000",
    "",
    {},
    '[perl #129069] - no output and valgrind clean'
);

fresh_perl_is(
    "00my sub\0",
    "Missing name in \"my sub\" at - line 1.\n",
    {},
    '[perl #129069] - "Missing name" warning and valgrind clean'
);

fresh_perl_like(
    "#!perl -i u\nprint 'OK'",
    qr/OK/,
    {},
    '[perl #129336] - #!perl -i argument handling'
);
SKIP:
{
    ord("A") == 65
      or skip "These tests won't work on EBCIDIC", 3;
    fresh_perl_is(
        "BEGIN{\$^H=hex ~0}\xF3",
        "Integer overflow in hexadecimal number at - line 1.\n"
      . "Malformed UTF-8 character: \\xf3 (too short; 1 byte available, need 4) at - line 1.\n"
      . "Malformed UTF-8 character (fatal) at - line 1.",
        {},
        '[perl #128996] - use of PL_op after op is freed'
    );
    fresh_perl_like(
        qq(BEGIN{\$0="";\$^H=-hex join""=>1}""\xFF),
        qr/Malformed UTF-8 character: \\xff \(too short; 1 byte available, need 13\) at - line 1\./,
        {},
        '[perl #128997] - buffer read overflow'
    );
    fresh_perl_like(
        qq(BEGIN{\$^H=0x800000}\n   0m 0\xB5\xB500\xB5\0),
        qr/Malformed UTF-8 character: \\xb5 \(unexpected continuation byte 0xb5, with no preceding start byte\)/,
        {},
        '[perl #129000] read before buffer'
    );
}
# probably only failed under ASAN
fresh_perl_is(
    "stat\tt\$#0",
    <<'EOM',
$# is no longer supported as of Perl 5.30 at - line 1.
EOM
    {},
    "[perl #129273] heap use after free or overflow"
);

fresh_perl_like('flock  _$', qr/Not enough arguments for flock/, {stderr => 1},
                "[perl #129190] intuit_method() invalidates PL_bufptr");

# Below are tests for the single set of Latin1 range paired string delimiters
# enabled by a feature, when the string isn't UTF-8.  It is more convenient to
# do all the UTF-8 testing for this feature in t/lib/croak/toke and
# t/lib/warnings/toke.
use feature 'evalbytes';
my $lhs = "\N{U+AB}";
utf8::downgrade($lhs);
my $rhs = "\N{U+BB}";
utf8::downgrade($rhs);

my @warnings;
local $SIG{__WARN__} = sub {
    my $warning = $_[0];
    chomp $warning;
    push @warnings, ($warning =~ s/\n/\n# /sgr);
};

evalbytes <<EOS;
use feature 'extra_paired_delimiters';

my \$warns = q$lhs this string uses paired latin1 delimiters $rhs;

no warnings 'experimental::extra_paired_delimiters';

my \$nowarn = q$lhs this string uses paired latin1 delimiters $rhs;
no feature 'extra_paired_delimiters';
my \$warn2= q$lhs this string uses lhs delimiter fore/aft $lhs;
my \$warn3= q$rhs this string uses rhs delimiter fore/aft $rhs;
EOS

is($@, "", "Various tests of string delims $lhs/$rhs returned without error");
is(@warnings, 3, "And the expected number of warnings were generated");
like($warnings[0],
     qr/Use of '$lhs' is experimental as a string delimiter at/,
     'And the first warning is as expected');
like($warnings[1],
     qr/Use of '$lhs' is deprecated as a string delimiter at/,
     'And the second warning is as expected');
like($warnings[2],
     qr/Use of '$rhs' is deprecated as a string delimiter at/,
     'And the third warning is as expected');

undef @warnings;
evalbytes <<EOS;
use feature 'extra_paired_delimiters';
no warnings 'experimental::extra_paired_delimiters';
my \$warn2= q$lhs this string uses lhs delimiter fore/aft $lhs;
EOS

like($@, qr/Can't find string terminator "$rhs" anywhere before EOF/,
     "Using paired delimiter both fore/aft fails as expected");
is(@warnings, 0, "With no warnings generated");

undef @warnings;
evalbytes <<EOS;
no warnings 'deprecated';
my \$warn2= q$lhs this string uses lhs delimiter fore/aft $rhs;
EOS

like($@, qr/Can't find string terminator "$lhs" anywhere before EOF/,
     "Using extra paired delimiter outside scope fails as expected");
is(@warnings, 0, "With no warnings generated");

undef @warnings;
evalbytes <<EOS;
use feature 'extra_paired_delimiters';
no warnings 'experimental::extra_paired_delimiters';
my \$warn2= q$rhs this string reverses the delimiters $lhs;
EOS

is($@, "", "Reversing delimiters works, as expected"
   . " within scope of extra delims");
is(@warnings, 0, "With no warnings generated");

undef @warnings;
evalbytes <<EOS;
no warnings 'deprecated';
my \$warn2= q$rhs this string uses lhs delimiter fore/aft $lhs;
EOS

like($@, qr/Can't find string terminator "$rhs" anywhere before EOF/,
     "Using terminating paired delimiter fore, opening aft fails as expected"
   . " outside scope of extra delims");
is(@warnings, 0, "With no warnings generated");

undef @warnings;
evalbytes <<EOS;
no warnings 'experimental::extra_paired_delimiters';
use feature 'extra_paired_delimiters';
my \$good= q$lhs this $lhs string has $lhs $lhs nested $rhs paired $rhs $rhs delimiters $rhs;
EOS

is($@, "", "Using nested extra paired delimiters works");
is(@warnings, 0, "With no warnings generated");

undef @warnings;
evalbytes <<EOS;
no warnings 'experimental::extra_paired_delimiters';
use feature 'extra_paired_delimiters';
my \$good= q$lhs this $lhs string has $lhs too few closing $lhs nested $rhs paired rhs $rhs delimiters $rhs;
EOS

like($@, qr/Can't find string terminator "$rhs" anywhere before EOF/,
     "Using too few closing delims in nesting fails as expected");
is(@warnings, 0, "With no warnings generated");
