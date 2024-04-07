#!./perl -w

$|=1;   # outherwise things get mixed up in output

BEGIN {
	chdir 't' if -d 't';
	require './test.pl';
    set_up_inc( qw '../lib ../ext/re' );
	eval 'require Config'; # assume defaults if this fails
}

skip_all_without_unicode_tables();

use strict;
use open qw(:utf8 :std);

# Show that it works when all warnings are enabled upon invocation.  This file
# includes tests that the default warnings are enabled by default, and the
# non-default ones aren't.
use warnings;
BEGIN { ${^WARNING_BITS} = undef }  # Kludge to restore default warnings

# Kind of a kludge to mark warnings to be expected only if we are testing
# under "use re 'strict'"
my $only_strict_marker = ':expected_only_under_strict';

## If the markers used are changed (search for "MARKER1" in regcomp.c),
## update only these two regexs, and leave the {#} in the @death/@warning
## arrays below. The {#} is a meta-marker -- it marks where the marker should
## go.

sub fixup_expect ($$) {

    # Fixes up the expected results by inserting the boiler plate text.
    # Returns empty string if that is what is expected.  Otherwise, handles
    # either a scalar, turning it into a single element array; or a ref to an
    # array, adjusting each element.  If called in array context, returns an
    # array, otherwise the join of all elements.

    # The string $only_strict_marker will be removed from any expect line it
    # begins, and if $strict is not true, that expect line will be removed
    # from the output (hence won't be expected)

    my ($expect_ref, $strict) = @_;
    return "" if $expect_ref eq "";

    my @expect;
    if (ref $expect_ref) {
        @expect = @$expect_ref;
    }
    else {
        @expect = $expect_ref;
    }

    my @new_expect;
    foreach my $element (@expect) {
        $element =~ s/\{\#\}/in regex; marked by <-- HERE in/;
        $element =~ s/\{\#\}/ <-- HERE /;
        $element .= " at ";
        next if $element =~ s/ ^ $only_strict_marker \s* //x && ! $strict;
        push @new_expect, $element;
    }
    return wantarray ? @new_expect : join "", @new_expect;
}

sub add_markers {
    my ($element)= @_;
    $element =~ s/ at .* line \d+\.?\n$//;
    $element =~ s/in regex; marked by <-- HERE in/{#}/;
    $element =~ s/ <-- HERE /{#}/;
    return $element;
}

## Because we don't "use utf8" in this file, we need to do some extra legwork
## for the utf8 tests: Prepend 'use utf8' to the pattern, and mark the strings
## to check against as UTF-8, but for this all to work properly, the character
## 'ネ' (U+30CD) is required in each pattern somewhere as a marker.
##
## This also creates a second variant of the tests to check if the
## latin1 error messages are working correctly.  Because we don't 'use utf8',
## we can't tell if something is UTF-8 or Latin1, so you need the suffix
## '; no latin1' to not have the second variant.
my $l1   = "\x{ef}";
my $utf8 = "\x{30cd}";
utf8::encode($utf8);

sub mark_as_utf8 {
    my @ret;
    for (my $i = 0; $i < @_; $i += 2) {
        my $pat = $_[$i];
        my $msg = $_[$i+1];
        my $l1_pat = $pat =~ s/$utf8/$l1/gr;
        my $l1_msg;
        $pat = "use utf8; $pat";

        if (ref $msg) {
            $l1_msg = [ map { s/$utf8/$l1/gr } @$msg ];
            @$msg   = map { my $c = $_; utf8::decode($c); $c } @$msg;
        }
        else {
            $l1_msg = $msg =~ s/$utf8/$l1/gr;
            utf8::decode($msg);
        }
        push @ret, $pat => $msg;

        push @ret, $l1_pat => $l1_msg unless $l1_pat =~ /#no latin1/;
    }
    return @ret;
}

my $inf_m1 = ($Config::Config{reg_infty} || ((1<<31)-1)) - 1;
my $inf_p1 = $inf_m1 + 2;

my $B_hex = sprintf("\\x%02X", ord "B");
my $low_mixed_alpha = ('A' lt 'a') ? 'A' : 'a';
my $high_mixed_alpha = ('A' lt 'a') ? 'a' : 'A';
my $low_mixed_digit = ('A' lt '0') ? 'A' : '0';
my $high_mixed_digit = ('A' lt '0') ? '0' : 'A';

my $colon_hex = sprintf "%02X", ord(":");
my $tab_hex = sprintf "%02X", ord("\t");

# Key-value pairs of strings eval'd as patterns => warn/error messages that
# they should generate.  In some cases, the value is an array of multiple
# messages.  Some groups have the message(s) be default on; others, default
# off.  This can be overridden on an individual key basis by preceding the
# pattern string with either 'default_on' or 'default_off'
#
# The first set are those that should be fatal errors.

my $bug133423 = "(?[(?^:(?[\\\x00]))\\]\x00|2[^^]\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80])R.\\670";

my @death =
(
 '/[[=foo=]]/' => 'POSIX syntax [= =] is reserved for future extensions {#} m/[[=foo=]{#}]/',

 '/(?<= .*)/' =>  'Lookbehind longer than 255 not implemented in regex m/(?<= .*)/',

 '/(?<= a+)/' =>  'Lookbehind longer than 255 not implemented in regex m/(?<= a+)/',
 '/(?<= a{255})/' =>  'Lookbehind longer than 255 not implemented in regex m/(?<= a{255})/',
 '/(?<= a{0,255})/' =>  'Lookbehind longer than 255 not implemented in regex m/(?<= a{0,255})/',
 '/(?<= a{200}b{55})/' =>  'Lookbehind longer than 255 not implemented in regex m/(?<= a{200}b{55})/',

 '/(?<= x{1000})/' => 'Lookbehind longer than 255 not implemented in regex m/(?<= x{1000})/',
 '/(?<= (?&x))(?<x>x+)/' => 'Lookbehind longer than 255 not implemented in regex m/(?<= (?&x))(?<x>x+)/',

 '/(?@)/' => 'Sequence (?@...) not implemented {#} m/(?@{#})/',

 '/(?{ 1/' => 'Missing right curly or square bracket',

 '/(?(1x))/' => 'Switch condition not recognized {#} m/(?(1x{#}))/',
 '/(?(1x(?#)))/'=> 'Switch condition not recognized {#} m/(?(1x{#}(?#)))/',

 '/(?(1)/'    => 'Switch (?(condition)... not terminated {#} m/(?(1){#}/',
 '/(?(1)x/'    => 'Switch (?(condition)... not terminated {#} m/(?(1)x{#}/',
 '/(?(1)x|y/'    => 'Switch (?(condition)... not terminated {#} m/(?(1)x|y{#}/',
 '/(?(1)x|y|z)/' => 'Switch (?(condition)... contains too many branches {#} m/(?(1)x|y|{#}z)/',

 '/(?(x)y|x)/' => 'Unknown switch condition (?(...)) {#} m/(?(x{#})y|x)/',
 '/(?(??{}))/' => 'Unknown switch condition (?(...)) {#} m/(?(?{#}?{}))/',
 '/(?(?[]))/' => 'Unknown switch condition (?(...)) {#} m/(?(?{#}[]))/',

 '/(?/' => 'Sequence (? incomplete {#} m/(?{#}/',

 '/(?;x/' => 'Sequence (?;...) not recognized {#} m/(?;{#}x/',
 '/(?<;x/' => 'Group name must start with a non-digit word character {#} m/(?<;{#}x/',
 '/(?\ix/' => 'Sequence (?\...) not recognized {#} m/(?\{#}ix/',
 '/(?\mx/' => 'Sequence (?\...) not recognized {#} m/(?\{#}mx/',
 '/(?\:x/' => 'Sequence (?\...) not recognized {#} m/(?\{#}:x/',
 '/(?\=x/' => 'Sequence (?\...) not recognized {#} m/(?\{#}=x/',
 '/(?\!x/' => 'Sequence (?\...) not recognized {#} m/(?\{#}!x/',
 '/(?\<=x/' => 'Sequence (?\...) not recognized {#} m/(?\{#}<=x/',
 '/(?\<!x/' => 'Sequence (?\...) not recognized {#} m/(?\{#}<!x/',
 '/(?\>x/' => 'Sequence (?\...) not recognized {#} m/(?\{#}>x/',
 '/(?^-i:foo)/' => 'Sequence (?^-...) not recognized {#} m/(?^-{#}i:foo)/',
 '/(?^-i)foo/' => 'Sequence (?^-...) not recognized {#} m/(?^-{#}i)foo/',
 '/(?^d:foo)/' => 'Sequence (?^d...) not recognized {#} m/(?^d{#}:foo)/',
 '/(?^d)foo/' => 'Sequence (?^d...) not recognized {#} m/(?^d{#})foo/',
 '/(?^lu:foo)/' => 'Regexp modifiers "l" and "u" are mutually exclusive {#} m/(?^lu{#}:foo)/',
 '/(?^lu)foo/' => 'Regexp modifiers "l" and "u" are mutually exclusive {#} m/(?^lu{#})foo/',
'/(?da:foo)/' => 'Regexp modifiers "d" and "a" are mutually exclusive {#} m/(?da{#}:foo)/',
'/(?lil:foo)/' => 'Regexp modifier "l" may not appear twice {#} m/(?lil{#}:foo)/',
'/(?aaia:foo)/' => 'Regexp modifier "a" may appear a maximum of twice {#} m/(?aaia{#}:foo)/',
'/(?i-l:foo)/' => 'Regexp modifier "l" may not appear after the "-" {#} m/(?i-l{#}:foo)/',

 '/((x)/' => 'Unmatched ( {#} m/({#}(x)/',
 '/{(}/' => 'Unmatched ( {#} m/{({#}}/',    # [perl #127599]

 "/x{$inf_p1}/" => "Quantifier in {,} bigger than $inf_m1 {#} m/x{$inf_p1\{#}}/",
 "/x{$inf_p1,}/" => "Quantifier in {,} bigger than $inf_m1 {#} m/x{$inf_p1\{#},}/",
 "/x{01,2}/" => "Invalid quantifier in {,} {#} m/x{01{#},2}/",
 "/x{1,02}/" => "Invalid quantifier in {,} {#} m/x{1,02{#}}/",


 '/x**/' => 'Nested quantifiers {#} m/x**{#}/',

 '/x[/' => 'Unmatched [ {#} m/x[{#}/',

 '/*/', => 'Quantifier follows nothing {#} m/*{#}/',

 '/\p{x/' => 'Missing right brace on \p{} {#} m/\p{{#}x/',

 '/[\p{x]/' => 'Missing right brace on \p{} {#} m/[\p{{#}x]/',

 '/(x)\2/' => 'Reference to nonexistent group {#} m/(x)\2{#}/',

 '/\g/' => 'Unterminated \g... pattern {#} m/\g{#}/',
 '/\g{1/' => 'Unterminated \g{...} pattern {#} m/\g{1{#}/',
 '/\g{-abc}/' => 'Group name must start with a non-digit word character {#} m/\g{-{#}abc}/',
 '/(?<;x/' => 'Group name must start with a non-digit word character {#} m/(?<;{#}x/',

 'my $m = "\\\"; $m =~ $m', => 'Trailing \ in regex m/\/',

 '/\x{ 1 /' => 'Missing right brace on \x{} {#} m/\x{ 1{#} /',
 '/\x{X/' => 'Missing right brace on \x{} {#} m/\x{{#}X/',

 '/[\x{X]/' => 'Missing right brace on \x{} {#} m/[\x{{#}X]/',
 '/[\x{ A ]/' => 'Missing right brace on \x{} {#} m/[\x{ A{#} ]/',

 '/\o{ 1 /' => 'Missing right brace on \o{} {#} m/\o{ 1{#} /',
 '/\o{X/'   => 'Missing right brace on \o{} {#} m/\o{{#}X/',

 '/[\o{X]/' => 'Missing right brace on \o{} {#} m/[\o{{#}X]/',
 '/[\o{ 7 ]/' => 'Missing right brace on \o{} {#} m/[\o{ 7{#} ]/',

 '/[[:barf:]]/' => 'POSIX class [:barf:] unknown {#} m/[[:barf:]{#}]/',

 '/[[=barf=]]/' => 'POSIX syntax [= =] is reserved for future extensions {#} m/[[=barf=]{#}]/',

 '/[[.barf.]]/' => 'POSIX syntax [. .] is reserved for future extensions {#} m/[[.barf.]{#}]/',

 '/[z-a]/' => 'Invalid [] range "z-a" {#} m/[z-a{#}]/',

 '/\p/' => 'Empty \p {#} m/\p{#}/',
 '/\P/' => 'Empty \P {#} m/\P{#}/',
 '/\p{}/' => 'Empty \p{} {#} m/\p{{#}}/',
 '/\P{}/' => 'Empty \P{} {#} m/\P{{#}}/',

'/a\b{cde/' => 'Missing right brace on \b{} {#} m/a\b{{#}cde/',
'/a\B{cde/' => 'Missing right brace on \B{} {#} m/a\B{{#}cde/',

 '/\b{}/' => 'Empty \b{} {#} m/\b{}{#}/',
 '/\B{}/' => 'Empty \B{} {#} m/\B{}{#}/',

 '/\b{gc}/' => "'gc' is an unknown bound type {#} m/\\b{gc{#}}/",
 '/\B{gc}/' => "'gc' is an unknown bound type {#} m/\\B{gc{#}}/",

 '/(?[[[::]]])/' => "Unexpected ']' with no following ')' in (?[... {#} m/(?[[[::]]{#}])/",
 '/(?[[[:w:]]])/' => "Unexpected ']' with no following ')' in (?[... {#} m/(?[[[:w:]]{#}])/",
 '/(?[a])/' =>  'Unexpected character {#} m/(?[a{#}])/',
 '/(?[ + \t ])/' => 'Unexpected binary operator \'+\' with no preceding operand {#} m/(?[ +{#} \t ])/',
 '/(?[ \cK - ( + \t ) ])/' => 'Unexpected binary operator \'+\' with no preceding operand {#} m/(?[ \cK - ( +{#} \t ) ])/',
 '/(?[ \cK ( \t ) ])/' => 'Unexpected \'(\' with no preceding operator {#} m/(?[ \cK ({#} \t ) ])/',
 '/(?[ \cK \t ])/' => 'Operand with no preceding operator {#} m/(?[ \cK \t{#} ])/',
 '/(?[ \0004 ])/' => 'Need exactly 3 octal digits {#} m/(?[ \0004 {#}])/',
 '/(?[ \05 ])/' => 'Need exactly 3 octal digits {#} m/(?[ \05 {#}])/',
 '/(?[ \o{1038} ])/' => 'Non-octal character {#} m/(?[ \o{1038{#}} ])/',
 '/(?[ \o{} ])/' => 'Empty \o{} {#} m/(?[ \o{}{#} ])/',
 '/(?[ \x{defg} ])/' => 'Non-hex character {#} m/(?[ \x{defg{#}} ])/',
 '/(?[ \xabcdef ])/' => 'Use \\x{...} for more than two hex characters {#} m/(?[ \xabc{#}def ])/',
 '/(?[ \x{} ])/' => 'Empty \x{} {#} m/(?[ \x{}{#} ])/',
 '/(?[ \cK + ) ])/' => 'Unexpected \')\' {#} m/(?[ \cK + ){#} ])/',
 '/(?[ \cK + ])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[ \cK + {#}])/',
 '/(?[ ( ) ])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[ ( ){#} ])/',
 '/(?[[0]+()+])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[[0]+(){#}+])/',
 '/(?[ \p{foo} ])/' => 'Can\'t find Unicode property definition "foo" {#} m/(?[ \p{foo}{#} ])/',
 '/(?[ \p{ foo = bar } ])/' => 'Can\'t find Unicode property definition "foo = bar" {#} m/(?[ \p{ foo = bar }{#} ])/',
 '/(?[ \8 ])/' => 'Unrecognized escape \8 in character class {#} m/(?[ \8{#} ])/',
 '/(?[ \t ]/' => "Unexpected ']' with no following ')' in (?[... {#} m/(?[ \\t ]{#}/",
 '/(?[ [ \t ]/' => "Syntax error in (?[...]) {#} m/(?[ [ \\t ]{#}/",
 '/(?[ \t ] ]/' => "Unexpected ']' with no following ')' in (?[... {#} m/(?[ \\t ]{#} ]/",
 '/(?[ [ ] ]/' => "Syntax error in (?[...]) {#} m/(?[ [ ] ]{#}/",
 '/(?[ \t + \e # This was supposed to be a comment ])/' =>
    "Syntax error in (?[...]) {#} m/(?[ \\t + \\e # This was supposed to be a comment ]){#}/",
 '/(?[ ])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[ {#}])/',
 'm/(?[[a-\d]])/' => 'False [] range "a-\d" {#} m/(?[[a-\d{#}]])/',
 'm/(?[[\w-x]])/' => 'False [] range "\w-" {#} m/(?[[\w-{#}x]])/',
 'm/(?[[a-\pM]])/' => 'False [] range "a-\pM" {#} m/(?[[a-\pM{#}]])/',
 'm/(?[[\pM-x]])/' => 'False [] range "\pM-" {#} m/(?[[\pM-{#}x]])/',
 'm/(?[[^\N{LATIN CAPITAL LETTER A WITH MACRON AND GRAVE}]])/' => '\N{} here is restricted to one character {#} m/(?[[^\N{U+100.300{#}}]])/',
 'm/(?[ \p{Digit} & (?^(?[ \p{Thai} | \p{Lao} ]))])/' => 'Sequence (?^(...) not recognized {#} m/(?[ \p{Digit} & (?^({#}?[ \p{Thai} | \p{Lao} ]))])/',
 'm/(?[ \p{Digit} & (?(?[ \p{Thai} | \p{Lao} ]))])/' => 'Unexpected character {#} m/(?[ \p{Digit} & (?{#}(?[ \p{Thai} | \p{Lao} ]))])/',
 'm/\p{Is_Is_Any}/' => 'Unknown user-defined property name \p{main::Is_Is_Any}',
 'm/\o{/' => 'Missing right brace on \o{} {#} m/\o{{#}/',
 'm/\o/' => 'Missing braces on \o{} {#} m/\o{#}/',
 'm/\o{}/' => 'Empty \o{} {#} m/\o{}{#}/',
 'm/[\o{]/' => 'Missing right brace on \o{} {#} m/[\o{{#}]/',
 'm/[\o]/' => 'Missing braces on \o{} {#} m/[\o{#}]/',
 'm/[\o{}]/' => 'Empty \o{} {#} m/[\o{}{#}]/',
 'm/(?^-i:foo)/' => 'Sequence (?^-...) not recognized {#} m/(?^-{#}i:foo)/',
 'm/\87/' => 'Reference to nonexistent group {#} m/\87{#}/',
 'm/a\87/' => 'Reference to nonexistent group {#} m/a\87{#}/',
 'm/a\97/' => 'Reference to nonexistent group {#} m/a\97{#}/',
 'm/(*DOOF)/' => 'Unknown verb pattern \'DOOF\' {#} m/(*DOOF){#}/',
 'm/(?&a/'  => 'Sequence (?&... not terminated {#} m/(?&a{#}/',
 'm/(?P=/' => 'Sequence ?P=... not terminated {#} m/(?P={#}/',
 "m/(?'/"  => "Sequence (?'... not terminated {#} m/(?'{#}/",
 "m/(?</"  => "Sequence (?<... not terminated {#} m/(?<{#}/",
 'm/(?&/'  => 'Sequence (?&... not terminated {#} m/(?&{#}/',
 'm/(?(</' => 'Sequence (?(<... not terminated {#} m/(?(<{#}/',
 "m/(?('/" => "Sequence (?('... not terminated {#} m/(?('{#}/",
 'm/\g{/'  => 'Sequence \g{... not terminated {#} m/\g{{#}/',
 'm/\k</'  => 'Sequence \k<... not terminated {#} m/\k<{#}/',
 '/((?# This is a comment in the middle of a token)?:foo)/' => 'In \'(?...)\', the \'(\' and \'?\' must be adjacent {#} m/((?# This is a comment in the middle of a token)?{#}:foo)/',
 '/((?# This is a comment in the middle of a token)*FAIL)/' => 'In \'(*VERB...)\', the \'(\' and \'*\' must be adjacent {#} m/((?# This is a comment in the middle of a token)*{#}FAIL)/',
 '/((?# This is a comment in the middle of a token)*script_run:foo)/' => 'In \'(*...)\', the \'(\' and \'*\' must be adjacent {#} m/((?# This is a comment in the middle of a token)*{#}script_run:foo)/',

 '/(*script_runfoo)/' => 'Unknown \'(*...)\' construct \'script_runfoo\' {#} m/(*script_runfoo){#}/',
 '/(*srfoo)/' => 'Unknown \'(*...)\' construct \'srfoo\' {#} m/(*srfoo){#}/',
 '/(*script_run)/' => '\'(*script_run\' requires a terminating \':\' {#} m/(*script_run{#})/',
 '/(*sr)/' => '\'(*sr\' requires a terminating \':\' {#} m/(*sr{#})/',
 '/(*pla)/' => '\'(*pla\' requires a terminating \':\' {#} m/(*pla{#})/',
 '/(*script_run/' => 'Unterminated \'(*...\' construct {#} m/(*script_run{#}/',
 '/(*sr/' => 'Unterminated \'(*...\' construct {#} m/(*sr{#}/',
 '/(*script_run:foo/' => 'Unterminated \'(*...\' argument {#} m/(*script_run:foo{#}/',
 '/(*sr:foo/' => 'Unterminated \'(*...\' argument {#} m/(*sr:foo{#}/',
 '/(?[\ &!])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[\ &!{#}])/',    # [perl #126180]
 '/(?[\ +!])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[\ +!{#}])/',    # [perl #126180]
 '/(?[\ -!])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[\ -!{#}])/',    # [perl #126180]
 '/(?[\ ^!])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[\ ^!{#}])/',    # [perl #126180]
 '/(?[\ |!])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[\ |!{#}])/',    # [perl #126180]
 '/(?[()-!])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[(){#}-!])/',    # [perl #126204]
 '/(?[!()])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[!(){#}])/',      # [perl #126404]
 '/\w{/' => 'Unescaped left brace in regex is illegal here {#} m/\w{{#}/',
 '/\q{/' => 'Unescaped left brace in regex is illegal here {#} m/\q{{#}/',
 '/\A{/' => 'Unescaped left brace in regex is illegal here {#} m/\A{{#}/',
 '/(?<=/' => 'Sequence (?<=... not terminated {#} m/(?<={#}/',                        # [perl #128170]
 '/(?<!/' => 'Sequence (?<!... not terminated {#} m/(?<!{#}/',
 '/(?!/' => 'Sequence (?!... not terminated {#} m/(?!{#}/',
 '/(?=/' => 'Sequence (?=... not terminated {#} m/(?={#}/',
 '/\p{vertical  tab}/' => 'Can\'t find Unicode property definition "vertical  tab" {#} m/\\p{vertical  tab}{#}/', # [perl #132055]
 "/$bug133423/" => "Unexpected ']' with no following ')' in (?[... {#} m/(?[(?^:(?[\\ '/[^/' => 'Unmatched [ {#} m/[{#}^/', # [perl #133767]
 '/\p{Is_Other_Alphabetic=F}/ ' => 'Can\'t find Unicode property definition "Is_Other_Alphabetic=F" {#} m/\p{Is_Other_Alphabetic=F}{#}/',
 '/\p{Is_Other_Alphabetic=F}/ ' => 'Can\'t find Unicode property definition "Is_Other_Alphabetic=F" {#} m/\p{Is_Other_Alphabetic=F}{#}/',
 '/\x{100}(?(/' => 'Unknown switch condition (?(...)) {#} m/\\x{100}(?({#}/', # [perl #133896]
 '/(?[\N{KEYCAP DIGIT NINE}/' => '\N{} here is restricted to one character {#} m/(?[\\N{U+39.FE0F.20E3{#}}/', # [perl #133988]
 '/0000000000000000[\N{U+0.00}0000/' => 'Unmatched [ {#} m/0000000000000000[{#}\N{U+0.00}0000/', # [perl #134059]
 '/\p{nv=\b5\b}/' => 'Can\'t find Unicode property definition "nv=\\b5\\b" {#} m/\\p{nv=\\b5\\b}{#}/',
 '/\p{nv=:(?g)10:}/' => 'Use of modifier \'g\' is not allowed in Unicode property wildcard subpatterns {#} m/(?g{#})10/',
 '/\p{gc=:L*:}/' => 'Use of quantifier \'*\' is not allowed in Unicode property wildcard subpatterns {#} m/L*{#}/',
 '/\p{gc=:L\G:}/' => 'Use of \'\G\' is not allowed in Unicode property wildcard subpatterns {#} m/L\G{#}/',
 '/\p{gc=:(?a)L:}/' => 'Use of modifier \'a\' is not allowed in Unicode property wildcard subpatterns {#} m/(?a){#}L/',
 '/\p{gc=:(?u)L:}/' => 'Use of modifier \'u\' is not allowed in Unicode property wildcard subpatterns {#} m/(?u){#}L/',
 '/\p{gc=:(?d)L:}/' => 'Use of modifier \'d\' is not allowed in Unicode property wildcard subpatterns {#} m/(?d){#}L/',
 '/\p{gc=:(?l)L:}/' => 'Use of modifier \'l\' is not allowed in Unicode property wildcard subpatterns {#} m/(?l){#}L/',
 '/\p{gc=:(?-m)L:}/' => 'Use of modifier \'-m\' is not allowed in Unicode property wildcard subpatterns {#} m/(?-m{#})L/',
 '/\p{gc=:\pS:}/' => 'Use of \'\\pS\' is not allowed in Unicode property wildcard subpatterns {#} m/\\pS{#}/',
 '/\p{gc=:\PS:}/' => 'Use of \'\\PS\' is not allowed in Unicode property wildcard subpatterns {#} m/\\PS{#}/',
 '/\p{gc=:[\pS]:}/' => 'Use of \'\\pS\' is not allowed in Unicode property wildcard subpatterns {#} m/[\\pS{#}]/',
 '/\p{gc=:[\PS]:}/' => 'Use of \'\\PS\' is not allowed in Unicode property wildcard subpatterns {#} m/[\\PS{#}]/',
 '/(?[\p{name=KATAKANA LETTER AINU P}])/' => 'Unicode string properties are not implemented in (?[...]) {#} m/(?[\p{name=KATAKANA LETTER AINU P}{#}])/',
 '/(?[ (?^x:(?[ ])) ])/' => 'Incomplete expression within \'(?[ ])\' {#} m/(?[ (?^x:(?[ {#}])) ])/',
 '/(?[ (?x:(?[ ])) ])/'  => 'Incomplete expression within \'(?[ ])\' {#} m/(?[ (?x:(?[ {#}])) ])/', # GH #16779
);

# These are messages that are death under 'use re "strict"', and may or may
# not warn otherwise.  See comment before @warning as to why some have a
# \x{100} in them.  This array has 3 elements per construct.  [0] is the regex
# to use; [1] is the message under no strict (empty to not warn), and [2] is
# under strict.
my @death_only_under_strict = (
    'm/\xABC/' => "",
               => 'Use \x{...} for more than two hex characters {#} m/\xABC{#}/',
    'm/[\xABC]/' => "",
                 => 'Use \x{...} for more than two hex characters {#} m/[\xABC{#}]/',

    # some messages below aren't all category 'regexp'.  (Hence we have to
    # turn off 'digit' messages as well below)
    'm/\xAG/' => 'Non-hex character \'G\' terminates \x early.  Resolved as "\x0AG" {#} m/\xA{#}G/',
              => 'Non-hex character {#} m/\xAG{#}/',
    'm/[\xAG]/' => 'Non-hex character \'G\' terminates \x early.  Resolved as "\x0AG" {#} m/[\xA{#}G]/',
                => 'Non-hex character {#} m/[\xAG{#}]/',
    'm/\o{789}/' => 'Non-octal character \'8\' terminates \o early.  Resolved as "\o{007}" {#} m/\o{789}{#}/',
                 => 'Non-octal character {#} m/\o{78{#}9}/',
    'm/[\o{789}]/' => 'Non-octal character \'8\' terminates \o early.  Resolved as "\o{007}" {#} m/[\o{789}{#}]/',
                   => 'Non-octal character {#} m/[\o{78{#}9}]/',
    'm/\x{}/' => "",
              => 'Empty \x{} {#} m/\x{}{#}/',
    'm/[\x{}]/' => "",
                => 'Empty \x{} {#} m/[\x{}{#}]/',
    'm/\x{ABCDEFG}/' => 'Non-hex character \'G\' terminates \x early.  Resolved as "\x{ABCDEF}" {#} m/\x{ABCDEFG}{#}/',
                     => 'Non-hex character {#} m/\x{ABCDEFG{#}}/',
    'm/[\x{ABCDEFG}]/' => 'Non-hex character \'G\' terminates \x early.  Resolved as "\x{ABCDEF}" {#} m/[\x{ABCDEFG}{#}]/',
                       => 'Non-hex character {#} m/[\x{ABCDEFG{#}}]/',
    "m'[\\y]\\x{100}'" => 'Unrecognized escape \y in character class passed through {#} m/[\y{#}]\x{100}/',
                       => 'Unrecognized escape \y in character class {#} m/[\y{#}]\x{100}/',
    'm/[a-\d]\x{100}/' => 'False [] range "a-\d" {#} m/[a-\d{#}]\x{100}/',
                       => 'False [] range "a-\d" {#} m/[a-\d{#}]\x{100}/',
    'm/[\w-x]\x{100}/' => 'False [] range "\w-" {#} m/[\w-{#}x]\x{100}/',
                       => 'False [] range "\w-" {#} m/[\w-{#}x]\x{100}/',
    'm/[a-\pM]\x{100}/' => 'False [] range "a-\pM" {#} m/[a-\pM{#}]\x{100}/',
                        => 'False [] range "a-\pM" {#} m/[a-\pM{#}]\x{100}/',
    'm/[\pM-x]\x{100}/' => 'False [] range "\pM-" {#} m/[\pM-{#}x]\x{100}/',
                        => 'False [] range "\pM-" {#} m/[\pM-{#}x]\x{100}/',
    'm/[^\N{LATIN CAPITAL LETTER A WITH MACRON AND GRAVE}]/' => 'Using just the first character returned by \N{} in character class {#} m/[^\N{U+100.300}{#}]/',
                                       => '\N{} here is restricted to one character {#} m/[^\N{U+100.300{#}}]/',
    'm/[\x03-\N{LATIN CAPITAL LETTER A WITH MACRON AND GRAVE}]/' => 'Using just the first character returned by \N{} in character class {#} m/[\x03-\N{U+100.300}{#}]/',
                                            => '\N{} here is restricted to one character {#} m/[\x03-\N{U+100.300{#}}]/',
    'm/[\N{LATIN CAPITAL LETTER A WITH MACRON AND GRAVE}-\x{10FFFF}]/' => 'Using just the first character returned by \N{} in character class {#} m/[\N{U+100.300}{#}-\x{10FFFF}]/',
                                                  => '\N{} here is restricted to one character {#} m/[\N{U+100.300{#}}-\x{10FFFF}]/',
    '/[\08]/'   => 'Non-octal character \'8\' terminates \0 early.  Resolved as "\0008" {#} m/[\08{#}]/',
                => 'Need exactly 3 octal digits {#} m/[\08{#}]/',
    '/[\018]/'  => 'Non-octal character \'8\' terminates \0 early.  Resolved as "\0018" {#} m/[\018{#}]/',
                => 'Need exactly 3 octal digits {#} m/[\018{#}]/',
    '/[\_\0]/'  => "",
                => 'Need exactly 3 octal digits {#} m/[\_\0]{#}/',
    '/[\07]/'   => "",
                => 'Need exactly 3 octal digits {#} m/[\07]{#}/',
    '/[\0005]/' => "",
                => 'Need exactly 3 octal digits {#} m/[\0005]{#}/',
    '/[\8\9]\x{100}/' => ['Unrecognized escape \8 in character class passed through {#} m/[\8{#}\9]\x{100}/',
                          'Unrecognized escape \9 in character class passed through {#} m/[\8\9{#}]\x{100}/',
                         ],
                      => 'Unrecognized escape \8 in character class {#} m/[\8{#}\9]\x{100}/',
    '/[a-\d]\x{100}/' => 'False [] range "a-\d" {#} m/[a-\d{#}]\x{100}/',
                      => 'False [] range "a-\d" {#} m/[a-\d{#}]\x{100}/',
    '/[\d-b]\x{100}/' => 'False [] range "\d-" {#} m/[\d-{#}b]\x{100}/',
                      => 'False [] range "\d-" {#} m/[\d-{#}b]\x{100}/',
    '/[\s-\d]\x{100}/' => 'False [] range "\s-" {#} m/[\s-{#}\d]\x{100}/',
                       => 'False [] range "\s-" {#} m/[\s-{#}\d]\x{100}/',
    '/[\d-\s]\x{100}/' => 'False [] range "\d-" {#} m/[\d-{#}\s]\x{100}/',
                       => 'False [] range "\d-" {#} m/[\d-{#}\s]\x{100}/',
    '/[a-[:digit:]]\x{100}/' => 'False [] range "a-[:digit:]" {#} m/[a-[:digit:]{#}]\x{100}/',
                             => 'False [] range "a-[:digit:]" {#} m/[a-[:digit:]{#}]\x{100}/',
    '/[[:digit:]-b]\x{100}/' => 'False [] range "[:digit:]-" {#} m/[[:digit:]-{#}b]\x{100}/',
                             => 'False [] range "[:digit:]-" {#} m/[[:digit:]-{#}b]\x{100}/',
    '/[[:alpha:]-[:digit:]]\x{100}/' => 'False [] range "[:alpha:]-" {#} m/[[:alpha:]-{#}[:digit:]]\x{100}/',
                                     => 'False [] range "[:alpha:]-" {#} m/[[:alpha:]-{#}[:digit:]]\x{100}/',
    '/[[:digit:]-[:alpha:]]\x{100}/' => 'False [] range "[:digit:]-" {#} m/[[:digit:]-{#}[:alpha:]]\x{100}/',
                                     => 'False [] range "[:digit:]-" {#} m/[[:digit:]-{#}[:alpha:]]\x{100}/',
    '/[a\zb]\x{100}/' => 'Unrecognized escape \z in character class passed through {#} m/[a\z{#}b]\x{100}/',
                      => 'Unrecognized escape \z in character class {#} m/[a\z{#}b]\x{100}/',
    '/[ab]/'          => "",
                        => 'Literal vertical space in [] is illegal except under /x {#} m/[a{#}b]/',
    '/:{4,a}/'     => 'Unescaped left brace in regex is passed through {#} m/:{{#}4,a}/',
                   => 'Unescaped left brace in regex is illegal here {#} m/:{{#}4,a}/',
    '/xa{3\,4}y/'  => 'Unescaped left brace in regex is passed through {#} m/xa{{#}3\,4}y/',
                   => 'Unescaped left brace in regex is illegal here {#} m/xa{{#}3\,4}y/',
    '/\\${[^\\}]*}/' => 'Unescaped left brace in regex is passed through {#} m/\\${{#}[^\\}]*}/',
                     => 'Unescaped left brace in regex is illegal here {#} m/\\${{#}[^\\}]*}/',
    '/.{/'         => 'Unescaped left brace in regex is passed through {#} m/.{{#}/',
                   => 'Unescaped left brace in regex is illegal here {#} m/.{{#}/',
    '/[x]{/'       => 'Unescaped left brace in regex is passed through {#} m/[x]{{#}/',
                   => 'Unescaped left brace in regex is illegal here {#} m/[x]{{#}/',
    '/\p{Latin}{/' => 'Unescaped left brace in regex is passed through {#} m/\p{Latin}{{#}/',
                   => 'Unescaped left brace in regex is illegal here {#} m/\p{Latin}{{#}/',
    '/\x{100}\x/'  => "",
                   => "Empty \\x {#} m/\\x{100}\\x{#}/",
    '/\o{ 1 20 }/' => 'Non-octal character \' \' terminates \o early.  Resolved as "\o{001}" {#} m/\o{ 1 20 }{#}/',
                   => 'Non-octal character {#} m/\\o{ 1 {#}20 }/',
    '/\x{ 5 0 }/'  => 'Non-hex character \' \' terminates \x early.  Resolved as "\x{05}" {#} m/\x{ 5 0 }{#}/',
                   => 'Non-hex character {#} m/\\x{ 5 {#}0 }/',
);

# These need the character 'ネ' as a marker for mark_as_utf8()
my @death_utf8 = mark_as_utf8(
 '/ネ(?<= .*)/' =>  'Lookbehind longer than 255 not implemented in regex m/ネ(?<= .*)/',

 '/(?<= ネ{1000})/' => 'Lookbehind longer than 255 not implemented in regex m/(?<= ネ{1000})/',

 '/ネ(?ネ)ネ/' => 'Sequence (?ネ...) not recognized {#} m/ネ(?ネ{#})ネ/',

 '/ネ(?(1ネ))ネ/' => 'Switch condition not recognized {#} m/ネ(?(1ネ{#}))ネ/',

 '/(?(1)ネ|y|ヌ)/' => 'Switch (?(condition)... contains too many branches {#} m/(?(1)ネ|y|{#}ヌ)/',

 '/(?(ネ)y|ネ)/' => 'Unknown switch condition (?(...)) {#} m/(?(ネ{#})y|ネ)/',

 '/ネ(?/' => 'Sequence (? incomplete {#} m/ネ(?{#}/',

 '/ネ(?;ネ/' => 'Sequence (?;...) not recognized {#} m/ネ(?;{#}ネ/',
 '/ネ(?<;ネ/' => 'Group name must start with a non-digit word character {#} m/ネ(?<;{#}ネ/',
 '/ネ(?\ixネ/' => 'Sequence (?\...) not recognized {#} m/ネ(?\{#}ixネ/',
 '/ネ(?^lu:ネ)/' => 'Regexp modifiers "l" and "u" are mutually exclusive {#} m/ネ(?^lu{#}:ネ)/',
'/ネ(?lil:ネ)/' => 'Regexp modifier "l" may not appear twice {#} m/ネ(?lil{#}:ネ)/',
'/ネ(?aaia:ネ)/' => 'Regexp modifier "a" may appear a maximum of twice {#} m/ネ(?aaia{#}:ネ)/',
'/ネ(?i-l:ネ)/' => 'Regexp modifier "l" may not appear after the "-" {#} m/ネ(?i-l{#}:ネ)/',

 '/ネ((ネ)/' => 'Unmatched ( {#} m/ネ({#}(ネ)/',

 "/ネ{$inf_p1}ネ/" => "Quantifier in {,} bigger than $inf_m1 {#} m/ネ{$inf_p1\{#}}ネ/",
 "/ネ{$inf_p1,}ネ/" => "Quantifier in {,} bigger than $inf_m1 {#} m/ネ{$inf_p1\{#},}ネ/",
 "/ネ{01}ネ/" => "Invalid quantifier in {,} {#} m/ネ{01{#}}ネ/",
 "/ネ{1,02}ネ/" => "Invalid quantifier in {,} {#} m/ネ{1,02{#}}ネ/",


 '/ネ**ネ/' => 'Nested quantifiers {#} m/ネ**{#}ネ/',

 '/ネ[ネ/' => 'Unmatched [ {#} m/ネ[{#}ネ/',

 '/*ネ/', => 'Quantifier follows nothing {#} m/*{#}ネ/',

 '/ネ\p{ネ/' => 'Missing right brace on \p{} {#} m/ネ\p{{#}ネ/',

 '/(ネ)\2ネ/' => 'Reference to nonexistent group {#} m/(ネ)\2{#}ネ/',

 '/\g{ネ/; #no latin1' => 'Sequence \g{... not terminated {#} m/\g{ネ{#}/',

 'my $m = "ネ\\\"; $m =~ $m', => 'Trailing \ in regex m/ネ\/',

 '/\x{ネ/' => 'Missing right brace on \x{} {#} m/\x{{#}ネ/',
 '/ネ[\x{ネ]ネ/' => 'Missing right brace on \x{} {#} m/ネ[\x{{#}ネ]ネ/',
 '/ネ[\x{ネ]/' => 'Missing right brace on \x{} {#} m/ネ[\x{{#}ネ]/',

 '/ネ\o{ネ/' => 'Missing right brace on \o{} {#} m/ネ\o{{#}ネ/',

 '/[ネ-a]ネ/' => 'Invalid [] range "ネ-a" {#} m/[ネ-a{#}]ネ/',

 '/ネ\p{}ネ/' => 'Empty \p{} {#} m/ネ\p{{#}}ネ/',

 '/ネ(?[[[:ネ]]])ネ/' => "Unexpected ']' with no following ')' in (?[... {#} m/ネ(?[[[:ネ]]{#}])ネ/",
 '/ネ(?[[[:ネ: ])ネ/' => "Syntax error in (?[...]) {#} m/ネ(?[[[:ネ: ])ネ{#}/",
 '/ネ(?[[[::]]])ネ/' => "Unexpected ']' with no following ')' in (?[... {#} m/ネ(?[[[::]]{#}])ネ/",
 '/ネ(?[[[:ネ:]]])ネ/' => "Unexpected ']' with no following ')' in (?[... {#} m/ネ(?[[[:ネ:]]{#}])ネ/",
 '/ネ(?[ネ])ネ/' =>  'Unexpected character {#} m/ネ(?[ネ{#}])ネ/',
 '/ネ(?[ + [ネ] ])/' => 'Unexpected binary operator \'+\' with no preceding operand {#} m/ネ(?[ +{#} [ネ] ])/',
 '/ネ(?[ \cK - ( + [ネ] ) ])/' => 'Unexpected binary operator \'+\' with no preceding operand {#} m/ネ(?[ \cK - ( +{#} [ネ] ) ])/',
 '/ネ(?[ \cK ( [ネ] ) ])/' => 'Unexpected \'(\' with no preceding operator {#} m/ネ(?[ \cK ({#} [ネ] ) ])/',
 '/ネ(?[ \cK [ネ] ])ネ/' => 'Operand with no preceding operator {#} m/ネ(?[ \cK [ネ{#}] ])ネ/',
 '/ネ(?[ \0004 ])ネ/' => 'Need exactly 3 octal digits {#} m/ネ(?[ \0004 {#}])ネ/',
 '/(?[ \o{ネ} ])ネ/' => 'Non-octal character {#} m/(?[ \o{ネ{#}} ])ネ/',
 '/ネ(?[ \o{} ])ネ/' => 'Empty \o{} {#} m/ネ(?[ \o{}{#} ])ネ/',
 '/(?[ \x{ネ} ])ネ/' => 'Non-hex character {#} m/(?[ \x{ネ{#}} ])ネ/',
 '/(?[ \p{ネ} ])/' => 'Can\'t find Unicode property definition "ネ" {#} m/(?[ \p{ネ}{#} ])/',
 '/(?[ \p{ ネ = bar } ])/' => 'Can\'t find Unicode property definition "ネ = bar" {#} m/(?[ \p{ ネ = bar }{#} ])/',
 '/ネ(?[ \t ]/' => "Unexpected ']' with no following ')' in (?[... {#} m/ネ(?[ \\t ]{#}/",
 '/(?[ \t + \e # ネ This was supposed to be a comment ])/' =>
    "Syntax error in (?[...]) {#} m/(?[ \\t + \\e # ネ This was supposed to be a comment ]){#}/",
 'm/(*ネ)ネ/' => q<Unknown '(*...)' construct 'ネ' {#} m/(*ネ){#}ネ/>,
 '/\cネ/' => "Character following \"\\c\" must be printable ASCII {#} m/\\cネ{#}/",
 '/[\cネ]/' => "Character following \"\\c\" must be printable ASCII {#} m/[\\cネ{#}]/",
 '/\b{ネ}/' => "'ネ' is an unknown bound type {#} m/\\b{ネ{#}}/",
 '/\B{ネ}/' => "'ネ' is an unknown bound type {#} m/\\B{ネ{#}}/",
);
push @death, @death_utf8;

my @death_utf8_only_under_strict = (
    "m'ネ[\\y]ネ'" => 'Unrecognized escape \y in character class passed through {#} m/ネ[\y{#}]ネ/',
                   => 'Unrecognized escape \y in character class {#} m/ネ[\y{#}]ネ/',
    'm/ネ[ネ-\d]ネ/' => 'False [] range "ネ-\d" {#} m/ネ[ネ-\d{#}]ネ/',
                     => 'False [] range "ネ-\d" {#} m/ネ[ネ-\d{#}]ネ/',
    'm/ネ[\w-ネ]ネ/' => 'False [] range "\w-" {#} m/ネ[\w-{#}ネ]ネ/',
                     => 'False [] range "\w-" {#} m/ネ[\w-{#}ネ]ネ/',
    'm/ネ[ネ-\pM]ネ/' => 'False [] range "ネ-\pM" {#} m/ネ[ネ-\pM{#}]ネ/',
                      => 'False [] range "ネ-\pM" {#} m/ネ[ネ-\pM{#}]ネ/',
    '/ネ[ネ-[:digit:]]ネ/' => 'False [] range "ネ-[:digit:]" {#} m/ネ[ネ-[:digit:]{#}]ネ/',
                           => 'False [] range "ネ-[:digit:]" {#} m/ネ[ネ-[:digit:]{#}]ネ/',
    '/ネ[\d-\s]ネ/' => 'False [] range "\d-" {#} m/ネ[\d-{#}\s]ネ/',
                    => 'False [] range "\d-" {#} m/ネ[\d-{#}\s]ネ/',
    '/ネ[a\zb]ネ/' => 'Unrecognized escape \z in character class passed through {#} m/ネ[a\z{#}b]ネ/',
                   => 'Unrecognized escape \z in character class {#} m/ネ[a\z{#}b]ネ/',
);
# Tests involving a user-defined charnames translator are in pat_advanced.t

# In the following arrays of warnings, the value can be an array of things to
# expect.  If the empty string, it means no warning should be raised.


# Key-value pairs of code/error of code that should have non-fatal regexp
# warnings.  Most currently have \x{100} appended to them to force them to be
# upgraded to UTF-8, and the first pass restarted.  Previously this would
# cause some warnings to be output twice.  This tests that that behavior has
# been fixed.

my @warning = (
    'm/\b*\x{100}/' => '\b* matches null string many times {#} m/\b*{#}\x{100}/',
    '/\b{g}/a' => "Using /u for '\\b{g}' instead of /a {#} m/\\b{g}{#}/",
    '/\B{gcb}/a' => "Using /u for '\\B{gcb}' instead of /a {#} m/\\B{gcb}{#}/",
    'm/[:blank:]\x{100}/' => 'POSIX syntax [: :] belongs inside character classes {#} m/[:blank:]{#}\x{100}/',
    'm/[[:cntrl:]][:^ascii:]\x{100}/' =>  'POSIX syntax [: :] belongs inside character classes {#} m/[[:cntrl:]][:^ascii:]{#}\x{100}/',
    'm/[[:ascii]]\x{100}/' => "Assuming NOT a POSIX class since there is no terminating ':' {#} m/[[:ascii{#}]]\\x{100}/",
    'm/(?[[:word]])\x{100}/' => "Assuming NOT a POSIX class since there is no terminating ':' {#} m/(?[[:word{#}]])\\x{100}/",
    "m'\\y\\x{100}'"     => 'Unrecognized escape \y passed through {#} m/\y{#}\x{100}/',
    '/x{3,1}/'   => 'Quantifier {n,m} with n > m can\'t match {#} m/x{3,1}{#}/',
    '/\08/' => 'Non-octal character \'8\' terminates \0 early.  Resolved as "\0008" {#} m/\08{#}/',

    '/\018/' => 'Non-octal character \'8\' terminates \0 early.  Resolved as "\0018" {#} m/\018{#}/',
    '/(?=a)*/' => '(?=a)* matches null string many times {#} m/(?=a)*{#}/',
    'my $x = \'\m\'; qr/a$x/' => 'Unrecognized escape \m passed through {#} m/a\m{#}/',
    '/\q/' => 'Unrecognized escape \q passed through {#} m/\q{#}/',
    '/\q\p{Any}/' => 'Unrecognized escape \q passed through {#} m/\q{#}\p{Any}/',

    # These two tests do not include the marker, because regcomp.c no
    # longer knows where it goes by the time this warning is emitted.
    # See [perl #122680] regcomp warning gives wrong position of
    # problem.
    '/(?=a){1,3}\x{100}/' => 'Quantifier unexpected on zero-length expression in regex m/(?=a){1,3}\x{100}/',
    '/(a|b)(?=a){3}\x{100}/' => 'Quantifier unexpected on zero-length expression in regex m/(a|b)(?=a){3}\x{100}/',

    '/\_/' => "",
    '/[\006]/' => "",
    '/[:alpha:]\x{100}/' => 'POSIX syntax [: :] belongs inside character classes {#} m/[:alpha:]{#}\x{100}/',
    '/[:zog:]\x{100}/' => 'POSIX syntax [: :] belongs inside character classes (but this one isn\'t fully valid) {#} m/[:zog:]{#}\x{100}/',
    '/[.zog.]\x{100}/' => 'POSIX syntax [. .] belongs inside character classes (but this one isn\'t implemented) {#} m/[.zog.]{#}\x{100}/',
    '/[a-b]/' => "",
    '/(?c)\x{100}/' => 'Useless (?c) - use /gc modifier {#} m/(?c{#})\x{100}/',
    '/(?-c)\x{100}/' => 'Useless (?-c) - don\'t use /gc modifier {#} m/(?-c{#})\x{100}/',
    '/(?g)\x{100}/' => 'Useless (?g) - use /g modifier {#} m/(?g{#})\x{100}/',
    '/(?-g)\x{100}/' => 'Useless (?-g) - don\'t use /g modifier {#} m/(?-g{#})\x{100}/',
    '/(?o)\x{100}/' => 'Useless (?o) - use /o modifier {#} m/(?o{#})\x{100}/',
    '/(?-o)\x{100}/' => 'Useless (?-o) - don\'t use /o modifier {#} m/(?-o{#})\x{100}/',
    '/(?g-o)\x{100}/' => [ 'Useless (?g) - use /g modifier {#} m/(?g{#}-o)\x{100}/',
                    'Useless (?-o) - don\'t use /o modifier {#} m/(?g-o{#})\x{100}/',
                  ],
    '/(?g-c)\x{100}/' => [ 'Useless (?g) - use /g modifier {#} m/(?g{#}-c)\x{100}/',
                    'Useless (?-c) - don\'t use /gc modifier {#} m/(?g-c{#})\x{100}/',
                  ],
      # (?c) means (?g) error won't be thrown
     '/(?o-cg)\x{100}/' => [ 'Useless (?o) - use /o modifier {#} m/(?o{#}-cg)\x{100}/',
                      'Useless (?-c) - don\'t use /gc modifier {#} m/(?o-c{#}g)\x{100}/',
                    ],
    '/(?ogc)\x{100}/' => [ 'Useless (?o) - use /o modifier {#} m/(?o{#}gc)\x{100}/',
                    'Useless (?g) - use /g modifier {#} m/(?og{#}c)\x{100}/',
                    'Useless (?c) - use /gc modifier {#} m/(?ogc{#})\x{100}/',
                  ],
    '/a{1,1}?\x{100}/' => 'Useless use of greediness modifier \'?\' {#} m/a{1,1}?{#}\x{100}/',
    "/(?[ [ % - % ] ])/" => "",
    "/(?[ [ : - \\x$colon_hex ] ])\\x{100}/" => "\": - \\x$colon_hex \" is more clearly written simply as \":\" {#} m/(?[ [ : - \\x$colon_hex {#}] ])\\x{100}/",
    "/(?[ [ \\x$colon_hex - : ] ])\\x{100}/" => "\"\\x$colon_hex\ - : \" is more clearly written simply as \":\" {#} m/(?[ [ \\x$colon_hex - : {#}] ])\\x{100}/",
    "/(?[ [ \\t - \\x$tab_hex ] ])\\x{100}/" => "\"\\t - \\x$tab_hex \" is more clearly written simply as \"\\t\" {#} m/(?[ [ \\t - \\x$tab_hex {#}] ])\\x{100}/",
    "/(?[ [ \\x$tab_hex - \\t ] ])\\x{100}/" => "\"\\x$tab_hex\ - \\t \" is more clearly written simply as \"\\t\" {#} m/(?[ [ \\x$tab_hex - \\t {#}] ])\\x{100}/",
    "/(?[ [ $B_hex - C ] ])/" => "Ranges of ASCII printables should be some subset of \"0-9\", \"A-Z\", or \"a-z\" {#} m/(?[ [ $B_hex - C {#}] ])/",
    "/(?[ [ A - $B_hex ] ])/" => "Ranges of ASCII printables should be some subset of \"0-9\", \"A-Z\", or \"a-z\" {#} m/(?[ [ A - $B_hex {#}] ])/",
    "/(?[ [ $low_mixed_alpha - $high_mixed_alpha ] ])/" => "Ranges of ASCII printables should be some subset of \"0-9\", \"A-Z\", or \"a-z\" {#} m/(?[ [ $low_mixed_alpha - $high_mixed_alpha {#}] ])/",
    "/(?[ [ $low_mixed_digit - $high_mixed_digit ] ])/" => "Ranges of ASCII printables should be some subset of \"0-9\", \"A-Z\", or \"a-z\" {#} m/(?[ [ $low_mixed_digit - $high_mixed_digit {#}] ])/",
    "/[alnum]/" => "",
    "/[^alnum]/" => "",
    '/[:blank]\x{100}/' => 'POSIX syntax [: :] belongs inside character classes (but this one isn\'t fully valid) {#} m/[:blank{#}]\x{100}/',
    '/[[:digit]]\x{100}/' => 'Assuming NOT a POSIX class since there is no terminating \':\' {#} m/[[:digit{#}]]\x{100}/', # [perl # 8904]
    '/[[:digit:foo]\x{100}/' => 'Assuming NOT a POSIX class since there is no terminating \']\' {#} m/[[:digit:{#}foo]\x{100}/',
    '/[[:di#it:foo]\x{100}/x' => 'Assuming NOT a POSIX class since there is no terminating \']\' {#} m/[[:di#it:{#}foo]\x{100}/',
    '/[[:dgit]]\x{100}/' => 'Assuming NOT a POSIX class since there is no terminating \':\' {#} m/[[:dgit{#}]]\x{100}/',
    '/[[:dgit:foo]\x{100}/' => 'Assuming NOT a POSIX class since there is no terminating \']\' {#} m/[[:dgit:{#}foo]\x{100}/',
    '/[[:dgt]]\x{100}/' => "",      # Far enough away from a real class to not be recognized as one
    '/[[:dgt:foo]\x{100}/' => "",
    '/[[:DIGIT]]\x{100}/' => [ 'Assuming NOT a POSIX class since the name must be all lowercase letters {#} m/[[:DIGIT{#}]]\x{100}/',
                               'Assuming NOT a POSIX class since there is no terminating \':\' {#} m/[[:DIGIT{#}]]\x{100}/',
                           ],
    '/[[digit]\x{100}/' => [ 'Assuming NOT a POSIX class since there must be a starting \':\' {#} m/[[{#}digit]\x{100}/',
                             'Assuming NOT a POSIX class since there is no terminating \':\' {#} m/[[digit{#}]\x{100}/',
                           ],
    '/[[alpha]]\x{100}/' => [ 'Assuming NOT a POSIX class since there must be a starting \':\' {#} m/[[{#}alpha]]\x{100}/',
                              'Assuming NOT a POSIX class since there is no terminating \':\' {#} m/[[alpha{#}]]\x{100}/',
                           ],
    '/[[^word]\x{100}/' => [ 'Assuming NOT a POSIX class since the \'^\' must come after the colon {#} m/[[^{#}word]\x{100}/',
                              'Assuming NOT a POSIX class since there must be a starting \':\' {#} m/[[^{#}word]\x{100}/',
                              'Assuming NOT a POSIX class since there is no terminating \':\' {#} m/[[^word{#}]\x{100}/',
                            ],
    '/[[   ^   :   x d i g i t   :   ]   ]\x{100}/' => [ 'Assuming NOT a POSIX class since no blanks are allowed in one {#} m/[[   {#}^   :   x d i g i t   :   ]   ]\x{100}/',
                                               'Assuming NOT a POSIX class since the \'^\' must come after the colon {#} m/[[   ^{#}   :   x d i g i t   :   ]   ]\x{100}/',
                                               'Assuming NOT a POSIX class since no blanks are allowed in one {#} m/[[   ^   {#}:   x d i g i t   :   ]   ]\x{100}/',
                                               'Assuming NOT a POSIX class since no blanks are allowed in one {#} m/[[   ^   :   {#}x d i g i t   :   ]   ]\x{100}/',
                                               'Assuming NOT a POSIX class since no blanks are allowed in one {#} m/[[   ^   :   x d i g i t   :   ]{#}   ]\x{100}/',
                                               $only_strict_marker . 'Unescaped literal \']\' {#} m/[[   ^   :   x d i g i t   :   ]   ]{#}\x{100}/',
                            ],
    '/[foo:lower:]]\x{100}/' => 'Assuming NOT a POSIX class since it doesn\'t start with a \'[\' {#} m/[foo{#}:lower:]]\x{100}/',
    '/[[;upper;]]\x{100}/' => [ 'Assuming NOT a POSIX class since a semi-colon was found instead of a colon {#} m/[[;{#}upper;]]\x{100}/',
                                'Assuming NOT a POSIX class since a semi-colon was found instead of a colon {#} m/[[;upper;]{#}]\x{100}/',
                              ],
    '/[foo;punct;]]\x{100}/' => [ 'Assuming NOT a POSIX class since it doesn\'t start with a \'[\' {#} m/[foo{#};punct;]]\x{100}/',
                                  'Assuming NOT a POSIX class since a semi-colon was found instead of a colon {#} m/[foo;{#}punct;]]\x{100}/',
                                  'Assuming NOT a POSIX class since a semi-colon was found instead of a colon {#} m/[foo;punct;]{#}]\x{100}/',
                                ],
   '/[][[:alpha:]]/' => "",        # [perl #127581]
   '/[][[:alpha:]\\@\\\\^_?]/' => "", # [perl #131522]
    '/(?[[:w:]])/' => "",
    '/([.].*)[.]/'   => "",    # [perl #127582]
    '/[.].*[.]/'     => "",    # [perl #127604]
    '/abc/xix' => "",
    '/(?xmsixp:abc)/' => "",
    '/(?xmsixp)abc/' => "",
    '/(?xxxx:abc)/' => "",

); # See comments before this for why '\x{100}' is generally needed

# These need the character 'ネ' as a marker for mark_as_utf8()
my @warnings_utf8 = mark_as_utf8(
    'm/ネ\b*ネ/' => '\b* matches null string many times {#} m/ネ\b*{#}ネ/',
    '/(?=ネ)*/' => '(?=ネ)* matches null string many times {#} m/(?=ネ)*{#}/',
    'm/ネ[:foo:]ネ/' => 'POSIX syntax [: :] belongs inside character classes (but this one isn\'t fully valid) {#} m/ネ[:foo:]{#}ネ/',
    '/ネ(?c)ネ/' => 'Useless (?c) - use /gc modifier {#} m/ネ(?c{#})ネ/',
    '/utf8 ネ (?ogc) ネ/' => [
        'Useless (?o) - use /o modifier {#} m/utf8 ネ (?o{#}gc) ネ/',
        'Useless (?g) - use /g modifier {#} m/utf8 ネ (?og{#}c) ネ/',
        'Useless (?c) - use /gc modifier {#} m/utf8 ネ (?ogc{#}) ネ/',
    ],
   '/ネ[[:ネ:]]ネ/' => "",
   '/ネ(?[[:ネ:]])ネ/' => "",

);

push @warning, @warnings_utf8;

my @warning_only_under_strict = (
    '/[\N{U+00}-\x01]\x{100}/' => 'Both or neither range ends should be Unicode {#} m/[\N{U+00}-\x01{#}]\x{100}/',
    '/[\x00-\N{SOH}]\x{100}/' => 'Both or neither range ends should be Unicode {#} m/[\x00-\N{U+01}{#}]\x{100}/',
    '/[\N{DEL}-\o{377}]\x{100}/' => 'Both or neither range ends should be Unicode {#} m/[\N{U+7F}-\o{377}{#}]\x{100}/',
    '/[\o{0}-\N{U+01}]\x{100}/' => 'Both or neither range ends should be Unicode {#} m/[\o{0}-\N{U+01}{#}]\x{100}/',
    '/[\000-\N{U+01}]\x{100}/' => 'Both or neither range ends should be Unicode {#} m/[\000-\N{U+01}{#}]\x{100}/',
    '/[\N{DEL}-\377]\x{100}/' => 'Both or neither range ends should be Unicode {#} m/[\N{U+7F}-\377{#}]\x{100}/',
    '/[\N{U+00}-A]\x{100}/' => 'Ranges of ASCII printables should be some subset of "0-9", "A-Z", or "a-z" {#} m/[\N{U+00}-A{#}]\x{100}/',
    '/[a-\N{U+FF}]\x{100}/' => 'Ranges of ASCII printables should be some subset of "0-9", "A-Z", or "a-z" {#} m/[a-\N{U+FF}{#}]\x{100}/',
    '/[\N{U+00}-\a]\x{100}/' => "",
    '/[\a-\N{U+FF}]\x{100}/' => "",
    '/[\N{U+100}-\x{101}]/' => "",
    "/[%-%]/" => "",
    "/[:-\\x$colon_hex]\\x{100}/" => "\":-\\x$colon_hex\" is more clearly written simply as \":\" {#} m/[:-\\x$colon_hex\{#}]\\x{100}/",
    "/[\\x$colon_hex-:]\\x{100}/" => "\"\\x$colon_hex-:\" is more clearly written simply as \":\" {#} m/[\\x$colon_hex\-:{#}]\\x{100}/",
    "/[\\t-\\x$tab_hex]\\x{100}/" => "\"\\t-\\x$tab_hex\" is more clearly written simply as \"\\t\" {#} m/[\\t-\\x$tab_hex\{#}]\\x{100}/",
    "/[\\x$tab_hex-\\t]\\x{100}/" => "\"\\x$tab_hex-\\t\" is more clearly written simply as \"\\t\" {#} m/[\\x$tab_hex\-\\t{#}]\\x{100}/",
    "/[$B_hex-C]/" => "Ranges of ASCII printables should be some subset of \"0-9\", \"A-Z\", or \"a-z\" {#} m/[$B_hex-C{#}]/",
    "/[A-$B_hex]/" => "Ranges of ASCII printables should be some subset of \"0-9\", \"A-Z\", or \"a-z\" {#} m/[A-$B_hex\{#}]/",
    "/[$low_mixed_alpha-$high_mixed_alpha]/" => "Ranges of ASCII printables should be some subset of \"0-9\", \"A-Z\", or \"a-z\" {#} m/[$low_mixed_alpha-$high_mixed_alpha\{#}]/",
    "/[$low_mixed_digit-$high_mixed_digit]/" => "Ranges of ASCII printables should be some subset of \"0-9\", \"A-Z\", or \"a-z\" {#} m/[$low_mixed_digit-$high_mixed_digit\{#}]/",
    '/\b<GCB}/' => 'Unescaped literal \'}\' {#} m/\b<GCB}{#}/',
    '/[ ]def]/' => 'Unescaped literal \']\' {#} m/[ ]def]{#}/',
    '/(?)/' => 'Empty (?) without any modifiers {#} m/(?){#}/', # [perl #132851]
);

my @warning_utf8_only_under_strict = mark_as_utf8(
 '/ネ[᪉-᪐]/; #no latin1' => "Ranges of digits should be from the same group of 10 {#} m/ネ[᪉-᪐{#}]/",
 '/ネ(?[ [ ᪉ - ᪐ ] ])/; #no latin1' => "Ranges of digits should be from the same group of 10 {#} m/ネ(?[ [ ᪉ - ᪐ {#}] ])/",
 '/ネ[᧙-᧚]/; #no latin1' => "Ranges of digits should be from the same group of 10 {#} m/ネ[᧙-᧚{#}]/",
 '/ネ(?[ [ ᧙ - ᧚ ] ])/; #no latin1' => "Ranges of digits should be from the same group of 10 {#} m/ネ(?[ [ ᧙ - ᧚ {#}] ])/",
 '/ネ(?[ [ 𝟘 - 𝟡 ] ])/; #no latin1' => "",
 '/ネ(?[ [ 𝟧 - 𝟱 ] ])/; #no latin1' => "Ranges of digits should be from the same group of 10 {#} m/ネ(?[ [ 𝟧 - 𝟱 {#}] ])/",
 '/ネ(?[ [ 𝟧 - 𝟰 ] ])/; #no latin1' => "Ranges of digits should be from the same group of 10 {#} m/ネ(?[ [ 𝟧 - 𝟰 {#}] ])/",
);

push @warning_only_under_strict, @warning_utf8_only_under_strict;

my @experimental_vlb = (
    '/(?<=(p|qq|rrr))/' => 'Variable length positive lookbehind with capturing' .
                           ' is experimental {#} m/(?<=(p|qq|rrr)){#}/',
    '/(?<!(p|qq|rrr))/' => 'Variable length negative lookbehind with capturing' .
                           ' is experimental {#} m/(?<!(p|qq|rrr)){#}/',
    '/(?| (?=(foo)) | (?<=(foo)|p) )/'
            => 'Variable length positive lookbehind with capturing' .
               ' is experimental {#} m/(?| (?=(foo)) | (?<=(foo)|p) ){#}/',
    '/(?| (?=(foo)) | (?<=(foo)|p) )/x'
            => 'Variable length positive lookbehind with capturing' .
               ' is experimental {#} m/(?| (?=(foo)) | (?<=(foo)|p) ){#}/',
    '/(?| (?=(foo)) | (?<!(foo)|p) )/'
            => 'Variable length negative lookbehind with capturing' .
               ' is experimental {#} m/(?| (?=(foo)) | (?<!(foo)|p) ){#}/',
    '/(?| (?=(foo)) | (?<!(foo)|p) )/x'
            => 'Variable length negative lookbehind with capturing' .
               ' is experimental {#} m/(?| (?=(foo)) | (?<!(foo)|p) ){#}/',
    '/(?<!(foo|bop(*ACCEPT)|bar)baz)/'
            => 'Variable length negative lookbehind with capturing' .
               ' is experimental {#} m/(?<!(foo|bop(*ACCEPT)|bar)baz){#}/',
    '/(?<=(foo|bop(*ACCEPT)|bar)baz)/'
            => 'Variable length positive lookbehind with capturing' .
               ' is experimental {#} m/(?<=(foo|bop(*ACCEPT)|bar)baz){#}/',
);

my @wildcard = (
    'm!(?[\p{name=/KATAKANA/}])$!' =>
    [
     'The Unicode property wildcards feature is experimental',
     'Using just the single character results returned by \p{} in (?[...]) {#} m/(?[\p{name=/KATAKANA/}{#}])$/'
    ], # [GH #17732] Null pointer deref
);

my @deprecated = (
 '/^{/'          => "",
 '/foo|{/'       => "",
 '/foo|^{/'      => "",
 '/foo(:?{bar)/' => "",
 '/\s*{/'        => "",
 '/a{3,4}{/'     => "",
);

for my $strict ("", "use re 'strict';") {

    # First time just use @death; but under strict we add the things that fail
    # there.  Doing it this way makes sure that 'strict' doesnt change the
    # things that are already fatal when not under strict.
    if ($strict) {
        for (my $i = 0; $i < @death_only_under_strict; $i += 3) {
            push @death, $death_only_under_strict[$i],    # The regex
                         $death_only_under_strict[$i+2];  # The fatal msg
        }
        for (my $i = 0; $i < @death_utf8_only_under_strict; $i += 3) {

            # Same with the utf8 versions
            push @death, mark_as_utf8($death_utf8_only_under_strict[$i],
                                      $death_utf8_only_under_strict[$i+2]);
        }
    }
    for (my $i = 0; $i < @death; $i += 2) {
        my $regex = $death[$i] =~ s/ default_ (on | off) //rx;
        my $expect = fixup_expect($death[$i+1], $strict);
        if ($expect eq "") {
            fail("$0: Internal error: '$death[$i]' should have an error message");
        }
        else {
            no warnings 'experimental::re_strict';
            no warnings 'experimental::uniprop_wildcards';

            warning_is(sub {
                    my $meaning_of_life;
                    my $eval_string = "$strict $regex";
                    $_ = "x";
                    eval "$eval_string; \$meaning_of_life = 42";
                    ok (! defined $meaning_of_life, "$eval_string died");
                    my $error= $@;
                    if ($error =~ qr/\Q$expect/) {
                        ok(1, "... and gave expected message");
                    } else {
                        ok(0,$eval_string);
                        diag("Have: " . _q(add_markers($error)));
                        diag("Want: " . _q($death[$i+1]));
                    }
                }, undef, "... and no other warnings");
        }
    }
}

for my $strict ("",  "no warnings 'experimental::re_strict'; use re 'strict';") {
    my @warning_tests = @warning;

    # Build the tests for @warning.  Use the strict/non-strict versions
    # appropriately.
    if ($strict) {
        push @warning_tests, @warning_only_under_strict;
    }
    else {
        for (my $i = 0; $i < @warning_only_under_strict; $i += 2) {

            # (?[ ]) are always under strict
            if ($warning_only_under_strict[$i] =~ /\Q(?[/) {
                push @warning_tests, $warning_only_under_strict[$i],  # The regex
                                    $warning_only_under_strict[$i+1];
            }
            else {
                push @warning_tests, $warning_only_under_strict[$i],  # The regex
                                    "";    # No warning because not strict
            }
        }
        for (my $i = 0; $i < @death_only_under_strict; $i += 3) {
            push @warning_tests, $death_only_under_strict[$i],    # The regex
                                 $death_only_under_strict[$i+1];  # The warning
        }
        for (my $i = 0; $i < @death_utf8_only_under_strict; $i += 3) {
            push @warning_tests, mark_as_utf8($death_utf8_only_under_strict[$i],
                                        $death_utf8_only_under_strict[$i+1]);
        }
    }

    foreach my $ref (
        \@warning_tests,
        \@wildcard,
        \@deprecated,
        \@experimental_vlb,
    ){
        my $warning_type;
        my $default_on;
        if ($ref == \@warning_tests) {
            $warning_type = 'regexp, digit';
            $default_on = $strict;
        }
        elsif ($ref == \@deprecated) {
            $warning_type = 'regexp, deprecated';
            $default_on = 1;
        }
        elsif ($ref == \@wildcard) {
            $warning_type = 'experimental::uniprop_wildcards';
            $default_on = 1;
        }
        elsif ($ref == \@experimental_vlb) {
            $warning_type = 'experimental::vlb';
            $default_on = 1;
        }
        else {
            fail("$0: Internal error: Unexpected loop variable");
        }

        for (my $i = 0; $i < @$ref; $i += 2) {
            my $this_default_on = $default_on;
            my $regex = $ref->[$i];
            if ($regex =~ s/ default_ (on | off) //x) {
                $this_default_on = $1 eq 'on';
            }
            my @expect = fixup_expect($ref->[$i+1], $strict);

            # A length-1 array with an empty warning means no warning gets
            # generated at all.
            undef @expect if @expect == 1 && $expect[0] eq "";

            {
                $_ = "x";
                #use feature 'unicode_eval';
                #print STDERR __LINE__, ": ", "eval '$strict no warnings; $regex'", "\n";
                eval "$strict no warnings; $regex";
            }
            if (is($@, "", "$strict $regex did not die")) {
                my @got = capture_warnings(sub {
                                        $_ = "x";
                                        eval "$strict $regex" });
                my $count = @expect;
                if (! is(scalar @got, scalar @expect,
                            "... and gave expected number ($count) of warnings"))
                {
                    if (@got < @expect) {
                        $count = @got;
                        note "Expected warnings not gotten:\n\t" . join "\n\t",
                                                    @expect[$count .. $#expect];
                    }
                    else {
                        note "Unexpected warnings gotten:\n\t" . join("\n\t",
                                                         @got[$count .. $#got]);
                    }
                }
                foreach my $i (0 .. $count - 1) {
                    if (! like($got[$i], qr/\Q$expect[$i]/,
                                               "... and gave expected warning"))
                    {
                        chomp($got[$i]);
                        chomp($expect[$i]);
                        diag("GOT\n'$got[$i]'\nEXPECT\n'$expect[$i]'");
                    }
                    else {
                            # Turning off this type of warning should make the
                            # count go down by at least 1.
                        ok ($count - 1 >= capture_warnings(sub {
                            $_ = "x";
                            eval "$strict no warnings '$warning_type'; $regex;" }
                           ),
                           "... and turning off '$warning_type' warnings suppressed it");

                        # Test that whether the warning is on by default is
                        # correct.  This test relies on the fact that we
                        # are outside the scope of any ‘use warnings’.
                        local $^W;
                        my @warns = capture_warnings(sub {
                                            $_ = "x";
                                            eval "$strict $regex"
                                            });
                        # Warning should be on as well if is testing
                        # '(?[...])' which turns on strict
                        if (   $this_default_on
                            || (    grep { $_ =~ /\Q(?[/ } @expect
                                and $ref != \@warning_tests))
                        {
                           ok @warns > 0, "... and the warning is on by default";
                        }
                        elsif (! (ok @warns == 0,
                                     "... and the warning is off by default"))
                        {
                               diag("GOT\n" . join "\n", @warns);
                        }
                    }
                }
            }
        }
    }
}

done_testing();
