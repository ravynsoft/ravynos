#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(  qw(../lib .) );
    require Config; import Config;
    require "./loc_tools.pl";
}

plan tests => 60;

if ($Config{ebcdic} eq 'define') {
    $_ = join "", map chr($_), 129..233;

    # 105 characters - 52 letters = 53 backslashes
    # 105 characters + 53 backslashes = 158 characters
    $_ = quotemeta $_;
    is(length($_), 158, "quotemeta string");
    # 104 non-backslash characters
    is(tr/\\//cd, 104, "tr count non-backslashed");
} else { # some ASCII descendant, then.
    $_ = join "", map chr(utf8::unicode_to_native($_)), 32..127;

    # 96 characters - 52 letters - 10 digits - 1 underscore = 33 backslashes
    # 96 characters + 33 backslashes = 129 characters
    $_ = quotemeta $_;
    is(length($_), 129, "quotemeta string");
    # 95 non-backslash characters
    is(tr/\\//cd, 95, "tr count non-backslashed");
}

is(length(quotemeta ""), 0, "quotemeta empty string");

is("aA\UbB\LcC\EdD", "aABBccdD", 'aA\UbB\LcC\EdD');
is("aA\LbB\UcC\EdD", "aAbbCCdD", 'aA\LbB\UcC\EdD');
is("\L\upERL", "Perl", '\L\upERL');
is("\u\LpERL", "Perl", '\u\LpERL');
is("\U\lPerl", "pERL", '\U\lPerl');
is("\l\UPerl", "pERL", '\l\UPerl');
is("\u\LpE\Q#X#\ER\EL", "Pe\\#x\\#rL", '\u\LpE\Q#X#\ER\EL');
is("\l\UPe\Q!x!\Er\El", "pE\\!X\\!Rl", '\l\UPe\Q!x!\Er\El');
is("\Q\u\LpE.X.R\EL\E.", "Pe\\.x\\.rL.", '\Q\u\LpE.X.R\EL\E.');
is("\Q\l\UPe*x*r\El\E*", "pE\\*X\\*Rl*", '\Q\l\UPe*x*r\El\E*');
is("\U\lPerl\E\E\E\E", "pERL", '\U\lPerl\E\E\E\E');
is("\l\UPerl\E\E\E\E", "pERL", '\l\UPerl\E\E\E\E');

is(quotemeta("\x{263a}"), "\\\x{263a}", "quotemeta Unicode quoted");
is(length(quotemeta("\x{263a}")), 2, "quotemeta Unicode quoted length");
is(quotemeta("\x{100}"), "\x{100}", "quotemeta Unicode nonquoted");
is(length(quotemeta("\x{100}")), 1, "quotemeta Unicode nonquoted length");

my $char = ":";
utf8::upgrade($char);
is(quotemeta($char), "\\$char", "quotemeta '$char' in UTF-8");
is(length(quotemeta($char)), 2, "quotemeta '$char'  in UTF-8 length");

$char = "M";
utf8::upgrade($char);
is(quotemeta($char), "$char", "quotemeta '$char' in UTF-8");
is(length(quotemeta($char)), 1, "quotemeta '$char'  in UTF-8 length");

my $char = "\N{U+D7}";
utf8::upgrade($char);
is(quotemeta($char), "\\$char", "quotemeta '\\N{U+D7}' in UTF-8");
is(length(quotemeta($char)), 2, "quotemeta '\\N{U+D7}'  in UTF-8 length");

$char = "\N{U+DF}";
utf8::upgrade($char);
is(quotemeta($char), "$char", "quotemeta '\\N{U+DF}' in UTF-8");
is(length(quotemeta($char)), 1, "quotemeta '\\N{U+DF}'  in UTF-8 length");

{
    no feature 'unicode_strings';

    # BF is chosen because it is NOt alphanumeric in both Latin1 and EBCDIC
    # DF is chosen because it IS alphanumeric in both Latin1 and EBCDIC
    is(quotemeta("\x{bf}"), "\\\x{bf}", "quotemeta Latin1 no unicode_strings quoted");
    is(length(quotemeta("\x{bf}")), 2, "quotemeta Latin1 no unicode_strings quoted length");
    is(quotemeta("\x{df}"), "\\\x{df}", "quotemeta Latin1 no unicode_strings quoted");
    is(length(quotemeta("\x{df}")), 2, "quotemeta Latin1 no unicode_strings quoted length");

  SKIP: {
    skip 'Locales not available', 8 unless locales_enabled('LC_CTYPE');
    use locale;

    my $char = ":";
    is(quotemeta($char), "\\$char", "quotemeta '$char' locale");
    is(length(quotemeta($char)), 2, "quotemeta '$char' locale");

    $char = "M";
    utf8::upgrade($char);
    is(quotemeta($char), "$char", "quotemeta '$char' locale");
    is(length(quotemeta($char)), 1, "quotemeta '$char' locale");

    my $char = "\x{BF}";
    is(quotemeta($char), "\\$char", "quotemeta '\\x{BF}' locale");
    is(length(quotemeta($char)), 2, "quotemeta '\\x{BF}' locale length");

    $char = "\x{DF}";  # Every non-ASCII Latin1 is quoted in locale.
    is(quotemeta($char), "\\$char", "quotemeta '\\x{DF}' locale");
    is(length(quotemeta($char)), 2, "quotemeta '\\x{DF}' locale length");
    }
}
{
    use feature 'unicode_strings';
    is(quotemeta("\x{bf}"), "\\\x{bf}", "quotemeta Latin1 unicode_strings quoted");
    is(length(quotemeta("\x{bf}")), 2, "quotemeta Latin1 unicode_strings quoted length");
    is(quotemeta("\x{df}"), "\x{df}", "quotemeta Latin1 unicode_strings nonquoted");
    is(length(quotemeta("\x{df}")), 1, "quotemeta Latin1 unicode_strings nonquoted length");

  SKIP: {
    skip 'Locales not available', 12 unless locales_enabled('LC_CTYPE');
    use locale;

    my $char = ":";
    utf8::upgrade($char);
    is(quotemeta($char), "\\$char", "quotemeta '$char' locale in UTF-8");
    is(length(quotemeta($char)), 2, "quotemeta '$char' locale  in UTF-8 length");

    $char = "M";
    utf8::upgrade($char);
    is(quotemeta($char), "$char", "quotemeta '$char' locale in UTF-8");
    is(length(quotemeta($char)), 1, "quotemeta '$char' locale in UTF-8 length");

    my $char = "\N{U+D7}";
    utf8::upgrade($char);
    is(quotemeta($char), "\\$char", "quotemeta '\\N{U+D7}' locale in UTF-8");
    is(length(quotemeta($char)), 2, "quotemeta '\\N{U+D7}' locale in UTF-8 length");

    $char = "\N{U+DF}";  # Every non-ASCII Latin1 is quoted in locale.
    utf8::upgrade($char);
    is(quotemeta($char), "\\$char", "quotemeta '\\N{U+DF}' locale in UTF-8");
    is(length(quotemeta($char)), 2, "quotemeta '\\N{U+DF}' locale in UTF-8 length");

    is(quotemeta("\x{263a}"), "\\\x{263a}", "quotemeta locale Unicode quoted");
    is(length(quotemeta("\x{263a}")), 2, "quotemeta locale Unicode quoted length");
    is(quotemeta("\x{100}"), "\x{100}", "quotemeta locale Unicode nonquoted");
    is(length(quotemeta("\x{100}")), 1, "quotemeta locale Unicode nonquoted length");
  }
}

$a = "foo|bar";
is("a\Q\Ec$a", "acfoo|bar", '\Q\E');
is("a\L\Ec$a", "acfoo|bar", '\L\E');
is("a\l\Ec$a", "acfoo|bar", '\l\E');
is("a\U\Ec$a", "acfoo|bar", '\U\E');
is("a\u\Ec$a", "acfoo|bar", '\u\E');
