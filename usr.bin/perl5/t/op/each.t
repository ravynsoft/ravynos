#!./perl

use strict;
use warnings;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    require './charset_tools.pl';
    set_up_inc('../lib');
}

my %h;
$h{'abc'} = 'ABC';
$h{'def'} = 'DEF';
$h{'jkl','mno'} = "JKL\034MNO";
$h{'a',2,3,4,5} = join("\034",'A',2,3,4,5);
$h{'a'} = 'A';
$h{'b'} = 'B';
$h{'c'} = 'C';
$h{'d'} = 'D';
$h{'e'} = 'E';
$h{'f'} = 'F';
$h{'g'} = 'G';
$h{'h'} = 'H';
$h{'i'} = 'I';
$h{'j'} = 'J';
$h{'k'} = 'K';
$h{'l'} = 'L';
$h{'m'} = 'M';
$h{'n'} = 'N';
$h{'o'} = 'O';
$h{'p'} = 'P';
$h{'q'} = 'Q';
$h{'r'} = 'R';
$h{'s'} = 'S';
$h{'t'} = 'T';
$h{'u'} = 'U';
$h{'v'} = 'V';
$h{'w'} = 'W';
$h{'x'} = 'X';
$h{'y'} = 'Y';
$h{'z'} = 'Z';

my @keys = keys %h;
my @values = values %h;

is ($#keys, 29, "keys");
is ($#values, 29, "values");

my $i = 0;		# stop -w complaints

while (my ($key,$value) = each(%h)) {
    if ($key eq $keys[$i] && $value eq $values[$i]
        && (('a' lt 'A' && $key lt $value) || $key gt $value)) {
	$key =~ y/a-z/A-Z/;
	$i++ if $key eq $value;
    }
}

is ($i, 30, "each count");

@keys = ('blurfl', keys(%h), 'dyick');
is ($#keys, 31, "added a key");

SKIP: {
    skip "no Hash::Util on miniperl", 4, if is_miniperl;
    require Hash::Util;
    sub Hash::Util::num_buckets (\%);

    my $size = Hash::Util::num_buckets(%h);
    keys %h = $size * 5;
    my $newsize = Hash::Util::num_buckets(%h);
    is ($newsize, $size * 8, "resize");
    keys %h = 1;
    $size = Hash::Util::num_buckets(%h);
    is ($size, $newsize, "same size");
    %h = (1,1);
    $size = Hash::Util::num_buckets(%h);
    is ($size, $newsize, "still same size");
    undef %h;
    %h = (1,1);
    $size = Hash::Util::num_buckets(%h);
    is ($size, 8, "size 8");
}

# test scalar each
my %hash = 1..20;
my $total = 0;
my $key;
$total += $key while $key = each %hash;
is ($total, 100, "test scalar each");

for (1..3) { my @foo = each %hash }
keys %hash;
$total = 0;
$total += $key while $key = each %hash;
is ($total, 100, "test scalar keys resets iterator");

for (1..3) { my @foo = each %hash }
$total = 0;
$total += $key while $key = each %hash;
isnt ($total, 100, "test iterator of each is being maintained");

for (1..3) { my @foo = each %hash }
values %hash;
$total = 0;
$total += $key while $key = each %hash;
is ($total, 100, "test values keys resets iterator");

is (keys(%hash), 10, "keys (%hash)");
SKIP: {
    skip "no Hash::Util on miniperl", 8, if is_miniperl;
    require Hash::Util;
    sub Hash::Util::num_buckets (\%);

    my $size = Hash::Util::num_buckets(%hash);
    cmp_ok($size, '>=', keys %hash, 'sanity check - more buckets than keys');
    %hash = ();
    is(Hash::Util::num_buckets(%hash), $size,
       "size doesn't change when hash is emptied");

    %hash = split /, /, 'Pugh, Pugh, Barney McGrew, Cuthbert, Dibble, Grubb';
    is (keys(%hash), 3, "now 3 keys");
    # 3 keys won't be enough to trigger any "must grow" criteria:
    is(Hash::Util::num_buckets(%hash), $size,
       "size doesn't change with 3 keys");

    keys(%hash) = keys(%hash);
    is (Hash::Util::num_buckets(%hash), $size,
	"assign to keys does not shrink hash bucket array");
    is (keys(%hash), 3, "still 3 keys");
    keys(%hash) = $size + 100;
    cmp_ok(Hash::Util::num_buckets(%hash), '>', $size,
           "assign to keys will grow hash bucket array");
    is (keys(%hash), 3, "but still 3 keys");
}

@::tests = (&next_test, &next_test, &next_test);
{
    package Obj;
    sub DESTROY { print "ok $::tests[1] # DESTROY called\n"; }
    {
	my $h = { A => bless [], __PACKAGE__ };
        while (my($k,$v) = each %$h) {
	    print "ok $::tests[0]\n" if $k eq 'A' and ref($v) eq 'Obj';
	}
    }
    print "ok $::tests[2]\n";
}

# Check for Unicode hash keys.
my %u = ("\x{12}", "f", "\x{123}", "fo", "\x{1234}",  "foo");
$u{"\x{12345}"}  = "bar";
@u{"\x{10FFFD}"} = "zap";

my %u2;
foreach (keys %u) {
    is (length(), 1, "Check length of " . _qq $_);
    $u2{$_} = $u{$_};
}
ok (eq_hash(\%u, \%u2), "copied unicode hash keys correctly?");

my $a = byte_utf8a_to_utf8n("\xe3\x81\x82"); my $A = "\x{3042}";
my %b = ( $a => "non-utf8");
%u = ( $A => "utf8");

is (exists $b{$A}, '', "utf8 key in bytes hash");
is (exists $u{$a}, '', "bytes key in utf8 hash");
print "# $b{$_}\n" for keys %b; # Used to core dump before change #8056.
pass ("if we got here change 8056 worked");
print "# $u{$_}\n" for keys %u; # Used to core dump before change #8056.
pass ("change 8056 is thanks to Inaba Hiroto");

{
    my %u;
    my $u0 = pack("U0U", 0x00B6);
    my $b0 = byte_utf8a_to_utf8n("\xC2\xB6"); # 0xC2 0xB6 is U+00B6 in UTF-8
    my $u1 = pack("U0U", 0x0100);
    my $b1 = byte_utf8a_to_utf8n("\xC4\x80"); # 0xC4 0x80 is U+0100 in UTF-8

    $u{$u0} = 1;
    $u{$b0} = 2; 
    $u{$u1} = 3;
    $u{$b1} = 4;

    is(scalar keys %u, 4, "four different Unicode keys"); 
    is($u{$u0}, 1, "U+00B6        -> 1");
    is($u{$b0}, 2, "U+00C2 U+00B6 -> 2");
    is($u{$u1}, 3, "U+0100        -> 3 ");
    is($u{$b1}, 4, "U+00C4 U+0080 -> 4");
}

# test for syntax errors
for my $k (qw(each keys values)) {
    eval $k;
    like($@, qr/^Not enough arguments for $k/, "$k demands argument");
}

{
    my %foo=(1..10);
    my ($k,$v);
    my $count=keys %foo;
    my ($k1,$v1)=each(%foo);
    my $yes = 0;
    if (%foo) { $yes++ }
    my ($k2,$v2)=each(%foo);
    my $rest=0;
    while (each(%foo)) {$rest++};
    is($yes,1,"if(%foo) was true - my");
    isnt($k1,$k2,"if(%foo) didnt mess with each (key) - my");
    isnt($v1,$v2,"if(%foo) didnt mess with each (value) - my");
    is($rest,3,"Got the expected number of keys - my");
    my $hsv=1 && %foo;
    is($hsv,$count,"Got the count of keys from %foo in scalar assignment context - my");
    my @arr=%foo&&%foo;
    is(@arr,10,"Got expected number of elements in list context - my");
}    
{
    our %foo=(1..10);
    my ($k,$v);
    my $count=keys %foo;
    my ($k1,$v1)=each(%foo);
    my $yes = 0;
    if (%foo) { $yes++ }
    my ($k2,$v2)=each(%foo);
    my $rest=0;
    while (each(%foo)) {$rest++};
    is($yes,1,"if(%foo) was true - our");
    isnt($k1,$k2,"if(%foo) didnt mess with each (key) - our");
    isnt($v1,$v2,"if(%foo) didnt mess with each (value) - our");
    is($rest,3,"Got the expected number of keys - our");
    my $hsv=1 && %foo;
    is($hsv,$count,"Got the count of keys from %foo in scalar assignment context - our");
    my @arr=%foo&&%foo;
    is(@arr,10,"Got expected number of elements in list context - our");
}    
{
    # make sure a deleted active iterator gets freed timely, even if the
    # hash is otherwise empty

    package Single;

    my $c = 0;
    sub DESTROY { $c++ };

    {
	my %h = ("a" => bless []);
	my ($k,$v) = each %h;
	delete $h{$k};
	::is($c, 0, "single key not yet freed");
    }
    ::is($c, 1, "single key now freed");
}

{
    # Make sure each() does not leave the iterator in an inconsistent state
    # (RITER set to >= 0, with EITER null) if the active iterator is
    # deleted, leaving the hash apparently empty.
    my %h;
    $h{1} = 2;
    each %h;
    delete $h{1};
    each %h;
    $h{1}=2;
    is join ("-", each %h), '1-2',
	'each on apparently empty hash does not leave RITER set';
}
SKIP:{
    my $code= <<'TEST_CODE';
    my $warned= 0;
    local $SIG{__WARN__}= sub {
        /\QUse of each() on hash after insertion without resetting hash iterator results in undefined behavior\E/
            and $warned=1 for @_;
    };
    my %h= map { $_ => $_ } "A".."F";
    while (my ($k, $v)= each %h) {
        $h{"$k$k"}= $v;
    }
    print "a:$warned,";
    no warnings 'internal';
    $warned= 0;
    %h= map { $_ => $_ } "A".."F";
    while (my ($k, $v)= each %h) {
        $h{"$k$k"}= $v;
    }
    print "b:$warned\n";
TEST_CODE
    local $ENV{PERL_HASH_SEED};
    local $ENV{PERL_PERTURB_KEYS};
    fresh_perl_like($code,
            qr/\Aa:1,b:0\z/,
            undef,
            'Hash iterator reset warnings fires when expected');
}
{
    # Test that the call to hv_iternext_flags() that calls prime_env_iter()
    # produces the results consistent with subsequent iterations of %ENV
    my $raw = run_perl(switches => ['-l'],
                       prog => 'for (1,2) { @a = keys %ENV; print scalar @a; print for @a }');
    my @lines = split /\n/, $raw;
    my $count1 = shift @lines;
    my @got1 = splice @lines, 0, $count1;
    my $count2 = shift @lines;
    is($count1, $count2, 'both iterations of %ENV returned the same count of keys');
    is(scalar @lines, $count2, 'second iteration of %ENV printed all keys');
    is(join("\n", sort @got1), join("\n", sort @lines), 'both iterations of %ENV returned identical keys');
}

fresh_perl_like('$a = keys %ENV; $b = () = keys %ENV; $c = keys %ENV; print qq=$a,$b,$c=',
                qr/^([1-9][0-9]*),\1,\1$/,
                undef,
                'keys %ENV in scalar context triggers prime_env_iter if needed');
fresh_perl_like('$a = $ENV{PATH}; $a = $ENV{q=DCL$PATH=}; $a = keys %ENV; $b = () = keys %ENV; $c = keys %ENV; print qq=$a,$b,$c=',
                qr/^([1-9][0-9]*),\1,\1$/,
                undef,
                '%ENV lookup, and keys %ENV in scalar context remain consistent');

use feature 'refaliasing';
no warnings 'experimental::refaliasing';
$a = 7;
my %h2;
\$h2{f} = \$a;
($a, $b) = (each %h2);
is "$a $b", "f 7", 'each in list assignment';
$a = 7;
($a, $b) = (3, values %h2);
is "$a $b", "3 7", 'values in list assignment';

done_testing();
