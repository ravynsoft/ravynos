
# Time-stamp: "2004-05-07 15:43:11 ADT"

use strict;
use Test;

my @them;
BEGIN { plan('tests' => 63) };
BEGIN { print "# Perl version $] under $^O\n" }

use Pod::Escapes qw(:ALL);
ok 1;

eval " binmode(STDOUT, ':utf8') ";

print "# Pod::Escapes version $Pod::Escapes::VERSION\n";
print "# I'm ", (chr(65) eq 'A') ? '' : 'not ', "in ASCII world.\n";
print "#\n#------------------------\n#\n";

foreach my $quotie (qw( \n \r \cm \cj \t \f \b \a \e )) {
  my $val = eval "\"$quotie\"";
  if($@) {
    ok 0;
    print "# Error in evalling quotie \"$quotie\"\n";
  } elsif(!defined $val) {
    ok 0;
    print "# \"$quotie\" is undef!?\n";
  } else {
    ok 1;
    print "# \"$quotie\" is ", ord($val), "\n";
  }
}

print "#\n#------------------------\n#\n";

print "# 'A' tests...\n";
ok e2char('65'), 'A';
ok e2char('x41'), 'A';
ok e2char('x041'), 'A';
ok e2char('x0041'), 'A';
ok e2char('x00041'), 'A';
ok e2char('0101'), 'A';
ok e2char('00101'), 'A';
ok e2char('000101'), 'A';
ok e2char('0000101'), 'A';

print "# '<' tests...\n";
ok e2char('lt'), '<';
ok e2char('60'), '<';
ok e2char('074'), '<';
ok e2char('0074'), '<';
ok e2char('00074'), '<';
ok e2char('000074'), '<';

ok e2char('x3c'), '<';
ok e2char('x3C'), '<';
ok e2char('x03c'), '<';
ok e2char('x003c'), '<';
ok e2char('x0003c'), '<';
ok e2char('x00003c'), '<';
ok e2char('0x3c'), '<';
ok e2char('0x3C'), '<';
ok e2char('0x03c'), '<';
ok e2char('0x003c'), '<';
ok e2char('0x0003c'), '<';
ok e2char('0x00003c'), '<';

ok e2char('65') ne e2char('lt');

print "# eacute tests...\n";
ok defined e2char('eacute');

print "#    eacute is <", e2char('eacute'), "> which is code ",
      ord(e2char('eacute')), "\n";

ok e2char('eacute'), e2char('233');
ok e2char('eacute'), e2char('0351');
ok e2char('eacute'), e2char('xe9');
ok e2char('eacute'), e2char('xE9');

print "# pi tests...\n";
ok defined e2char('pi');

print "#    pi is <", e2char('pi'), "> which is code ",
      ord(e2char('pi')), "\n";

ok e2char('pi'), e2char('960');
ok e2char('pi'), e2char('01700');
ok e2char('pi'), e2char('001700');
ok e2char('pi'), e2char('0001700');
ok e2char('pi'), e2char('x3c0');
ok e2char('pi'), e2char('x3C0');
ok e2char('pi'), e2char('x03C0');
ok e2char('pi'), e2char('x003C0');
ok e2char('pi'), e2char('x0003C0');


print "# various hash tests...\n";

ok scalar keys %Name2character;
ok defined $Name2character{'eacute'};
ok $Name2character{'lt'} eq '<';

ok scalar keys %Latin1Code_to_fallback;
ok defined $Latin1Code_to_fallback{233};

ok scalar keys %Latin1Char_to_fallback;
ok defined $Latin1Char_to_fallback{chr(233)};

ok scalar keys %Code2USASCII;
ok defined $Code2USASCII{65};
ok $Code2USASCII{65} eq 'A';


