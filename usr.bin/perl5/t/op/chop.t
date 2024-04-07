#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './charset_tools.pl';
}

my $tests_count = 148;
plan tests => $tests_count;

$_ = 'abc';
$c = foo();
is ($c . $_, 'cab', 'optimized');

$_ = 'abc';
$c = chop($_);
is ($c . $_ , 'cab', 'unoptimized');

sub foo {
    chop;
}

@foo = ("hi \n","there\n","!\n");
@bar = @foo;
chop(@bar);
is (join('',@bar), 'hi there!', 'chop list of strings');

$foo = "\n";
chop($foo,@foo);
is (join('',$foo,@foo), 'hi there!', 'chop on list reduces one-character element to an empty string');

$_ = "foo\n\n";
$got = chomp();
is($got, 1, 'check return value when chomp string ending with two newlines; $/ is set to default of one newline');
is ($_, "foo\n", 'chomp string ending with two newlines while $/ is set to one newline' );

$_ = "foo\n";
$got = chomp();
is($got, 1, 'check return value chomp string ending with one newline while $/ is set to a newline');
is ($_, "foo", 'test typical use of chomp; chomp a string ending in a single newline while $/ is set to default of one newline');

$_ = "foo";
$got = chomp();
is($got, 0, 'check return value when chomp a string that does not end with current value of $/, 0 should be returned');
is ($_, "foo", 'chomp a string that does not end with the current value of $/');

$_ = "foo";
$/ = "oo";
$got = chomp();
is ($got, "2", 'check return value when chomp string with $/ consisting of more than one character, and with the ending of the string matching $/');
is ($_, "f", 'chomp a string when $/ consists of two characters that are at the end of the string, check that chomped string contains remnant of original string');

$_ = "bar";
$/ = "oo";
$got = chomp();
is($got, "0", 'check return value when call chomp with $/ consisting of more than one character, and with the ending of the string NOT matching $/');
is ($_, "bar", 'chomp a string when $/ consists of two characters that are NOT at the end of the string');

$_ = "f\n\n\n\n\n";
$/ = "";
$got = chomp();
is ($got, 5, 'check return value when chomp in paragraph mode on string ending with 5 newlines');
is ($_, "f", 'chomp in paragraph mode on string ending with 5 newlines');

$_ = "f\n\n";
$/ = "";
$got = chomp();
is ($got, 2, 'check return value when chomp in paragraph mode on string ending with 2 newlines');
is ($_, "f", 'chomp in paragraph mode on string ending with 2 newlines');

$_ = "f\n";
$/ = "";
$got = chomp();
is ($got, 1, 'check return value when chomp in paragraph mode on string ending with 1 newline');
is ($_, "f", 'chomp in paragraph mode on string ending with 1 newlines');

$_ = "f";
$/ = "";
$got = chomp();
is ($got, 0, 'check return value when chomp in paragraph mode on string ending with no newlines');
is ($_, "f", 'chomp in paragraph mode on string lacking trailing newlines');

$_ = "xx";
$/ = "xx";
$got = chomp();
is ($got, 2, 'check return value when chomp string that consists solely of current value of $/');
is ($_, "", 'chomp on string that consists solely of current value of $/; check that empty string remains');

$_ = "axx";
$/ = "xx";
$got = chomp();
is ($got, 2, 'check return value when chomp string that ends with current value of $/. $/ contains two characters');
is ($_, "a", 'check that when chomp string that ends with currnt value of $/, the part of original string that wasn\'t in $/ remains');

$_ = "axx";
$/ = "yy";
$got = chomp();
is ($got, 0, 'check return value when chomp string that does not end with $/');
is ($_, "axx", 'chomp a string that does not end with $/, the entire string should remain intact');

# This case once mistakenly behaved like paragraph mode.
$_ = "ab\n";
$/ = \3;
$got = chomp();
is ($got, 0, 'check return value when call chomp with $_ = "ab\\n", $/ = \3' );
is ($_, "ab\n", 'chomp with $_ = "ab\\n", $/ = \3' );

# Go Unicode.

$_ = "abc\x{1234}";
chop;
is ($_, "abc", 'Go Unicode');

$_ = "abc\x{1234}d";
chop;
is ($_, "abc\x{1234}");

$_ = "\x{1234}\x{2345}";
chop;
is ($_, "\x{1234}");

my @stuff = qw(this that);
is (chop(@stuff[0,1]), 't');

# bug id 20010305.012 (#5972)
@stuff = qw(ab cd ef);
is (chop(@stuff = @stuff), 'f');

@stuff = qw(ab cd ef);
is (chop(@stuff[0, 2]), 'f');

my %stuff = (1..4);
is (chop(@stuff{1, 3}), '4');

# chomp should not stringify references unless it decides to modify them
$_ = [];
$/ = "\n";
$got = chomp();
ok ($got == 0) or print "# got $got\n";
is (ref($_), "ARRAY", "chomp ref (modify)");

$/ = ")";  # the last char of something like "ARRAY(0x80ff6e4)"
$got = chomp();
ok ($got == 1) or print "# got $got\n";
ok (!ref($_), "chomp ref (no modify)");

$/ = "\n";

%chomp = ("One" => "One", "Two\n" => "Two", "" => "");
%chop = ("One" => "On", "Two\n" => "Two", "" => "");

foreach (keys %chomp) {
  my $key = $_;
  eval {chomp $_};
  if ($@) {
    my $err = $@;
    $err =~ s/\n$//s;
    fail ("\$\@ = \"$err\"");
  } else {
    is ($_, $chomp{$key}, "chomp hash key");
  }
}

foreach (keys %chop) {
  my $key = $_;
  eval {chop $_};
  if ($@) {
    my $err = $@;
    $err =~ s/\n$//s;
    fail ("\$\@ = \"$err\"");
  } else {
    is ($_, $chop{$key}, "chop hash key");
  }
}

# chop and chomp can't be lvalues
eval 'chop($x) = 1;';
ok($@ =~ /Can\'t modify.*chop.*in.*assignment/);
eval 'chomp($x) = 1;';
ok($@ =~ /Can\'t modify.*chom?p.*in.*assignment/);
eval 'chop($x, $y) = (1, 2);';
ok($@ =~ /Can\'t modify.*chop.*in.*assignment/);
eval 'chomp($x, $y) = (1, 2);';
ok($@ =~ /Can\'t modify.*chom?p.*in.*assignment/);

my @chars = ("N",
             uni_to_native("\xd3"),
             substr (uni_to_native("\xd4") . "\x{100}", 0, 1),
             chr 1296);
foreach my $start (@chars) {
  foreach my $end (@chars) {
    local $/ = $end;
    my $message = "start=" . ord ($start) . " end=" . ord $end;
    my $string = $start . $end;
    is (chomp ($string), 1, "$message [returns 1]");
    is ($string, $start, $message);

    my $end_utf8 = $end;
    utf8::encode ($end_utf8);
    next if $end_utf8 eq $end;

    # $end ne $end_utf8, so these should not chomp.
    $string = $start . $end_utf8;
    my $chomped = $string;
    is (chomp ($chomped), 0, "$message (end as bytes) [returns 0]");
    is ($chomped, $string, "$message (end as bytes)");

    $/ = $end_utf8;
    $string = $start . $end;
    $chomped = $string;
    is (chomp ($chomped), 0, "$message (\$/ as bytes) [returns 0]");
    is ($chomped, $string, "$message (\$/ as bytes)");
  }
}

{
    # returns length in characters, but not in bytes.
    $/ = "\x{100}";
    $a = "A$/";
    $b = chomp $a;
    is ($b, 1);

    $/ = "\x{100}\x{101}";
    $a = "A$/";
    $b = chomp $a;
    is ($b, 2);
}

{
    # [perl #36569] chop fails on decoded string with trailing nul
    my $asc = "perl\0";
    my $utf = "perl".pack('U',0); # marked as utf8
    is(chop($asc), "\0", "chopping ascii NUL");
    is(chop($utf), "\0", "chopping utf8 NUL");
    is($asc, "perl", "chopped ascii NUL");
    is($utf, "perl", "chopped utf8 NUL");
}

{
    # Change 26011: Re: A surprising segfault
    # to make sure only that these obfuscated sentences will not crash.

    map chop(+()), ('')x68;
    ok(1, "extend sp in pp_chop");

    map chomp(+()), ('')x68;
    ok(1, "extend sp in pp_chomp");
}

SKIP: {
    # [perl #73246] chop doesn't support utf8
    # the problem was UTF8_IS_START() didn't handle perl's extended UTF8
    # The first code point that failed was 0x80000000, which is now illegal on
    # 32-bit machines.

    use Config;
    ($Config{ivsize} > 4)
        or skip("this build can't handle very large characters", 4);

    # Use chr instead of \x{} so doesn't try to compile these on 32-bit
    # machines, which would crash
    my $utf = chr(0x80000001) . chr(0x80000000);
    my $result = chop($utf);
    is($utf, chr(0x80000001), "chopping high 'unicode'- remnant");
    is($result, chr(0x80000000), "chopping high 'unicode' - result");

    no warnings;
    $utf = chr(0x7fffffffffffffff) . chr(0x7ffffffffffffffe);
    $result = chop($utf);
    is($utf, chr(0x7fffffffffffffff), "chop even higher 'unicode'- remnant");
    is($result, chr(0x7ffffffffffffffe), "chop even higher 'unicode' - result");
}

$/ = "\n";
{
    my $expected = 99999;
    my $input = "UserID\talpha $expected\n";
    my $uid = '';
    chomp(my @line = split (/ |\t/,$input));
    $uid = $line[-1];
    is($uid, $expected,
        "RT #123057: chomp works as expected on split");
}

{
    my $a = local $/ = 7;
    $a = chomp $a;
    is $a, 1, 'lexical $a = chomp $a when $a eq $/ eq 7';
    $a = $/ = 0;
    $a = chomp $a;
    is $a, 1, 'lexical $a = chomp $a when $a eq $/ eq 0';
    my @a = "7";
    for my $b($a[0]) {
        $/ = 7;
        $b = chomp @a;
        is $b, 1,
          'lexical $b = chomp @a when $b eq $/ eq 7 and \$a[0] == \$b';
        $b = $/ = 0;
        $b = chomp @a;
        is $b, 1,
          'lexical $b = chomp @a when $b eq $/ eq 0 and \$a[0] == \$b';
    }
}
