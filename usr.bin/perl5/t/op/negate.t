#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 48;

# Some of these will cause warnings if left on.  Here we're checking the
# functionality, not the warnings.
no warnings "numeric";

# test cases based on [perl #36675] -'-10' eq '+10'
is(- 10, -10, "Simple numeric negation to negative");
is(- -10, 10, "Simple numeric negation to positive");
is(-"10", -10, "Negation of a positive string to negative");
is(-"10.0", -10, "Negation of a positive decimal sting to negative");
is(-"10foo", -10, "Negation of a numeric-lead string returns negation of numeric");
is(-"-10", 10, 'Negation of string starting with "-" returns a positive number - integer');
"-10" =~ /(.*)/;
is(-$1, 10, 'Negation of magical string starting with "-" - integer');
is(-"-10.0", 10.0, 'Negation of string starting with "-" returns a positive number - decimal');
"-10.0" =~ /(.*)/;
is(-$1, 10.0, 'Negation of magical string starting with "-" - decimal');
is(-"-10foo", "+10foo", 'Negation of string starting with "-" returns a string starting with "+" - non-numeric');
is(-"xyz", "-xyz", 'Negation of a negative string adds "-" to the front');
is(-"-xyz", "+xyz", "Negation of a negative string to positive");
is(-"+xyz", "-xyz", "Negation of a positive string to negative");
is(-bareword, "-bareword", "Negation of bareword treated like a string");
is(- -bareword, "+bareword", "Negation of -bareword returns string +bareword");
is(-" -10", 10, "Negation of a whitespace-lead numeric string");
is(-" -10.0", 10, "Negation of a whitespace-lead decimal string");
is(-" -10foo", 10,
    "Negation of a whitespace-lead sting starting with a numeric");
is(-"-e1", "+e1", "Negation of e1");

$x = "dogs";
()=0+$x;
is -$x, '-dogs', 'cached numeric value does not sabotage string negation';

is(-"97656250000000000", -97656250000000000, '-bigint vs -"bigint"');
"9765625000000000" =~ /(\d+)/;
is -$1, -"$1", '-$1 vs -"$1" with big int';

$a = "%apples";
chop($au = "%apples\x{100}");
is(-$au, -$a, 'utf8 flag makes no difference for string negation');
is -"\x{100}", 0, '-(non-ASCII) is equivalent to -(punct)';

sub TIESCALAR { bless[] }
sub STORE { $_[0][0] = $_[1] }
sub FETCH { $_[0][0] }

tie $t, "";
$a = "97656250000000000";
() = 0+$a;
$t = $a;
is -$t, -97656250000000000, 'magic str+int dualvar';

{ # Repeat most of the tests under use integer
    use integer;
    is(- 10, -10, "Simple numeric negation to negative");
    is(- -10, 10, "Simple numeric negation to positive");
    is(-"10", -10, "Negation of a positive string to negative");
    is(-"10.0", -10, "Negation of a positive decimal sting to negative");
    is(-"10foo", -10,
        "Negation of a numeric-lead string returns negation of numeric");
    is(-"-10", 10,
        'Negation of string starting with "-" returns a positive number -'
       .' integer');
    "-10" =~ /(.*)/;
    is(-$1, 10, 'Negation of magical string starting with "-" - integer');
    is(-"-10.0", 10,
        'Negation of string starting with "-" returns a positive number - '
       .'decimal');
    "-10.0" =~ /(.*)/;
    is(-$1, 10, 'Negation of magical string starting with "-" - decimal');
    is(-"-10foo", "+10foo",
       'Negation of string starting with "-" returns a string starting '
      .'with "+" - non-numeric');
    is(-"xyz", "-xyz",
       'Negation of a negative string adds "-" to the front');
    is(-"-xyz", "+xyz", "Negation of a negative string to positive");
    is(-"+xyz", "-xyz", "Negation of a positive string to negative");
    is(-bareword, "-bareword",
        "Negation of bareword treated like a string");
    is(- -bareword, "+bareword",
        "Negation of -bareword returns string +bareword");
    is(-" -10", 10, "Negation of a whitespace-lead numeric string");
    is(-" -10.0", 10, "Negation of a whitespace-lead decimal string");
    is(-" -10foo", 10,
        "Negation of a whitespace-lead sting starting with a numeric");
    is(-"-e1", "+e1", "Negation of e1 (use integer)");

    $x = "dogs";
    ()=0+$x;
    is -$x, '-dogs',
        'cached numeric value does not sabotage string negation';

    $a = "%apples";
    chop($au = "%apples\x{100}");
    is(-$au, -$a, 'utf8 flag makes no difference for string negation');
    is -"\x{100}", 0, '-(non-ASCII) is equivalent to -(punct)';
}

# [perl #120288] use integer should not stop barewords from being quoted
{
    use strict;
    use integer;
    is eval "return -a"||$@, "-a", '-bareword under strict+integer';
}
