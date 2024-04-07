#!./perl

use strict;
use warnings;
use Text::ParseWords;
use Test::More tests => 27;

my @words = shellwords(qq(foo "bar quiz" zoo));
is($words[0], 'foo');
is($words[1], 'bar quiz');
is($words[2], 'zoo');

{
  # Gonna get some undefined things back
  no warnings 'uninitialized' ;

  # Test quotewords() with other parameters and null last field
  @words = quotewords(':+', 1, 'foo:::"bar:foo":zoo zoo:');
  is(join(";", @words), qq(foo;"bar:foo";zoo zoo;));
}

# Test $keep eq 'delimiters' and last field zero
@words = quotewords('\s+', 'delimiters', '4 3 2 1 0');
is(join(";", @words), qq(4; ;3; ;2; ;1; ;0));

# Big ol' nasty test (thanks, Joerk!)
my $string = 'aaaa"bbbbb" cc\\ cc \\\\\\"dddd" eee\\\\\\"ffff" "gg"';

# First with $keep == 1
my $result = join('|', parse_line('\s+', 1, $string));
is($result, 'aaaa"bbbbb"|cc\\ cc|\\\\\\"dddd" eee\\\\\\"ffff"|"gg"');

# Now, $keep == 0
$result = join('|', parse_line('\s+', 0, $string));
is($result, 'aaaabbbbb|cc cc|\\"dddd eee\\"ffff|gg');

# Now test single quote behavior
$string = 'aaaa"bbbbb" cc\\ cc \\\\\\"dddd\' eee\\\\\\"ffff\' gg';
$result = join('|', parse_line('\s+', 0, $string));
is($result, 'aaaabbbbb|cc cc|\\"dddd eee\\\\\\"ffff|gg');

# Make sure @nested_quotewords does the right thing
my @lists = nested_quotewords('\s+', 0, 'a b c', '1 2 3', 'x y z');
is (@lists, 3);
is (@{$lists[0]}, 3);
is (@{$lists[1]}, 3);
is (@{$lists[2]}, 3);

# Now test error return
$string = 'foo bar baz"bach blech boop';

@words = shellwords($string);
is(@words, 0);

@words = parse_line('s+', 0, $string);
is(@words, 0);

@words = quotewords('s+', 0, $string);
is(@words, 0);

{
  # Gonna get some more undefined things back
  no warnings 'uninitialized' ;

  @words = nested_quotewords('s+', 0, $string);
  is(@words, 0);

  # Now test empty fields
  $result = join('|', parse_line(':', 0, 'foo::0:"":::'));
  is($result, 'foo||0||||');

  # Test for 0 in quotes without $keep
  $result = join('|', parse_line(':', 0, ':"0":'));
  is($result, '|0|');

  # Test for \001 in quoted string
  $result = join('|', parse_line(':', 0, ':"' . "\001" . '":'));
  is($result, "|\1|");

}

# Now test perlish single quote behavior
$Text::ParseWords::PERL_SINGLE_QUOTE = 1;
$string = 'aaaa"bbbbb" cc\ cc \\\\\"dddd\' eee\\\\\"\\\'ffff\' gg';
$result = join('|', parse_line('\s+', 0, $string));
is($result, 'aaaabbbbb|cc cc|\"dddd eee\\\\"\'ffff|gg');

# test whitespace in the delimiters
@words = quotewords(' ', 1, '4 3 2 1 0');
is(join(";", @words), qq(4;3;2;1;0));

# [perl #30442] Text::ParseWords does not handle backslashed newline inside quoted text
$string = qq{"field1"	"field2\\\nstill field2"	"field3"};

$result = join('|', parse_line("\t", 1, $string));
is($result, qq{"field1"|"field2\\\nstill field2"|"field3"});

$result = join('|', parse_line("\t", 0, $string));
is($result, "field1|field2\nstill field2|field3");

SKIP: { # unicode
  skip "No unicode",1 if $]<5.008;
  $string = qq{"field1"\x{1234}"field2\\\x{1234}still field2"\x{1234}"field3"};
  $result = join('|', parse_line("\x{1234}", 0, $string));
  is($result, "field1|field2\x{1234}still field2|field3",'Unicode');
}

# missing quote after matching regex used to hang after change #22997
"1234" =~ /(1)(2)(3)(4)/;
$string = qq{"missing quote};
$result = join('|', shellwords($string));
is($result, "");

# make sure shellwords strips out leading whitespace and trailng undefs
# from parse_line, so it's behavior is more like /bin/sh
$result = join('|', shellwords(" aa \\  \\ bb ", " \\  ", "cc dd ee\\ "));
is($result, "aa| | bb| |cc|dd|ee ");

$SIG{ALRM} = sub {die "Timeout!"};
alarm(3);
@words = Text::ParseWords::old_shellwords("foo\\");
is(@words, 1);
alarm(0);
