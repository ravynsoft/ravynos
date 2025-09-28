#!./perl -wT

use strict;
use warnings;
use Config;

# This tests plain 'use locale' and adorned 'use locale ":not_characters"'
# Because these pragmas are compile time, and I (khw) am trying to test
# without using 'eval' as much as possible, which might cloud the issue,  the
# crucial parts of the code are duplicated in a block for each pragma.

# Unfortunately, many systems have defective locale definitions.  This test
# file looks for both perl bugs and bugs in the system's locale definitions.
# It can be difficult to tease apart which is which.  For the latter, there
# are tests that are based on the POSIX standard.  A character isn't supposed
# to be both a space and graphic, for example.  Another example is if a
# character is the uppercase of another, that other should be the lowercase of
# the first.  Including tests for these allows you to test for defective
# locales, as described in perllocale.  The way this file distinguishes
# between defective locales, and perl bugs is to see what percentage of
# locales fail a given test.  If it's a lot, then it's more likely to be a
# perl bug; only a few, those particular locales are likely defective.  In
# that case the failing tests are marked TODO.  (They should be reported to
# the vendor, however; but it's not perl's problem.)  In some cases, this
# script has caused tickets to be filed against perl which turn out to be the
# platform's bug, but a higher percentage of locales are failing than the
# built-in cut-off point.  For those platforms, code has been added to
# increase the cut-off, so those platforms don't trigger failing test reports.
# Ideally, the platforms would get fixed and that code would be changed to
# only kick-in when run on versions that are earlier than the fixed one.  But,
# this rarely happens in practice.

# To make a TODO test, add the string 'TODO' to its %test_names value

my $is_ebcdic = ord("A") == 193;
my $os = lc $^O;

# Configure now lets you build a perl that silently ignores taint features
my $NoTaintSupport = exists($Config{taint_support}) && !$Config{taint_support};

no warnings 'locale';  # We test even weird locales; and do some scary things
                       # in ok locales

binmode STDOUT, ':utf8';
binmode STDERR, ':utf8';

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    unshift @INC, '.';
    require './loc_tools.pl';
    unless (locales_enabled('LC_CTYPE')) {
	print "1..0\n";
	exit;
    }
    $| = 1;
    require Config; import Config;
}

use feature 'fc';
use I18N::Langinfo qw(langinfo CODESET CRNCYSTR RADIXCHAR);

# =1 adds debugging output; =2 increases the verbosity somewhat
our $debug = $ENV{PERL_DEBUG_FULL_TEST} // 0;

# Certain tests have been shown to be problematical for a few locales.  Don't
# fail them unless at least this percentage of the tested locales fail.
# EBCDIC os390 has more locales fail than normal, because it has locales that
# move various critical characters like '['.
my $acceptable_failure_percentage = ($os =~ / ^ ( os390 ) $ /x)
                                    ? 10
                                    : 5;

# The list of test numbers of the problematic tests.
my %problematical_tests;

# If any %problematical_tests fails in one of these locales, it is
# considered a TODO.
my %known_bad_locales = (
                          irix => qr/ ^ (?: cs | hu | sk ) $/x,
                          darwin => qr/ ^ lt_LT.ISO8859 /ix,
                          os390 => qr/ ^ italian /ix,
                          netbsd => qr/\bISO8859-2\b/i,

                          # This may be the same bug as the cygwin below; it's
                          # generating malformed UTF-8 on the radix being
                          # mulit-byte
                          solaris => qr/ ^ ( ar_ | pa_ ) /x,
                        );

# cygwin isn't returning proper radix length in this locale, but supposedly to
# be fixed in later versions.
if ($os eq 'cygwin' && version->new(($Config{osvers} =~ /^(\d+(?:\.\d+)+)/)[0]) le v2.4.1) {
    $known_bad_locales{'cygwin'} = qr/ ^ ps_AF /ix;
}

use Dumpvalue;

my $dumper = Dumpvalue->new(
                            tick => qq{"},
                            quoteHighBit => 0,
                            unctrl => "quote"
                           );

sub debug {
  return unless $debug;
  my($mess) = join "", '# ', @_;
  chomp $mess;
  print STDERR $dumper->stringify($mess,1), "\n";
}

sub note {
    local $debug = 1;
    debug @_;
}

sub debug_more {
  return unless $debug > 1;
  return debug(@_);
}

sub debugf {
    printf STDERR @_ if $debug;
}

$a = 'abc %9';

my $test_num = 0;

sub ok {
    my ($result, $message) = @_;
    $message = "" unless defined $message;

    print 'not ' unless ($result);
    print "ok " . ++$test_num;
    print " $message";
    print "\n";
    return ($result) ? 1 : 0;
}

sub skip {
    return ok 1, "skipped: " . shift;
}

sub fail {
    return ok 0, shift;
}

# First we'll do a lot of taint checking for locales.
# This is the easiest to test, actually, as any locale,
# even the default locale will taint under 'use locale'.

sub is_tainted { # hello, camel two.
    no warnings 'uninitialized' ;
    my $dummy;
    local $@;
    not eval { $dummy = join("", @_), kill 0; 1 }
}

sub check_taint ($;$) {
    my $message_tail = $_[1] // "";

    # Extra blanks are so aligns with taint_not output
    $message_tail = ":     $message_tail" if $message_tail;
    if ($NoTaintSupport) {
        skip("your perl was built without taint support");
    }
    else {
        ok is_tainted($_[0]), "verify that is tainted$message_tail";
    }
}

sub check_taint_not ($;$) {
    my $message_tail = $_[1] // "";
    $message_tail = ":  $message_tail" if $message_tail;
    ok((not is_tainted($_[0])), "verify that isn't tainted$message_tail");
}

foreach my $category (qw(ALL COLLATE CTYPE MESSAGES MONETARY NUMERIC TIME)) {
    my $short_result = locales_enabled($category);
    ok ($short_result == 0 || $short_result == 1,
        "Verify locales_enabled('$category') returns 0 or 1");
    debug("locales_enabled('$category') returned '$short_result'");
    my $long_result = locales_enabled("LC_$category");
    if (! ok ($long_result == $short_result,
              "   and locales_enabled('LC_$category') returns "
            . "the same value")
    ) {
        debug("locales_enabled('LC_$category') returned $long_result");
    }
}

"\tb\t" =~ /^m?(\s)(.*)\1$/;
check_taint_not   $&, "not tainted outside 'use locale'";
;

use locale;	# engage locale and therefore locale taint.

# BE SURE TO COPY ANYTHING YOU ADD to these tests to the block below for
# ":notcharacters"

check_taint_not   $a, '$a';

check_taint       uc($a), 'uc($a)';
check_taint       "\U$a", '"\U$a"';
check_taint       ucfirst($a), 'ucfirst($a)';
check_taint       "\u$a", '"\u$a"';
check_taint       lc($a), 'lc($a)';
check_taint       fc($a), 'fc($a)';
check_taint       "\L$a", '"\L$a"';
check_taint       "\F$a", '"\F$a"';
check_taint       lcfirst($a), 'lcfirst($a)';
check_taint       "\l$a", '"\l$a"';

check_taint_not  sprintf('%e', 123.456), "sprintf('%e', 123.456)";
check_taint_not  sprintf('%f', 123.456), "sprintf('%f', 123.456)";
check_taint_not  sprintf('%g', 123.456), "sprintf('%g', 123.456)";
check_taint_not  sprintf('%d', 123.456), "sprintf('%d', 123.456)";
check_taint_not  sprintf('%x', 123.456), "sprintf('%x', 123.456)";

$_ = $a;	# untaint $_

$_ = uc($a);	# taint $_

check_taint      $_, '$_ = uc($a)';

/(\w)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(\\w)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";
check_taint_not  $`, "\t\$`";
check_taint_not  $', "\t\$'";
check_taint_not  $+, "\t\$+";
check_taint_not  $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(\W)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(\\W)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";
check_taint_not  $`, "\t\$`";
check_taint_not  $', "\t\$'";
check_taint_not  $+, "\t\$+";
check_taint_not  $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(\s)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(\\s)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";

/(\S)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(\\S)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";

"0" =~ /(\d)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(\\d)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";

/(\D)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(\\D)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";

/([[:alnum:]])/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /([[:alnum:]])/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";

/([[:^alnum:]])/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /([[:^alnum:]])/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

"a" =~ /(a)|(\w)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(a)|(\\w)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
ok($1 eq 'a', ("\t" x 5) . "\$1 is 'a'");
ok(! defined $2, ("\t" x 5) . "\$2 is undefined");
check_taint_not  $2, "\t\$2";
check_taint_not  $3, "\t\$3";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";

"\N{CYRILLIC SMALL LETTER A}" =~ /(\N{CYRILLIC CAPITAL LETTER A})/i;	# no tainting because no locale dependence
check_taint_not      $&, "\$& from /(\\N{CYRILLIC CAPITAL LETTER A})/i";
check_taint_not      $`, "\t\$`";
check_taint_not      $', "\t\$'";
check_taint_not      $+, "\t\$+";
check_taint_not      $1, "\t\$1";
ok($1 eq "\N{CYRILLIC SMALL LETTER A}", ("\t" x 4) . "\t\$1 is 'small cyrillic a'");
check_taint_not      $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /./";

"(\N{KELVIN SIGN})" =~ /(\N{KELVIN SIGN})/i;	# taints because depends on locale
check_taint      $&, "\$& from /(\\N{KELVIN SIGN})/i";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not      $2, "\t\$2";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /(.)/";

"a:" =~ /(.)\b(.)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(.)\\b(.)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint      $2, "\t\$2";
check_taint_not  $3, "\t\$3";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /./";

"aa" =~ /(.)\B(.)/;	# taint $&, $`, $', $+, $1.
check_taint      $&, "\$& from /(.)\\B(.)/";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint      $2, "\t\$2";
check_taint_not  $3, "\t\$3";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$& from /./";

"aaa" =~ /(.).(\1)/i;	# notaint because not locale dependent
check_taint_not      $&, "\$ & from /(.).(\\1)/";
check_taint_not      $`, "\t\$`";
check_taint_not      $', "\t\$'";
check_taint_not      $+, "\t\$+";
check_taint_not      $1, "\t\$1";
check_taint_not      $2, "\t\$2";
check_taint_not      $3, "\t\$3";

/(.)/;	# untaint $&, $`, $', $+, $1.
check_taint_not  $&, "\$ & from /./";

$_ = $a;	# untaint $_

check_taint_not  $_, 'untainting $_ works';

/(b)/;		# this must not taint
check_taint_not  $&, "\$ & from /(b)/";
check_taint_not  $`, "\t\$`";
check_taint_not  $', "\t\$'";
check_taint_not  $+, "\t\$+";
check_taint_not  $1, "\t\$1";
check_taint_not  $2, "\t\$2";

$_ = $a;	# untaint $_

check_taint_not  $_, 'untainting $_ works';

$b = uc($a);	# taint $b
s/(.+)/$b/;	# this must taint only the $_

check_taint      $_, '$_ (wasn\'t tainted) from s/(.+)/$b/ where $b is tainted';
check_taint_not  $&, "\t\$&";
check_taint_not  $`, "\t\$`";
check_taint_not  $', "\t\$'";
check_taint_not  $+, "\t\$+";
check_taint_not  $1, "\t\$1";
check_taint_not  $2, "\t\$2";

$_ = $a;	# untaint $_

s/(.+)/b/;	# this must not taint
check_taint_not  $_, '$_ (wasn\'t tainted) from s/(.+)/b/';
check_taint_not  $&, "\t\$&";
check_taint_not  $`, "\t\$`";
check_taint_not  $', "\t\$'";
check_taint_not  $+, "\t\$+";
check_taint_not  $1, "\t\$1";
check_taint_not  $2, "\t\$2";

$b = $a;	# untaint $b

($b = $a) =~ s/\w/$&/;
check_taint      $b, '$b from ($b = $a) =~ s/\w/$&/';	# $b should be tainted.
check_taint_not  $a, '$a from ($b = $a) =~ s/\w/$&/';	# $a should be not.

$_ = $a;	# untaint $_

s/(\w)/\l$1/;	# this must taint
check_taint      $_, '$_ (wasn\'t tainted) from s/(\w)/\l$1/,';	# this must taint
check_taint      $&, "\t\$&";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

$_ = $a;	# untaint $_

s/(\w)/\L$1/;	# this must taint
check_taint      $_, '$_ (wasn\'t tainted) from s/(\w)/\L$1/,';
check_taint      $&, "\t\$&";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

$_ = $a;	# untaint $_

s/(\w)/\u$1/;	# this must taint
check_taint      $_, '$_ (wasn\'t tainted) from s/(\w)/\u$1/';
check_taint      $&, "\t\$&";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

$_ = $a;	# untaint $_

s/(\w)/\U$1/;	# this must taint
check_taint      $_, '$_ (wasn\'t tainted) from s/(\w)/\U$1/';
check_taint      $&, "\t\$&";
check_taint      $`, "\t\$`";
check_taint      $', "\t\$'";
check_taint      $+, "\t\$+";
check_taint      $1, "\t\$1";
check_taint_not  $2, "\t\$2";

# After all this tainting $a should be cool.

check_taint_not  $a, '$a still not tainted';

"a" =~ /([a-z])/;
check_taint_not $1, '"a" =~ /([a-z])/';
"foo.bar_baz" =~ /^(.*)[._](.*?)$/;  # Bug 120675
check_taint_not $1, '"foo.bar_baz" =~ /^(.*)[._](.*?)$/';

# BE SURE TO COPY ANYTHING YOU ADD to the block below

{   # This is just the previous tests copied here with a different
    # compile-time pragma.

    use locale ':not_characters'; # engage restricted locale with different
                                  # tainting rules
    check_taint_not   $a, '$a';

    check_taint_not   uc($a), 'uc($a)';
    check_taint_not   "\U$a", '"\U$a"';
    check_taint_not   ucfirst($a), 'ucfirst($a)';
    check_taint_not   "\u$a", '"\u$a"';
    check_taint_not   lc($a), 'lc($a)';
    check_taint_not   fc($a), 'fc($a)';
    check_taint_not   "\L$a", '"\L$a"';
    check_taint_not   "\F$a", '"\F$a"';
    check_taint_not   lcfirst($a), 'lcfirst($a)';
    check_taint_not   "\l$a", '"\l$a"';

    check_taint_not  sprintf('%e', 123.456), "sprintf('%e', 123.456)";
    check_taint_not  sprintf('%f', 123.456), "sprintf('%f', 123.456)";
    check_taint_not  sprintf('%g', 123.456), "sprintf('%g', 123.456)";
    check_taint_not  sprintf('%d', 123.456), "sprintf('%d', 123.456)";
    check_taint_not  sprintf('%x', 123.456), "sprintf('%x', 123.456)";

    $_ = $a;	# untaint $_

    $_ = uc($a);

    check_taint_not  $_, '$_ = uc($a)';

    /(\w)/;
    check_taint_not  $&, "\$& from /(\\w)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(\W)/;
    check_taint_not  $&, "\$& from /(\\W)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(\s)/;
    check_taint_not  $&, "\$& from /(\\s)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";

    /(\S)/;
    check_taint_not  $&, "\$& from /(\\S)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";

    "0" =~ /(\d)/;
    check_taint_not  $&, "\$& from /(\\d)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";

    /(\D)/;
    check_taint_not  $&, "\$& from /(\\D)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";

    /([[:alnum:]])/;
    check_taint_not  $&, "\$& from /([[:alnum:]])/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";

    /([[:^alnum:]])/;
    check_taint_not  $&, "\$& from /([[:^alnum:]])/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    "a" =~ /(a)|(\w)/;
    check_taint_not  $&, "\$& from /(a)|(\\w)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    ok($1 eq 'a', ("\t" x 5) . "\$1 is 'a'");
    ok(! defined $2, ("\t" x 5) . "\$2 is undefined");
    check_taint_not  $2, "\t\$2";
    check_taint_not  $3, "\t\$3";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";

    "\N{CYRILLIC SMALL LETTER A}" =~ /(\N{CYRILLIC CAPITAL LETTER A})/i;
    check_taint_not      $&, "\$& from /(\\N{CYRILLIC CAPITAL LETTER A})/i";
    check_taint_not      $`, "\t\$`";
    check_taint_not      $', "\t\$'";
    check_taint_not      $+, "\t\$+";
    check_taint_not      $1, "\t\$1";
    ok($1 eq "\N{CYRILLIC SMALL LETTER A}", ("\t" x 4) . "\t\$1 is 'small cyrillic a'");
    check_taint_not      $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /./";

    "(\N{KELVIN SIGN})" =~ /(\N{KELVIN SIGN})/i;
    check_taint_not  $&, "\$& from /(\\N{KELVIN SIGN})/i";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not      $2, "\t\$2";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /(.)/";

    "a:" =~ /(.)\b(.)/;
    check_taint_not  $&, "\$& from /(.)\\b(.)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";
    check_taint_not  $3, "\t\$3";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /./";

    "aa" =~ /(.)\B(.)/;
    check_taint_not  $&, "\$& from /(.)\\B(.)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";
    check_taint_not  $3, "\t\$3";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$& from /./";

    "aaa" =~ /(.).(\1)/i;	# notaint because not locale dependent
    check_taint_not      $&, "\$ & from /(.).(\\1)/";
    check_taint_not      $`, "\t\$`";
    check_taint_not      $', "\t\$'";
    check_taint_not      $+, "\t\$+";
    check_taint_not      $1, "\t\$1";
    check_taint_not      $2, "\t\$2";
    check_taint_not      $3, "\t\$3";

    /(.)/;	# untaint $&, $`, $', $+, $1.
    check_taint_not  $&, "\$ & from /./";

    $_ = $a;	# untaint $_

    check_taint_not  $_, 'untainting $_ works';

    /(b)/;
    check_taint_not  $&, "\$ & from /(b)/";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    $_ = $a;	# untaint $_

    check_taint_not  $_, 'untainting $_ works';

    s/(.+)/b/;
    check_taint_not  $_, '$_ (wasn\'t tainted) from s/(.+)/b/';
    check_taint_not  $&, "\t\$&";
    check_taint_not  $`, "\t\$`";
    check_taint_not  $', "\t\$'";
    check_taint_not  $+, "\t\$+";
    check_taint_not  $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    $b = $a;	# untaint $b

    ($b = $a) =~ s/\w/$&/;
    check_taint_not     $b, '$b from ($b = $a) =~ s/\w/$&/';
    check_taint_not  $a, '$a from ($b = $a) =~ s/\w/$&/';

    $_ = $a;	# untaint $_

    s/(\w)/\l$1/;
    check_taint_not     $_, '$_ (wasn\'t tainted) from s/(\w)/\l$1/,';	# this must taint
    check_taint_not     $&, "\t\$&";
    check_taint_not     $`, "\t\$`";
    check_taint_not     $', "\t\$'";
    check_taint_not     $+, "\t\$+";
    check_taint_not     $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    $_ = $a;	# untaint $_

    s/(\w)/\L$1/;
    check_taint_not     $_, '$_ (wasn\'t tainted) from s/(\w)/\L$1/,';
    check_taint_not     $&, "\t\$&";
    check_taint_not     $`, "\t\$`";
    check_taint_not     $', "\t\$'";
    check_taint_not     $+, "\t\$+";
    check_taint_not     $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    $_ = $a;	# untaint $_

    s/(\w)/\u$1/;
    check_taint_not     $_, '$_ (wasn\'t tainted) from s/(\w)/\u$1/';
    check_taint_not     $&, "\t\$&";
    check_taint_not     $`, "\t\$`";
    check_taint_not     $', "\t\$'";
    check_taint_not     $+, "\t\$+";
    check_taint_not     $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    $_ = $a;	# untaint $_

    s/(\w)/\U$1/;
    check_taint_not     $_, '$_ (wasn\'t tainted) from s/(\w)/\U$1/';
    check_taint_not     $&, "\t\$&";
    check_taint_not     $`, "\t\$`";
    check_taint_not     $', "\t\$'";
    check_taint_not     $+, "\t\$+";
    check_taint_not     $1, "\t\$1";
    check_taint_not  $2, "\t\$2";

    # After all this tainting $a should be cool.

    check_taint_not  $a, '$a still not tainted';

    "a" =~ /([a-z])/;
    check_taint_not $1, '"a" =~ /([a-z])/';
    "foo.bar_baz" =~ /^(.*)[._](.*?)$/;  # Bug 120675
    check_taint_not $1, '"foo.bar_baz" =~ /^(.*)[._](.*?)$/';

}

# Here are in scope of 'use locale'

# I think we've seen quite enough of taint.
# Let us do some *real* locale work now,
# unless setlocale() is missing (i.e. minitest).

# The test number before our first setlocale()
my $final_without_setlocale = $test_num;

# Find locales.

debug "Scanning for locales...\n";

require POSIX; import POSIX ':locale_h';
my $categories = [ 'LC_CTYPE', 'LC_NUMERIC', 'LC_ALL' ];
my @Locale;
my @include_incompatible_locales;
if ($^O eq "aix"
    and version->new(($Config{osvers} =~ /^(\d+(\.\d+))/)[0]) < 7) {
    # https://www.ibm.com/support/pages/apar/IV22097
    skip("setlocale broken on old AIX");
}
else {
    debug "Scanning for just compatible";
    @Locale = find_locales($categories);
    debug "Scanning for even incompatible";
    @include_incompatible_locales = find_locales($categories,
                                              'even incompatible locales');
}
# The locales included in the incompatible list that aren't in the compatible
# one.
my @incompatible_locales;
if (@Locale < @include_incompatible_locales) {
    my %seen;
    @seen{@Locale} = ();

    foreach my $item (@include_incompatible_locales) {
        push @incompatible_locales, $item unless exists $seen{$item};
    }

    # For each bad locale, switch into it to find out why it's incompatible
    for my $bad_locale (@incompatible_locales) {
        my @warnings;

        use warnings 'locale';

        local $SIG{__WARN__} = sub {
            my $warning = $_[0];
            chomp $warning;
            push @warnings, ($warning =~ s/\n/\n# /sgr);
        };

        debug "Trying incompatible $bad_locale";
        my $ret = setlocale(&POSIX::LC_CTYPE, $bad_locale);

        my $message = "testing of locale '$bad_locale' is skipped";
        if (@warnings) {
            skip $message . ":\n# " . join "\n# ", @warnings;
        }
        elsif (! $ret) {
            skip("$message:\n#"
               . " setlocale(&POSIX::LC_CTYPE, '$bad_locale') failed");
        }
        else {
            fail $message . ", because it is was found to be incompatible with"
                          . " Perl, but could not discern reason";
        }
    }
}

debug "Locales =\n";
for ( @Locale ) {
    debug "$_\n";
}

unless (@Locale) {
    print "1..$test_num\n";
    exit;
}


setlocale(&POSIX::LC_ALL, "C");

my %posixes;

my %Problem;
my %Okay;
my %Known_bad_locale;   # Failed test for a locale known to be bad
my %Testing;
my @Added_alpha;   # Alphas that aren't in the C locale.
my %test_names;

sub disp_chars {
    # This returns a display string denoting the input parameter @_, each
    # entry of which is a single character in the range 0-255.  The first part
    # of the output is a string of the characters in @_ that are ASCII
    # graphics, and hence unambiguously displayable.  They are given by code
    # point order.  The second part is the remaining code points, the ordinals
    # of which are each displayed as 2-digit hex.  Blanks are inserted so as
    # to keep anything from the first part looking like a 2-digit hex number.

    no locale;
    my @chars = sort { ord $a <=> ord $b } @_;
    my $output = "";
    my $range_start;
    my $start_class;
    push @chars, chr(258);  # This sentinel simplifies the loop termination
                            # logic
    foreach my $i (0 .. @chars - 1) {
        my $char = $chars[$i];
        my $range_end;
        my $class;

        # We avoid using [:posix:] classes, as these are being tested in this
        # file.  Each equivalence class below is for things that can appear in
        # a range; those that can't be in a range have class -1.  0 for those
        # which should be output in hex; and >0 for the other ranges
        if ($char =~ /[A-Z]/) {
            $class = 2;
        }
        elsif ($char =~ /[a-z]/) {
            $class = 3;
        }
        elsif ($char =~ /[0-9]/) {
            $class = 4;
        }
        # Uncomment to get literal punctuation displayed instead of hex
        #elsif ($char =~ /[[\]!"#\$\%&\'()*+,.\/:\\;<=>?\@\^_`{|}~-]/) {
        #    $class = -1;    # Punct never appears in a range
        #}
        else {
            $class = 0;     # Output in hex
        }

        if (! defined $range_start) {
            if ($class < 0) {
                $output .= " " . $char;
            }
            else {
                $range_start = ord $char;
                $start_class = $class;
            }
        } # A range ends if not consecutive, or the class-type changes
        elsif (ord $char != ($range_end = ord($chars[$i-1])) + 1
              || $class != $start_class)
        {

            # Here, the current character is not in the range.  This means the
            # previous character must have been.  Output the range up through
            # that one.
            my $range_length = $range_end - $range_start + 1;
            if ($start_class > 0) {
                $output .= " " . chr($range_start);
                $output .= "-" . chr($range_end) if $range_length > 1;
            }
            else {
                $output .= sprintf(" %02X", $range_start);
                $output .= sprintf("-%02X", $range_end) if $range_length > 1;
            }

            # Handle the new current character, as potentially beginning a new
            # range
            undef $range_start;
            redo;
        }
    }

    $output =~ s/^ //;
    return $output;
}

sub disp_str ($) {
    my $string = shift;

    # Displays the string unambiguously.  ASCII printables are always output
    # as-is, though perhaps separated by blanks from other characters.  If
    # entirely printable ASCII, just returns the string.  Otherwise if valid
    # UTF-8 it uses the character names for non-printable-ASCII.  Otherwise it
    # outputs hex for each non-ASCII-printable byte.

    return $string if $string =~ / ^ [[:print:]]* $/xa;

    my $result = "";
    my $prev_was_punct = 1; # Beginning is considered punct
    if (utf8::valid($string) && utf8::is_utf8($string)) {
        use charnames ();
        foreach my $char (split "", $string) {

            # Keep punctuation adjacent to other characters; otherwise
            # separate them with a blank
            if ($char =~ /[[:punct:]]/a) {
                $result .= $char;
                $prev_was_punct = 1;
            }
            elsif ($char =~ /[[:print:]]/a) {
                $result .= "  " unless $prev_was_punct;
                $result .= $char;
                $prev_was_punct = 0;
            }
            else {
                $result .= "  " unless $prev_was_punct;
                my $name = charnames::viacode(ord $char);
                $result .= (defined $name) ? $name : ':unknown:';
                $prev_was_punct = 0;
            }
        }
    }
    else {
        use bytes;
        foreach my $char (split "", $string) {
            if ($char =~ /[[:punct:]]/a) {
                $result .= $char;
                $prev_was_punct = 1;
            }
            elsif ($char =~ /[[:print:]]/a) {
                $result .= " " unless $prev_was_punct;
                $result .= $char;
                $prev_was_punct = 0;
            }
            else {
                $result .= " " unless $prev_was_punct;
                $result .= sprintf("%02X", ord $char);
                $prev_was_punct = 0;
            }
        }
    }

    return $result;
}

sub report_result {
    my ($Locale, $i, $pass_fail, $message) = @_;
    if ($pass_fail) {
	push @{$Okay{$i}}, $Locale;
    }
    else {
        $message //= "";
        $message = "  ($message)" if $message;
	$Known_bad_locale{$i}{$Locale} = 1 if exists $known_bad_locales{$os}
                                         && $Locale =~ $known_bad_locales{$os};
	$Problem{$i}{$Locale} = 1;
	debug "failed $i ($test_names{$i}) with locale '$Locale'$message\n";
    }
}

sub report_multi_result {
    my ($Locale, $i, $results_ref) = @_;

    # $results_ref points to an array, each element of which is a character that was
    # in error for this test numbered '$i'.  If empty, the test passed

    my $message = "";
    if (@$results_ref) {
        $message = join " ", "for", disp_chars(@$results_ref);
    }
    report_result($Locale, $i, @$results_ref == 0, $message);
}

my $first_locales_test_number = $final_without_setlocale
                              + 1 + @incompatible_locales;
my $locales_test_number;
my $not_necessarily_a_problem_test_number;
my $first_casing_test_number;
my %setlocale_failed;   # List of locales that setlocale() didn't work on

foreach my $Locale (@Locale) {
    $locales_test_number = $first_locales_test_number - 1;
    debug "\n";
    debug "Locale = $Locale\n";

    unless (setlocale(&POSIX::LC_ALL, $Locale)) {
        $setlocale_failed{$Locale} = $Locale;
	next;
    }

    # We test UTF-8 locales only under ':not_characters';  It is easier to
    # test them in other test files than here.  Non- UTF-8 locales are tested
    # only under plain 'use locale', as otherwise we would have to convert
    # everything in them to Unicode.

    my %UPPER = ();     # All alpha X for which uc(X) == X and lc(X) != X
    my %lower = ();     # All alpha X for which lc(X) == X and uc(X) != X
    my %BoThCaSe = ();  # All alpha X for which uc(X) == lc(X) == X

    my $is_utf8_locale = is_locale_utf8($Locale);

    if ($debug) {
        debug "code set = " . langinfo(CODESET);
        debug "is utf8 locale? = $is_utf8_locale\n";
        debug "radix = " . disp_str(langinfo(RADIXCHAR)) . "\n";
        debug "currency = " . disp_str(langinfo(CRNCYSTR));
    }

    if (! $is_utf8_locale) {
        use locale;
        @{$posixes{'word'}} = grep /\w/, map { chr } 0..255;
        @{$posixes{'digit'}} = grep /\d/, map { chr } 0..255;
        @{$posixes{'space'}} = grep /\s/, map { chr } 0..255;
        @{$posixes{'alpha'}} = grep /[[:alpha:]]/, map {chr } 0..255;
        @{$posixes{'alnum'}} = grep /[[:alnum:]]/, map {chr } 0..255;
        @{$posixes{'ascii'}} = grep /[[:ascii:]]/, map {chr } 0..255;
        @{$posixes{'blank'}} = grep /[[:blank:]]/, map {chr } 0..255;
        @{$posixes{'cntrl'}} = grep /[[:cntrl:]]/, map {chr } 0..255;
        @{$posixes{'graph'}} = grep /[[:graph:]]/, map {chr } 0..255;
        @{$posixes{'lower'}} = grep /[[:lower:]]/, map {chr } 0..255;
        @{$posixes{'print'}} = grep /[[:print:]]/, map {chr } 0..255;
        @{$posixes{'punct'}} = grep /[[:punct:]]/, map {chr } 0..255;
        @{$posixes{'upper'}} = grep /[[:upper:]]/, map {chr } 0..255;
        @{$posixes{'xdigit'}} = grep /[[:xdigit:]]/, map {chr } 0..255;
        @{$posixes{'cased'}} = grep /[[:upper:][:lower:]]/i, map {chr } 0..255;

        # Sieve the uppercase and the lowercase.

        for (@{$posixes{'word'}}) {
            if (/[^\d_]/) { # skip digits and the _
                if (uc($_) eq $_) {
                    $UPPER{$_} = $_;
                }
                if (lc($_) eq $_) {
                    $lower{$_} = $_;
                }
            }
        }
    }
    else {
        use locale ':not_characters';
        @{$posixes{'word'}} = grep /\w/, map { chr } 0..255;
        @{$posixes{'digit'}} = grep /\d/, map { chr } 0..255;
        @{$posixes{'space'}} = grep /\s/, map { chr } 0..255;
        @{$posixes{'alpha'}} = grep /[[:alpha:]]/, map {chr } 0..255;
        @{$posixes{'alnum'}} = grep /[[:alnum:]]/, map {chr } 0..255;
        @{$posixes{'ascii'}} = grep /[[:ascii:]]/, map {chr } 0..255;
        @{$posixes{'blank'}} = grep /[[:blank:]]/, map {chr } 0..255;
        @{$posixes{'cntrl'}} = grep /[[:cntrl:]]/, map {chr } 0..255;
        @{$posixes{'graph'}} = grep /[[:graph:]]/, map {chr } 0..255;
        @{$posixes{'lower'}} = grep /[[:lower:]]/, map {chr } 0..255;
        @{$posixes{'print'}} = grep /[[:print:]]/, map {chr } 0..255;
        @{$posixes{'punct'}} = grep /[[:punct:]]/, map {chr } 0..255;
        @{$posixes{'upper'}} = grep /[[:upper:]]/, map {chr } 0..255;
        @{$posixes{'xdigit'}} = grep /[[:xdigit:]]/, map {chr } 0..255;
        @{$posixes{'cased'}} = grep /[[:upper:][:lower:]]/i, map {chr } 0..255;
        for (@{$posixes{'word'}}) {
            if (/[^\d_]/) { # skip digits and the _
                if (uc($_) eq $_) {
                    $UPPER{$_} = $_;
                }
                if (lc($_) eq $_) {
                    $lower{$_} = $_;
                }
            }
        }
    }

    # Ordered, where possible,  in groups of "this is a subset of the next
    # one"
    debug ":upper:  = ", disp_chars(@{$posixes{'upper'}}), "\n";
    debug ":lower:  = ", disp_chars(@{$posixes{'lower'}}), "\n";
    debug ":cased:  = ", disp_chars(@{$posixes{'cased'}}), "\n";
    debug ":alpha:  = ", disp_chars(@{$posixes{'alpha'}}), "\n";
    debug ":alnum:  = ", disp_chars(@{$posixes{'alnum'}}), "\n";
    debug ' \w      = ', disp_chars(@{$posixes{'word'}}), "\n";
    debug ":graph:  = ", disp_chars(@{$posixes{'graph'}}), "\n";
    debug ":print:  = ", disp_chars(@{$posixes{'print'}}), "\n";
    debug ' \d      = ', disp_chars(@{$posixes{'digit'}}), "\n";
    debug ":xdigit: = ", disp_chars(@{$posixes{'xdigit'}}), "\n";
    debug ":blank:  = ", disp_chars(@{$posixes{'blank'}}), "\n";
    debug ' \s      = ', disp_chars(@{$posixes{'space'}}), "\n";
    debug ":punct:  = ", disp_chars(@{$posixes{'punct'}}), "\n";
    debug ":cntrl:  = ", disp_chars(@{$posixes{'cntrl'}}), "\n";
    debug ":ascii:  = ", disp_chars(@{$posixes{'ascii'}}), "\n";

    foreach (keys %UPPER) {

	$BoThCaSe{$_}++ if exists $lower{$_};
    }
    foreach (keys %lower) {
	$BoThCaSe{$_}++ if exists $UPPER{$_};
    }
    foreach (keys %BoThCaSe) {
	delete $UPPER{$_};
	delete $lower{$_};
    }

    my %Unassigned;
    foreach my $ord ( 0 .. 255 ) {
        $Unassigned{chr $ord} = 1;
    }
    foreach my $class (keys %posixes) {
        foreach my $char (@{$posixes{$class}}) {
            delete $Unassigned{$char};
        }
    }

    debug "UPPER    = ", disp_chars(sort { ord $a <=> ord $b } keys %UPPER), "\n";
    debug "lower    = ", disp_chars(sort { ord $a <=> ord $b } keys %lower), "\n";
    debug "BoThCaSe = ", disp_chars(sort { ord $a <=> ord $b } keys %BoThCaSe), "\n";
    debug "Unassigned = ", disp_chars(sort { ord $a <=> ord $b } keys %Unassigned), "\n";

    my @failures;
    my @fold_failures;
    foreach my $x (sort { ord $a <=> ord $b } keys %UPPER) {
        my $ok;
        my $fold_ok;
        if ($is_utf8_locale) {
            use locale ':not_characters';
            $ok = $x =~ /[[:upper:]]/;
            $fold_ok = $x =~ /[[:lower:]]/i;
        }
        else {
            use locale;
            $ok = $x =~ /[[:upper:]]/;
            $fold_ok = $x =~ /[[:lower:]]/i;
        }
        push @failures, $x unless $ok;
        push @fold_failures, $x unless $fold_ok;
    }
    $locales_test_number++;
    $first_casing_test_number = $locales_test_number;
    $test_names{$locales_test_number} = 'Verify that /[[:upper:]]/ matches all alpha X for which uc(X) == X and lc(X) != X';
    report_multi_result($Locale, $locales_test_number, \@failures);

    $locales_test_number++;

    $test_names{$locales_test_number} = 'Verify that /[[:lower:]]/i matches all alpha X for which uc(X) == X and lc(X) != X';
    report_multi_result($Locale, $locales_test_number, \@fold_failures);

    undef @failures;
    undef @fold_failures;

    foreach my $x (sort { ord $a <=> ord $b } keys %lower) {
        my $ok;
        my $fold_ok;
        if ($is_utf8_locale) {
            use locale ':not_characters';
            $ok = $x =~ /[[:lower:]]/;
            $fold_ok = $x =~ /[[:upper:]]/i;
        }
        else {
            use locale;
            $ok = $x =~ /[[:lower:]]/;
            $fold_ok = $x =~ /[[:upper:]]/i;
        }
        push @failures, $x unless $ok;
        push @fold_failures, $x unless $fold_ok;
    }

    $locales_test_number++;
    $test_names{$locales_test_number} = 'Verify that /[[:lower:]]/ matches all alpha X for which lc(X) == X and uc(X) != X';
    report_multi_result($Locale, $locales_test_number, \@failures);

    $locales_test_number++;
    $test_names{$locales_test_number} = 'Verify that /[[:upper:]]/i matches all alpha X for which lc(X) == X and uc(X) != X';
    report_multi_result($Locale, $locales_test_number, \@fold_failures);

    {   # Find the alphabetic characters that are not considered alphabetics
        # in the default (C) locale.

	no locale;

	@Added_alpha = ();
	for (keys %UPPER, keys %lower, keys %BoThCaSe) {
	    push(@Added_alpha, $_) if (/\W/);
	}
    }

    @Added_alpha = sort { ord $a <=> ord $b } @Added_alpha;

    debug "Added_alpha = ", disp_chars(@Added_alpha), "\n";

    # Cross-check the whole 8-bit character set.

    ++$locales_test_number;
    my @f;
    $test_names{$locales_test_number} = 'Verify that \w and [:word:] are identical';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ unless /[[:word:]]/ == /\w/;
        }
        else {
            push @f, $_ unless /[[:word:]]/ == /\w/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that \d and [:digit:] are identical';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ unless /[[:digit:]]/ == /\d/;
        }
        else {
            push @f, $_ unless /[[:digit:]]/ == /\d/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that \s and [:space:] are identical';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ unless /[[:space:]]/ == /\s/;
        }
        else {
            push @f, $_ unless /[[:space:]]/ == /\s/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:posix:] and [:^posix:] are mutually exclusive';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ unless   (/[[:alpha:]]/ xor /[[:^alpha:]]/)   ||
                    (/[[:alnum:]]/ xor /[[:^alnum:]]/)   ||
                    (/[[:ascii:]]/ xor /[[:^ascii:]]/)   ||
                    (/[[:blank:]]/ xor /[[:^blank:]]/)   ||
                    (/[[:cntrl:]]/ xor /[[:^cntrl:]]/)   ||
                    (/[[:digit:]]/ xor /[[:^digit:]]/)   ||
                    (/[[:graph:]]/ xor /[[:^graph:]]/)   ||
                    (/[[:lower:]]/ xor /[[:^lower:]]/)   ||
                    (/[[:print:]]/ xor /[[:^print:]]/)   ||
                    (/[[:space:]]/ xor /[[:^space:]]/)   ||
                    (/[[:upper:]]/ xor /[[:^upper:]]/)   ||
                    (/[[:word:]]/  xor /[[:^word:]]/)    ||
                    (/[[:xdigit:]]/ xor /[[:^xdigit:]]/) ||

                    # effectively is what [:cased:] would be if it existed.
                    (/[[:upper:][:lower:]]/i xor /[^[:upper:][:lower:]]/i);
        }
        else {
            push @f, $_ unless   (/[[:alpha:]]/ xor /[[:^alpha:]]/)   ||
                    (/[[:alnum:]]/ xor /[[:^alnum:]]/)   ||
                    (/[[:ascii:]]/ xor /[[:^ascii:]]/)   ||
                    (/[[:blank:]]/ xor /[[:^blank:]]/)   ||
                    (/[[:cntrl:]]/ xor /[[:^cntrl:]]/)   ||
                    (/[[:digit:]]/ xor /[[:^digit:]]/)   ||
                    (/[[:graph:]]/ xor /[[:^graph:]]/)   ||
                    (/[[:lower:]]/ xor /[[:^lower:]]/)   ||
                    (/[[:print:]]/ xor /[[:^print:]]/)   ||
                    (/[[:space:]]/ xor /[[:^space:]]/)   ||
                    (/[[:upper:]]/ xor /[[:^upper:]]/)   ||
                    (/[[:word:]]/  xor /[[:^word:]]/)    ||
                    (/[[:xdigit:]]/ xor /[[:^xdigit:]]/) ||
                    (/[[:upper:][:lower:]]/i xor /[^[:upper:][:lower:]]/i);
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    # The rules for the relationships are given in:
    # http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap07.html


    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:lower:] contains at least a-z';
    for ('a' .. 'z') {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_  unless /[[:lower:]]/;
        }
        else {
            push @f, $_  unless /[[:lower:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:lower:] is a subset of [:alpha:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_  if /[[:lower:]]/ and ! /[[:alpha:]]/;
        }
        else {
            push @f, $_  if /[[:lower:]]/ and ! /[[:alpha:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:upper:] contains at least A-Z';
    for ('A' .. 'Z') {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_  unless /[[:upper:]]/;
        }
        else {
            push @f, $_  unless /[[:upper:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:upper:] is a subset of [:alpha:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_  if /[[:upper:]]/ and ! /[[:alpha:]]/;
        }
        else {
            push @f, $_ if /[[:upper:]]/  and ! /[[:alpha:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that /[[:lower:]]/i is a subset of [:alpha:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:lower:]]/i  and ! /[[:alpha:]]/;
        }
        else {
            push @f, $_ if /[[:lower:]]/i  and ! /[[:alpha:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:alpha:] is a subset of [:alnum:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:alpha:]]/  and ! /[[:alnum:]]/;
        }
        else {
            push @f, $_ if /[[:alpha:]]/  and ! /[[:alnum:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:digit:] contains at least 0-9';
    for ('0' .. '9') {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_  unless /[[:digit:]]/;
        }
        else {
            push @f, $_  unless /[[:digit:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:digit:] is a subset of [:alnum:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:digit:]]/  and ! /[[:alnum:]]/;
        }
        else {
            push @f, $_ if /[[:digit:]]/  and ! /[[:alnum:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:digit:] matches either 10 or 20 code points';
    report_result($Locale, $locales_test_number, @{$posixes{'digit'}} == 10 || @{$posixes{'digit'}} == 20);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that if there is a second set of digits in [:digit:], they are consecutive';
    if (@{$posixes{'digit'}} == 20) {
        my $previous_ord;
        for (map { chr } 0..255) {
            next unless /[[:digit:]]/;
            next if /[0-9]/;
            if (defined $previous_ord) {
                if ($is_utf8_locale) {
                    use locale ':not_characters';
                    push @f, $_ if ord $_ != $previous_ord + 1;
                }
                else {
                    push @f, $_ if ord $_ != $previous_ord + 1;
                }
            }
            $previous_ord = ord $_;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    my @xdigit_digits;  # :digit: & :xdigit:
    $test_names{$locales_test_number} = 'Verify that [:xdigit:] contains one or two blocks of 10 consecutive [:digit:] chars';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            # For utf8 locales, we actually use a stricter test: that :digit:
            # is a subset of :xdigit:, as we know that only 0-9 should match
            push @f, $_ if /[[:digit:]]/ and ! /[[:xdigit:]]/;
        }
        else {
            push @xdigit_digits, $_ if /[[:digit:]]/ and /[[:xdigit:]]/;
        }
    }
    if (! $is_utf8_locale) {

        # For non-utf8 locales, @xdigit_digits is a list of the characters
        # that are both :xdigit: and :digit:.  Because :digit: is stored in
        # increasing code point order (unless the tests above failed),
        # @xdigit_digits is as well.  There should be exactly 10 or
        # 20 of these.
        if (@xdigit_digits != 10 && @xdigit_digits != 20) {
            @f = @xdigit_digits;
        }
        else {

            # Look for contiguity in the series, adding any wrong ones to @f
            my @temp = @xdigit_digits;
            while (@temp > 1) {
                push @f, $temp[1] if ($temp[0] != $temp[1] - 1)

                                     # Skip this test for the 0th character of
                                     # the second block of 10, as it won't be
                                     # contiguous with the previous block
                                     && (! defined $xdigit_digits[10]
                                         || $temp[1] != $xdigit_digits[10]);
                shift @temp;
            }
        }
    }

    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:xdigit:] contains at least A-F, a-f';
    for ('A' .. 'F', 'a' .. 'f') {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_  unless /[[:xdigit:]]/;
        }
        else {
            push @f, $_  unless /[[:xdigit:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that any additional members of [:xdigit:], are in groups of 6 consecutive code points';
    my $previous_ord;
    my $count = 0;
    for my $chr (map { chr } 0..255) {
        next unless $chr =~ /[[:xdigit:]]/;
        if ($is_utf8_locale) {
            next if $chr =~ /[[:digit:]]/;
        }
        else {
            next if grep { $chr eq $_ } @xdigit_digits;
        }
        next if $chr =~ /[A-Fa-f]/;
        if (defined $previous_ord) {
            if ($is_utf8_locale) {
                use locale ':not_characters';
                push @f, $chr if ord $chr != $previous_ord + 1;
            }
            else {
                push @f, $chr if ord $chr != $previous_ord + 1;
            }
        }
        $count++;
        if ($count == 6) {
            undef $previous_ord;
        }
        else {
            $previous_ord = ord $chr;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:xdigit:] is a subset of [:graph:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:xdigit:]]/  and ! /[[:graph:]]/;
        }
        else {
            push @f, $_ if /[[:xdigit:]]/  and ! /[[:graph:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    # Note that xdigit doesn't have to be a subset of alnum

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:punct:] is a subset of [:graph:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:punct:]]/  and ! /[[:graph:]]/;
        }
        else {
            push @f, $_ if /[[:punct:]]/  and ! /[[:graph:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that the space character is not in [:graph:]';
    if ($is_utf8_locale) {
        use locale ':not_characters';
        push @f, " " if " " =~ /[[:graph:]]/;
    }
    else {
        push @f, " " if " " =~ /[[:graph:]]/;
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:space:] contains at least [\f\n\r\t\cK ]';
    for (' ', "\f", "\n", "\r", "\t", "\cK") {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_  unless /[[:space:]]/;
        }
        else {
            push @f, $_  unless /[[:space:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:blank:] contains at least [\t ]';
    for (' ', "\t") {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_  unless /[[:blank:]]/;
        }
        else {
            push @f, $_  unless /[[:blank:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:blank:] is a subset of [:space:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:blank:]]/  and ! /[[:space:]]/;
        }
        else {
            push @f, $_ if /[[:blank:]]/  and ! /[[:space:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that [:graph:] is a subset of [:print:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:graph:]]/  and ! /[[:print:]]/;
        }
        else {
            push @f, $_ if /[[:graph:]]/  and ! /[[:print:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that the space character is in [:print:]';
    if ($is_utf8_locale) {
        use locale ':not_characters';
        push @f, " " if " " !~ /[[:print:]]/;
    }
    else {
        push @f, " " if " " !~ /[[:print:]]/;
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that isn\'t both [:cntrl:] and [:print:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if (/[[:print:]]/ and /[[:cntrl:]]/);
        }
        else {
            push @f, $_ if (/[[:print:]]/ and /[[:cntrl:]]/);
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that isn\'t both [:alpha:] and [:digit:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:alpha:]]/ and /[[:digit:]]/;
        }
        else {
            push @f, $_ if /[[:alpha:]]/ and /[[:digit:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that isn\'t both [:alnum:] and [:punct:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if /[[:alnum:]]/ and /[[:punct:]]/;
        }
        else {
            push @f, $_ if /[[:alnum:]]/ and /[[:punct:]]/;
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that isn\'t both [:xdigit:] and [:punct:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if (/[[:punct:]]/ and /[[:xdigit:]]/);
        }
        else {
            push @f, $_ if (/[[:punct:]]/ and /[[:xdigit:]]/);
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    ++$locales_test_number;
    undef @f;
    $test_names{$locales_test_number} = 'Verify that isn\'t both [:graph:] and [:space:]';
    for (map { chr } 0..255) {
        if ($is_utf8_locale) {
            use locale ':not_characters';
            push @f, $_ if (/[[:graph:]]/ and /[[:space:]]/);
        }
        else {
            push @f, $_ if (/[[:graph:]]/ and /[[:space:]]/);
        }
    }
    report_multi_result($Locale, $locales_test_number, \@f);

    foreach ($first_casing_test_number..$locales_test_number) {
        $problematical_tests{$_} = 1;
    }


    # Test for read-only scalars' locale vs non-locale comparisons.

    {
        no locale;
        my $ok;
        $a = "qwerty";
        if ($is_utf8_locale) {
            use locale ':not_characters';
            $ok = ($a cmp "qwerty") == 0;
        }
        else {
            use locale;
            $ok = ($a cmp "qwerty") == 0;
        }
        report_result($Locale, ++$locales_test_number, $ok);
        $test_names{$locales_test_number} = 'Verify that cmp works with a read-only scalar; no- vs locale';
    }

    {
        my ($from, $to, $lesser, $greater,
            @test, %test, $test, $yes, $no, $sign);

        ++$locales_test_number;
        $test_names{$locales_test_number} = 'Verify that "le", "ne", etc work';
        $not_necessarily_a_problem_test_number = $locales_test_number;
        for (0..9) {
            # Select a slice.
            $from = int(($_*@{$posixes{'word'}})/10);
            $to = $from + int(@{$posixes{'word'}}/10);
            $to = $#{$posixes{'word'}} if ($to > $#{$posixes{'word'}});
            $lesser  = join('', @{$posixes{'word'}}[$from..$to]);
            # Select a slice one character on.
            $from++; $to++;
            $to = $#{$posixes{'word'}} if ($to > $#{$posixes{'word'}});
            $greater = join('', @{$posixes{'word'}}[$from..$to]);
            if ($is_utf8_locale) {
                use locale ':not_characters';
                ($yes, $no, $sign) = ($lesser lt $greater
                                    ? ("    ", "not ", 1)
                                    : ("not ", "    ", -1));
            }
            else {
                use locale;
                ($yes, $no, $sign) = ($lesser lt $greater
                                    ? ("    ", "not ", 1)
                                    : ("not ", "    ", -1));
            }
            # all these tests should FAIL (return 0).  Exact lt or gt cannot
            # be tested because in some locales, say, eacute and E may test
            # equal.
            @test =
                (
                    $no.'    ($lesser  le $greater)',  # 1
                    'not      ($lesser  ne $greater)', # 2
                    '         ($lesser  eq $greater)', # 3
                    $yes.'    ($lesser  ge $greater)', # 4
                    $yes.'    ($lesser  ge $greater)', # 5
                    $yes.'    ($greater le $lesser )', # 7
                    'not      ($greater ne $lesser )', # 8
                    '         ($greater eq $lesser )', # 9
                    $no.'     ($greater ge $lesser )', # 10
                    'not (($lesser cmp $greater) == -($sign))' # 11
                    );
            @test{@test} = 0 x @test;
            $test = 0;
            for my $ti (@test) {
                if ($is_utf8_locale) {
                    use locale ':not_characters';
                    $test{$ti} = eval $ti;
                }
                else {
                    # Already in 'use locale';
                    $test{$ti} = eval $ti;
                }
                $test ||= $test{$ti}
            }
            report_result($Locale, $locales_test_number, $test == 0);
            if ($test) {
                debug "lesser  = '$lesser'\n";
                debug "greater = '$greater'\n";
                debug "lesser cmp greater = ",
                        $lesser cmp $greater, "\n";
                debug "greater cmp lesser = ",
                        $greater cmp $lesser, "\n";
                debug "(greater) from = $from, to = $to\n";
                for my $ti (@test) {
                    debugf("# %-40s %-4s", $ti,
                            $test{$ti} ? 'FAIL' : 'ok');
                    if ($ti =~ /\(\.*(\$.+ +cmp +\$[^\)]+)\.*\)/) {
                        debugf("(%s == %4d)", $1, eval $1);
                    }
                    debugf("\n#");
                }

                last;
            }
        }

        use locale;

        my @sorted_controls;

        ++$locales_test_number;
        $test_names{$locales_test_number}
                = 'Skip in locales where there are no controls;'
                . ' otherwise verify that \0 sorts before any (other) control';
        if (! $posixes{'cntrl'}) {
            report_result($Locale, $locales_test_number, 1);

            # We use all code points for the tests below since there aren't
            # any controls
            push @sorted_controls, chr $_ for 1..255;
            @sorted_controls = sort @sorted_controls;
        }
        else {
            @sorted_controls = @{$posixes{'cntrl'}};
            push @sorted_controls, "\0",
                                unless grep { $_ eq "\0" } @sorted_controls;
            @sorted_controls = sort @sorted_controls;
            my $output = "";
            for my $control (@sorted_controls) {
                $output .= " " . disp_chars($control);
            }
            debug "sorted :cntrl: (plus NUL) = $output\n";
            my $ok = $sorted_controls[0] eq "\0";
            report_result($Locale, $locales_test_number, $ok);

            shift @sorted_controls if $ok;
        }

        my $lowest_control = $sorted_controls[0];

        ++$locales_test_number;
        $test_names{$locales_test_number}
            = 'Skip in locales where all controls have primary sorting weight; '
            . 'otherwise verify that \0 doesn\'t have primary sorting weight';
        if ("a${lowest_control}c" lt "ab") {
            report_result($Locale, $locales_test_number, 1);
        }
        else {
            my $ok = "ab" lt "a\0c";
            report_result($Locale, $locales_test_number, $ok);
        }

        ++$locales_test_number;
        $test_names{$locales_test_number}
                            = 'Verify that strings with embedded NUL collate';
        my $ok = "a\0a\0a" lt "a${lowest_control}a${lowest_control}a";
        report_result($Locale, $locales_test_number, $ok);

        ++$locales_test_number;
        $test_names{$locales_test_number}
                            = 'Verify that strings with embedded NUL and '
                            . 'extra trailing NUL collate';
        $ok = "a\0a\0" lt "a${lowest_control}a${lowest_control}";
        report_result($Locale, $locales_test_number, $ok);

        ++$locales_test_number;
        $test_names{$locales_test_number}
                            = 'Verify that empty strings collate';
        $ok = "" le "";
        report_result($Locale, $locales_test_number, $ok);

        ++$locales_test_number;
        $test_names{$locales_test_number}
            = "Skip in non-UTF-8 locales; otherwise verify that UTF8ness "
            . "doesn't matter with collation";
        if (! $is_utf8_locale) {
            report_result($Locale, $locales_test_number, 1);
        }
        else {

            # khw can't think of anything better.  Start with a string that is
            # higher than its UTF-8 representation in both EBCDIC and ASCII
            my $string = chr utf8::unicode_to_native(0xff);
            my $utf8_string = $string;
            utf8::upgrade($utf8_string);

            # 8 should be lt 9 in all locales (except ones that aren't
            # ASCII-based, which might fail this)
            $ok = ("a${string}8") lt ("a${utf8_string}9");
            report_result($Locale, $locales_test_number, $ok);
        }

        ++$locales_test_number;
        $test_names{$locales_test_number}
            = "Skip in UTF-8 locales; otherwise verify that single byte "
            . "collates before 0x100 and above";
        if ($is_utf8_locale) {
            report_result($Locale, $locales_test_number, 1);
        }
        else {
            my $max_collating = chr 0;  # Find byte that collates highest
            for my $i (0 .. 255) {
                my $char = chr $i;
                $max_collating = $char if $char gt $max_collating;
            }
            $ok = $max_collating lt chr 0x100;
            report_result($Locale, $locales_test_number, $ok);
        }

        ++$locales_test_number;
        $test_names{$locales_test_number}
            = "Skip in UTF-8 locales; otherwise verify that 0x100 and "
            . "above collate in code point order";
        if ($is_utf8_locale) {
            report_result($Locale, $locales_test_number, 1);
        }
        else {
            $ok = chr 0x100 lt chr 0x101;
            report_result($Locale, $locales_test_number, $ok);
        }
    }

    my $ok1;
    my $ok2;
    my $ok3;
    my $ok4;
    my $ok5;
    my $ok6;
    my $ok7;
    my $ok8;
    my $ok9;
    my $ok10;
    my $ok11;
    my $ok12;
    my $ok13;
    my $ok14;
    my $ok14_5;
    my $ok15;
    my $ok16;
    my $ok17;
    my $ok18;
    my $ok19;
    my $ok20;
    my $ok21;

    my $c;
    my $d;
    my $e;
    my $f;
    my $g;
    my $h;
    my $i;
    my $j;

    if (! $is_utf8_locale) {
        use locale;

        my ($x, $y) = (1.23, 1.23);

        $a = "$x";
        printf ''; # printf used to reset locale to "C"
        $b = "$y";
        $ok1 = $a eq $b;

        $c = "$x";
        my $z = sprintf ''; # sprintf used to reset locale to "C"
        $d = "$y";
        $ok2 = $c eq $d;
        {

            use warnings;
            my $w = 0;
            local $SIG{__WARN__} =
                sub {
                    print "# @_\n";
                    $w++;
                };

            # The == (among other ops) used to warn for locales
            # that had something else than "." as the radix character.

            $ok3 = $c == 1.23;
            $ok4 = $c == $x;
            $ok5 = $c == $d;
            {
                no locale;

                $e = "$x";

                $ok6 = $e == 1.23;
                $ok7 = $e == $x;
                $ok8 = $e == $c;
            }

            $f = "1.23";
            $g = 2.34;
            $h = 1.5;
            $i = 1.25;
            $j = "$h:$i";

            $ok9 = $f == 1.23;
            $ok10 = $f == $x;
            $ok11 = $f == $c;
            $ok12 = abs(($f + $g) - 3.57) < 0.01;
            $ok13 = $w == 0;
            $ok14 = $ok14_5 = $ok15 = $ok16 = 1;  # Skip for non-utf8 locales
        }
        {
            no locale;
            $ok17 = "1.5:1.25" eq sprintf("%g:%g", $h, $i);
        }
        $ok18 = $j eq sprintf("%g:%g", $h, $i);
    }
    else {
        use locale ':not_characters';

        my ($x, $y) = (1.23, 1.23);
        $a = "$x";
        printf ''; # printf used to reset locale to "C"
        $b = "$y";
        $ok1 = $a eq $b;

        $c = "$x";
        my $z = sprintf ''; # sprintf used to reset locale to "C"
        $d = "$y";
        $ok2 = $c eq $d;
        {
            use warnings;
            my $w = 0;
            local $SIG{__WARN__} =
                sub {
                    print "# @_\n";
                    $w++;
                };
            $ok3 = $c == 1.23;
            $ok4 = $c == $x;
            $ok5 = $c == $d;
            {
                no locale;
                $e = "$x";

                $ok6 = $e == 1.23;
                $ok7 = $e == $x;
                $ok8 = $e == $c;
            }

            $f = "1.23";
            $g = 2.34;
            $h = 1.5;
            $i = 1.25;
            $j = "$h:$i";

            $ok9 = $f == 1.23;
            $ok10 = $f == $x;
            $ok11 = $f == $c;
            $ok12 = abs(($f + $g) - 3.57) < 0.01;
            $ok13 = $w == 0;

            # Look for non-ASCII error messages, and verify that the first
            # such is in UTF-8 (the others almost certainly will be like the
            # first).  This is only done if the current locale has LC_MESSAGES
            $ok14 = 1;
            $ok14_5 = 1;
            if (   locales_enabled('LC_MESSAGES')
                && setlocale(&POSIX::LC_MESSAGES, $Locale))
            {
                foreach my $err (keys %!) {
                    use Errno;
                    $! = eval "&Errno::$err";   # Convert to strerror() output
                    my $errnum = 0+$!;
                    my $strerror = "$!";
                    if ("$strerror" =~ /\P{ASCII}/) {
                        $ok14 = utf8::is_utf8($strerror);
                        no locale;
                        $ok14_5 = "$!" !~ /\P{ASCII}/;
                        debug( disp_str(
                        "non-ASCII \$! for error $errnum='$strerror'"))
                                                                   if ! $ok14_5;
                        last;
                    }
                }
            }

            # Similarly, we verify that a non-ASCII radix is in UTF-8.  This
            # also catches if there is a disparity between sprintf and
            # stringification.

            my $string_g = "$g";
            my $sprintf_g = sprintf("%g", $g);

            $ok15 = $string_g =~ / ^ \p{ASCII}+ $ /x || utf8::is_utf8($string_g);
            $ok16 = $sprintf_g eq $string_g;
        }
        {
            no locale;
            $ok17 = "1.5:1.25" eq sprintf("%g:%g", $h, $i);
        }
        $ok18 = $j eq sprintf("%g:%g", $h, $i);
    }

    $ok19 = $ok20 = 1;
    if (locales_enabled('LC_TIME')) {
        if (setlocale(&POSIX::LC_TIME, $Locale)) { # These tests aren't
                                                   # affected by
                                                   # :not_characters
            my @times = CORE::localtime();

            use locale;
            $ok19 = POSIX::strftime("%p", @times) ne "%p"; # [perl #119425]
            my $date = POSIX::strftime("'%A'  '%B'  '%Z'  '%p'", @times);
            debug("'Day' 'Month' 'TZ' 'am/pm' = ", disp_str($date));

            # If there is any non-ascii, it better be UTF-8 in a UTF-8 locale,
            # and not UTF-8 if the locale isn't UTF-8.
            $ok20 = $date =~ / ^ \p{ASCII}+ $ /x
                    || $is_utf8_locale == utf8::is_utf8($date);
        }
    }

    $ok21 = 1;
    if (locales_enabled('LC_MESSAGES')) {
        foreach my $err (keys %!) {
            no locale;
            use Errno;
            $! = eval "&Errno::$err";   # Convert to strerror() output
            my $strerror = "$!";
            if ($strerror =~ /\P{ASCII}/) {
                $ok21 = 0;
                debug(disp_str("non-ASCII strerror=$strerror"));
                last;
            }
        }
    }

    report_result($Locale, ++$locales_test_number, $ok1);
    $test_names{$locales_test_number} = 'Verify that an intervening printf doesn\'t change assignment results';
    my $first_a_test = $locales_test_number;

    debug "$first_a_test..$locales_test_number: \$a = $a, \$b = $b, Locale = $Locale\n";

    report_result($Locale, ++$locales_test_number, $ok2);
    $test_names{$locales_test_number} = 'Verify that an intervening sprintf doesn\'t change assignment results';

    my $first_c_test = $locales_test_number;

    $test_names{++$locales_test_number} = 'Verify that a different locale radix works when doing "==" with a constant';
    report_result($Locale, $locales_test_number, $ok3);
    $problematical_tests{$locales_test_number} = 1;

    $test_names{++$locales_test_number} = 'Verify that a different locale radix works when doing "==" with a scalar';
    report_result($Locale, $locales_test_number, $ok4);
    $problematical_tests{$locales_test_number} = 1;

    report_result($Locale, ++$locales_test_number, $ok5);
    $test_names{$locales_test_number} = 'Verify that a different locale radix works when doing "==" with a scalar and an intervening sprintf';
    $problematical_tests{$locales_test_number} = 1;

    debug "$first_c_test..$locales_test_number: \$c = $c, \$d = $d, Locale = $Locale\n";

    report_result($Locale, ++$locales_test_number, $ok6);
    $test_names{$locales_test_number} = 'Verify that can assign stringified under inner no-locale block';
    my $first_e_test = $locales_test_number;

    report_result($Locale, ++$locales_test_number, $ok7);
    $test_names{$locales_test_number} = 'Verify that "==" with a scalar still works in inner no locale';

    $test_names{++$locales_test_number} = 'Verify that "==" with a scalar and an intervening sprintf still works in inner no locale';
    report_result($Locale, $locales_test_number, $ok8);
    $problematical_tests{$locales_test_number} = 1;

    debug "$first_e_test..$locales_test_number: \$e = $e, no locale\n";

    report_result($Locale, ++$locales_test_number, $ok9);
    $test_names{$locales_test_number} = 'Verify that after a no-locale block, a different locale radix still works when doing "==" with a constant';
    $problematical_tests{$locales_test_number} = 1;
    my $first_f_test = $locales_test_number;

    report_result($Locale, ++$locales_test_number, $ok10);
    $test_names{$locales_test_number} = 'Verify that after a no-locale block, a different locale radix still works when doing "==" with a scalar';
    $problematical_tests{$locales_test_number} = 1;

    $test_names{++$locales_test_number} = 'Verify that after a no-locale block, a different locale radix still works when doing "==" with a scalar and an intervening sprintf';
    report_result($Locale, $locales_test_number, $ok11);
    $problematical_tests{$locales_test_number} = 1;

    report_result($Locale, ++$locales_test_number, $ok12);
    $test_names{$locales_test_number} = 'Verify that after a no-locale block, a different locale radix can participate in an addition and function call as numeric';
    $problematical_tests{$locales_test_number} = 1;

    report_result($Locale, ++$locales_test_number, $ok13);
    $test_names{$locales_test_number} = 'Verify that don\'t get warning under "==" even if radix is not a dot';
    $problematical_tests{$locales_test_number} = 1;

    report_result($Locale, ++$locales_test_number, $ok14);
    $test_names{$locales_test_number} = 'Verify that non-ASCII UTF-8 error messages are in UTF-8';

    report_result($Locale, ++$locales_test_number, $ok14_5);
    $test_names{$locales_test_number} = '... and are ASCII outside "use locale"';

    report_result($Locale, ++$locales_test_number, $ok15);
    $test_names{$locales_test_number} = 'Verify that a number with a UTF-8 radix has a UTF-8 stringification';
    $problematical_tests{$locales_test_number} = 1;

    report_result($Locale, ++$locales_test_number, $ok16);
    $test_names{$locales_test_number} = 'Verify that a sprintf of a number with a UTF-8 radix yields UTF-8';
    $problematical_tests{$locales_test_number} = 1;

    report_result($Locale, ++$locales_test_number, $ok17);
    $test_names{$locales_test_number} = 'Verify that a sprintf of a number outside locale scope uses a dot radix';

    report_result($Locale, ++$locales_test_number, $ok18);
    $test_names{$locales_test_number} = 'Verify that a sprintf of a number back within locale scope uses locale radix';
    $problematical_tests{$locales_test_number} = 1;

    report_result($Locale, ++$locales_test_number, $ok19);
    $test_names{$locales_test_number} = 'Verify that strftime doesn\'t return "%p" in locales where %p is empty';

    report_result($Locale, ++$locales_test_number, $ok20);
    $test_names{$locales_test_number} = 'Verify that strftime returns date with UTF-8 flag appropriately set';
    $problematical_tests{$locales_test_number} = 1;   # This is broken in
                                                      # OS X 10.9.3

    report_result($Locale, ++$locales_test_number, $ok21);
    $test_names{$locales_test_number} = '"$!" is ASCII-only outside of locale scope';

    debug "$first_f_test..$locales_test_number: \$f = $f, \$g = $g, back to locale = $Locale\n";

    # Does taking lc separately differ from taking
    # the lc "in-line"?  (This was the bug 19990704.002 (#965), change #3568.)
    # The bug was in the caching of the 'o'-magic.
    if (! $is_utf8_locale) {
	use locale;

	sub lcA {
	    my $lc0 = lc $_[0];
	    my $lc1 = lc $_[1];
	    return $lc0 cmp $lc1;
	}

        sub lcB {
	    return lc($_[0]) cmp lc($_[1]);
	}

        my $x = "ab";
        my $y = "aa";
        my $z = "AB";

        report_result($Locale, ++$locales_test_number,
		    lcA($x, $y) == 1 && lcB($x, $y) == 1 ||
		    lcA($x, $z) == 0 && lcB($x, $z) == 0);
    }
    else {
	use locale ':not_characters';

	sub lcC {
	    my $lc0 = lc $_[0];
	    my $lc1 = lc $_[1];
	    return $lc0 cmp $lc1;
	}

        sub lcD {
	    return lc($_[0]) cmp lc($_[1]);
	}

        my $x = "ab";
        my $y = "aa";
        my $z = "AB";

        report_result($Locale, ++$locales_test_number,
		    lcC($x, $y) == 1 && lcD($x, $y) == 1 ||
		    lcC($x, $z) == 0 && lcD($x, $z) == 0);
    }
    $test_names{$locales_test_number} = 'Verify "lc(foo) cmp lc(bar)" is the same as using intermediaries for the cmp';

    # Does lc of an UPPER (if different from the UPPER) match
    # case-insensitively the UPPER, and does the UPPER match
    # case-insensitively the lc of the UPPER.  And vice versa.
    {
        use locale;
        no utf8;
        my $re = qr/[\[\(\{\*\+\?\|\^\$\\]/;

        my @f = ();
        ++$locales_test_number;
        $test_names{$locales_test_number} = 'Verify case insensitive matching works';
        foreach my $x (sort { ord $a <=> ord $b } keys %UPPER) {
            if (! $is_utf8_locale) {
                my $y = lc $x;
                next unless uc $y eq $x;
                debug_more( "UPPER=", disp_chars(($x)),
                            "; lc=", disp_chars(($y)), "; ",
                            "; fc=", disp_chars((fc $x)), "; ",
                            disp_chars(($x)), "=~/", disp_chars(($y)), "/i=",
                            $x =~ /\Q$y/i ? 1 : 0,
                            "; ",
                            disp_chars(($y)), "=~/", disp_chars(($x)), "/i=",
                            $y =~ /\Q$x/i ? 1 : 0,
                            "\n");
                #
                # If $x and $y contain regular expression characters
                # AND THEY lowercase (/i) to regular expression characters,
                # regcomp() will be mightily confused.  No, the \Q doesn't
                # help here (maybe regex engine internal lowercasing
                # is done after the \Q?)  An example of this happening is
                # the bg_BG (Bulgarian) locale under EBCDIC (OS/390 USS):
                # the chr(173) (the "[") is the lowercase of the chr(235).
                #
                # Similarly losing EBCDIC locales include cs_cz, cs_CZ,
                # el_gr, el_GR, en_us.IBM-037 (!), en_US.IBM-037 (!),
                # et_ee, et_EE, hr_hr, hr_HR, hu_hu, hu_HU, lt_LT,
                # mk_mk, mk_MK, nl_nl.IBM-037, nl_NL.IBM-037,
                # pl_pl, pl_PL, ro_ro, ro_RO, ru_ru, ru_RU,
                # sk_sk, sk_SK, sl_si, sl_SI, tr_tr, tr_TR.
                #
                # Similar things can happen even under (bastardised)
                # non-EBCDIC locales: in many European countries before the
                # advent of ISO 8859-x nationally customised versions of
                # ISO 646 were devised, reusing certain punctuation
                # characters for modified characters needed by the
                # country/language.  For example, the "|" might have
                # stood for U+00F6 or LATIN SMALL LETTER O WITH DIAERESIS.
                #
                if ($x =~ $re || $y =~ $re) {
                    print "# Regex characters in '$x' or '$y', skipping test $locales_test_number for locale '$Locale'\n";
                    next;
                }
                push @f, $x unless $x =~ /\Q$y/i && $y =~ /\Q$x/i;

                # fc is not a locale concept, so Perl uses lc for it.
                push @f, $x unless lc $x eq fc $x;
            }
            else {
                use locale ':not_characters';
                my $y = lc $x;
                next unless uc $y eq $x;
                debug_more( "UPPER=", disp_chars(($x)),
                            "; lc=", disp_chars(($y)), "; ",
                            "; fc=", disp_chars((fc $x)), "; ",
                            disp_chars(($x)), "=~/", disp_chars(($y)), "/i=",
                            $x =~ /\Q$y/i ? 1 : 0,
                            "; ",
                            disp_chars(($y)), "=~/", disp_chars(($x)), "/i=",
                            $y =~ /\Q$x/i ? 1 : 0,
                            "\n");

                push @f, $x unless $x =~ /\Q$y/i && $y =~ /\Q$x/i;

                # The places where Unicode's lc is different from fc are
                # skipped here by virtue of the 'next unless uc...' line above
                push @f, $x unless lc $x eq fc $x;
            }
        }

	foreach my $x (sort { ord $a <=> ord $b } keys %lower) {
            if (! $is_utf8_locale) {
                my $y = uc $x;
                next unless lc $y eq $x;
                debug_more( "lower=", disp_chars(($x)),
                            "; uc=", disp_chars(($y)), "; ",
                            "; fc=", disp_chars((fc $x)), "; ",
                            disp_chars(($x)), "=~/", disp_chars(($y)), "/i=",
                            $x =~ /\Q$y/i ? 1 : 0,
                            "; ",
                            disp_chars(($y)), "=~/", disp_chars(($x)), "/i=",
                            $y =~ /\Q$x/i ? 1 : 0,
                            "\n");
                if ($x =~ $re || $y =~ $re) { # See above.
                    print "# Regex characters in '$x' or '$y', skipping test $locales_test_number for locale '$Locale'\n";
                    next;
                }
                push @f, $x unless $x =~ /\Q$y/i && $y =~ /\Q$x/i;

                push @f, $x unless lc $x eq fc $x;
            }
            else {
                use locale ':not_characters';
                my $y = uc $x;
                next unless lc $y eq $x;
                debug_more( "lower=", disp_chars(($x)),
                            "; uc=", disp_chars(($y)), "; ",
                            "; fc=", disp_chars((fc $x)), "; ",
                            disp_chars(($x)), "=~/", disp_chars(($y)), "/i=",
                            $x =~ /\Q$y/i ? 1 : 0,
                            "; ",
                            disp_chars(($y)), "=~/", disp_chars(($x)), "/i=",
                            $y =~ /\Q$x/i ? 1 : 0,
                            "\n");
                push @f, $x unless $x =~ /\Q$y/i && $y =~ /\Q$x/i;

                push @f, $x unless lc $x eq fc $x;
            }
	}
	report_multi_result($Locale, $locales_test_number, \@f);
        $problematical_tests{$locales_test_number} = 1;
    }

    # [perl #109318]
    {
        my @f = ();
        ++$locales_test_number;
        $test_names{$locales_test_number} = 'Verify atof with locale radix and negative exponent';
        $problematical_tests{$locales_test_number} = 1;

        my $radix = langinfo(RADIXCHAR);
        my @nums = (
             "3.14e+9",  "3${radix}14e+9",  "3.14e-9",  "3${radix}14e-9",
            "-3.14e+9", "-3${radix}14e+9", "-3.14e-9", "-3${radix}14e-9",
        );

        if (! $is_utf8_locale) {
            use locale;
            for my $num (@nums) {
                push @f, $num
                    unless sprintf("%g", $num) =~ /3.+14/;
            }
        }
        else {
            use locale ':not_characters';
            for my $num (@nums) {
                push @f, $num
                    unless sprintf("%g", $num) =~ /3.+14/;
            }
        }

        report_result($Locale, $locales_test_number, @f == 0);
        if (@f) {
            print "# failed $locales_test_number locale '$Locale' numbers @f\n"
	}
    }
}

my $final_locales_test_number = $locales_test_number;

# Recount the errors.

TEST_NUM:
foreach $test_num ($first_locales_test_number..$final_locales_test_number) {
    my $has_non_global_failure = $Problem{$test_num}
                            || ! defined $Okay{$test_num}
                            || ! @{$Okay{$test_num}};
    print "not " if %setlocale_failed || $has_non_global_failure;
    print "ok $test_num";
    $test_names{$test_num} = "" unless defined $test_names{$test_num};

    # If TODO is in the test name, make it thus
    my $todo = $test_names{$test_num} =~ s/\s*TODO\s*//;
    print " $test_names{$test_num}";
    if ($todo) {
        print " # TODO\n";
    }
    elsif (%setlocale_failed || ! $has_non_global_failure) {
        print "\n";
    }
    elsif ($has_non_global_failure) {

        # If there are any locales that pass this test, or are known-bad, it
        # may be that there are enough passes that we TODO the failure, but
        # only for tests that we have decided can be problematical.
        if (  ($Okay{$test_num} || $Known_bad_locale{$test_num})
            && grep { $_ == $test_num } keys %problematical_tests)
        {
            # Don't count the known-bad failures when calculating the
            # percentage that fail.
            my $known_failures = (exists $Known_bad_locale{$test_num})
                                  ? scalar(keys $Known_bad_locale{$test_num}->%*)
                                  : 0;
            my $adjusted_failures = scalar(keys $Problem{$test_num}->%*)
                                    - $known_failures;

            # Specially handle failures where only known-bad locales fail.
            # This makes the diagnositics clearer.
            if ($adjusted_failures <= 0) {
                print " # TODO fails only on known bad locales: ",
                      join " ", keys $Known_bad_locale{$test_num}->%*, "\n";
                next TEST_NUM;
            }

            # Round to nearest .1%
            my $percent_fail = (int(.5 + (1000 * $adjusted_failures
                                          / scalar(@Locale))))
                               / 10;
            $todo = $percent_fail < $acceptable_failure_percentage;
            print " # TODO" if $todo;
            print "\n";

            if ($debug) {
                print "# $percent_fail% of locales (",
                      scalar(keys $Problem{$test_num}->%*),
                      " of ",
                      scalar(@Locale),
                      ") fail the above test (TODO cut-off is ",
                      $acceptable_failure_percentage,
                      "%)\n";
            }
            elsif ($todo) {
                print "# ", 100 - $percent_fail, "% of locales not known to be problematic on this platform\n";
                print "# pass the above test, so it is likely that the failures\n";
                print "# are errors in the locale definitions.  The test is marked TODO, as the\n";
                print "# problem is not likely to be Perl's\n";
            }
        }

        if ($debug) {
            print "# The code points that had this failure are given above.  Look for lines\n";
            print "# that match 'failed $test_num'\n";
        }
        else {
            print "# For more details, rerun, with environment variable PERL_DEBUG_FULL_TEST=1.\n";
            print "# Then look at that output for lines that match 'failed $test_num'\n";
        }
	if (defined $not_necessarily_a_problem_test_number
            && $test_num == $not_necessarily_a_problem_test_number)
        {
	    print "# The failure of test $not_necessarily_a_problem_test_number is not necessarily fatal.\n";
	    print "# It usually indicates a problem in the environment,\n";
	    print "# not in Perl itself.\n";
	}
    }
}

$test_num = $final_locales_test_number;

if ( ! defined $Config{d_setlocale_accepts_any_locale_name}) {
    # perl #115808
    use warnings;
    my $warned = 0;
    local $SIG{__WARN__} = sub {
        $warned = $_[0] =~ /uninitialized/;
    };
    my $z = "y" . setlocale(&POSIX::LC_ALL, "xyzzy");
    ok($warned, "variable set to setlocale(\"invalid locale name\") is considered uninitialized");
}

# Test that tainting and case changing works on utf8 strings.  These tests are
# placed last to avoid disturbing the hard-coded test numbers that existed at
# the time these were added above this in this file.
# This also tests that locale overrides unicode_strings in the same scope for
# non-utf8 strings.
setlocale(&POSIX::LC_ALL, "C");
{
    use locale;
    use feature 'unicode_strings';

    foreach my $function ("uc", "ucfirst", "lc", "lcfirst", "fc") {
        my @list;   # List of code points to test for $function

        # Used to calculate the changed case for ASCII characters by using the
        # ord, instead of using one of the functions under test.
        my $ascii_case_change_delta;
        my $above_latin1_case_change_delta; # Same for the specific ords > 255
                                            # that we use

        # We test an ASCII character, which should change case;
        # a Latin1 character, which shouldn't change case under this C locale,
        # an above-Latin1 character that when the case is changed would cross
        #   the 255/256 boundary, so doesn't change case
        #   (the \x{149} is one of these, but changes into 2 characters, the
        #   first one of which doesn't cross the boundary.
        # the final one in each list is an above-Latin1 character whose case
        #   does change.  The code below uses its position in its list as a
        #   marker to indicate that it, unlike the other code points above
        #   ASCII, has a successful case change
        #
        # All casing operations under locale (but not :not_characters) should
        # taint
        if ($function =~ /^u/) {
            @list = ("", "a",
                     chr(utf8::unicode_to_native(0xe0)),
                     chr(utf8::unicode_to_native(0xff)),
                     "\x{fb00}", "\x{149}", "\x{101}");
            $ascii_case_change_delta = ($is_ebcdic) ? +64 : -32;
            $above_latin1_case_change_delta = -1;
        }
        else {
            @list = ("", "A",
                     chr(utf8::unicode_to_native(0xC0)),
                     "\x{17F}", "\x{100}");
            $ascii_case_change_delta = ($is_ebcdic) ? -64 : +32;
            $above_latin1_case_change_delta = +1;
        }
        foreach my $is_utf8_locale (0 .. 1) {
            foreach my $j (0 .. $#list) {
                my $char = $list[$j];

                for my $encoded_in_utf8 (0 .. 1) {
                    my $should_be;
                    my $changed;
                    if (! $is_utf8_locale) {
                        no warnings 'locale';
                        $should_be = ($j == $#list)
                            ? chr(ord($char) + $above_latin1_case_change_delta)
                            : (length $char == 0 || utf8::native_to_unicode(ord($char)) > 127)
                              ? $char
                              : chr(ord($char) + $ascii_case_change_delta);

                        # This monstrosity is in order to avoid using an eval,
                        # which might perturb the results
                        $changed = ($function eq "uc")
                                    ? uc($char)
                                    : ($function eq "ucfirst")
                                      ? ucfirst($char)
                                      : ($function eq "lc")
                                        ? lc($char)
                                        : ($function eq "lcfirst")
                                          ? lcfirst($char)
                                          : ($function eq "fc")
                                            ? fc($char)
                                            : die("Unexpected function \"$function\"");
                    }
                    else {
                        {
                            no locale;

                            # For utf8-locales the case changing functions
                            # should work just like they do outside of locale.
                            # Can use eval here because not testing it when
                            # not in locale.
                            $should_be = eval "$function('$char')";
                            die "Unexpected eval error $@ from 'eval \"$function('$char')\"'" if  $@;

                        }
                        use locale ':not_characters';
                        $changed = ($function eq "uc")
                                    ? uc($char)
                                    : ($function eq "ucfirst")
                                      ? ucfirst($char)
                                      : ($function eq "lc")
                                        ? lc($char)
                                        : ($function eq "lcfirst")
                                          ? lcfirst($char)
                                          : ($function eq "fc")
                                            ? fc($char)
                                            : die("Unexpected function \"$function\"");
                    }
                    ok($changed eq $should_be,
                        "$function(\"$char\") in C locale "
                        . (($is_utf8_locale)
                            ? "(use locale ':not_characters'"
                            : "(use locale")
                        . (($encoded_in_utf8)
                            ? "; encoded in utf8)"
                            : "; not encoded in utf8)")
                        . " should be \"$should_be\", got \"$changed\"");

                    # Tainting shouldn't happen for use locale :not_character
                    # (a utf8 locale)
                    (! $is_utf8_locale)
                    ? check_taint($changed)
                    : check_taint_not($changed);

                    # Use UTF-8 next time through the loop
                    utf8::upgrade($char);
                }
            }
        }
    }
}

# Give final advice.

my $didwarn = 0;

foreach ($first_locales_test_number..$final_locales_test_number) {
    if ($Problem{$_}) {
	my @f = sort keys %{ $Problem{$_} };

        # Don't list the failures caused by known-bad locales.
        if (exists $known_bad_locales{$os}) {
            @f = grep { $_ !~ $known_bad_locales{$os} } @f;
            next unless @f;
        }
	my $f = join(" ", @f);
	$f =~ s/(.{50,60}) /$1\n#\t/g;
	print
	    "#\n",
            "# The locale ", (@f == 1 ? "definition" : "definitions"), "\n#\n",
	    "#\t", $f, "\n#\n",
	    "# on your system may have errors because the locale test $_\n",
	    "# \"$test_names{$_}\"\n",
            "# failed in ", (@f == 1 ? "that locale" : "those locales"),
            ".\n";
	print <<EOW;
#
# If your users are not using these locales you are safe for the moment,
# but please report this failure first to perlbug\@perl.org using the
# perlbug script (as described in the INSTALL file) so that the exact
# details of the failures can be sorted out first and then your operating
# system supplier can be alerted about these anomalies.
#
EOW
	$didwarn = 1;
    }
}

# Tell which locales were okay and which were not.

if ($didwarn) {
    my (@s, @F);

    foreach my $l (@Locale) {
	my $p = 0;
        if ($setlocale_failed{$l}) {
            $p++;
        }
        else {
            foreach my $t
                        ($first_locales_test_number..$final_locales_test_number)
            {
                $p++ if $Problem{$t}{$l};
            }
	}
	push @s, $l if $p == 0;
        push @F, $l unless $p == 0;
    }

    if (@s) {
        my $s = join(" ", @s);
        $s =~ s/(.{50,60}) /$1\n#\t/g;

        print
            "# The following locales\n#\n",
            "#\t", $s, "\n#\n",
	    "# tested okay.\n#\n",
    } else {
        print "# None of your locales were fully okay.\n";
    }

    if (@F) {
        my $F = join(" ", @F);
        $F =~ s/(.{50,60}) /$1\n#\t/g;

        my $details = "";
        unless ($debug) {
            $details = "# For more details, rerun, with environment variable PERL_DEBUG_FULL_TEST=1.\n";
        }
        elsif ($debug == 1) {
            $details = "# For even more details, rerun, with environment variable PERL_DEBUG_FULL_TEST=2.\n";
        }

        print
          "# The following locales\n#\n",
          "#\t", $F, "\n#\n",
          "# had problems.\n#\n",
          $details;
    } else {
        print "# None of your locales were broken.\n";
    }
}

if (exists $known_bad_locales{$os} && ! %Known_bad_locale) {
    $test_num++;
    print "ok $test_num $^O no longer has known bad locales # TODO\n";
}

print "1..$test_num\n";

# eof
