#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";
    eval 'use Errno';
    die $@ if $@ and !is_miniperl();
}

my @bad_salts =
   (
    [ '',   'zero-length' ],
    [ 'a',  'length 1' ],
    [ '!a', 'bad first character' ],
    [ 'a!', 'bad second character' ],
    [ '@a', 'fencepost before A' ],
    [ '[a', 'fencepost after Z' ],
    [ '`a', 'fencepost before a' ],
    [ '{a', 'fencepost after z' ],
    [ '-a', 'fencepost before .' ],
    [ ':a', 'fencepost after 9' ],
   );

my @good_salts = qw(aa zz AA ZZ .. 99);

plan tests => 2 * @bad_salts + 1 + @good_salts;

for my $bad_salt (@bad_salts) {
    my ($salt, $what) = @$bad_salt;
    $! = 0;
    is(crypt("abc", $salt), undef, "bad salt ($what)");
    is(0+$!, &Errno::EINVAL, "check errno ($what)");
}

is(crypt("abcdef", "ab"), "abDMWw5NL.afs", "sanity check result");

# just to check we're not rejecting any good salts
for my $good_salt (@good_salts) {
    isnt(crypt("abcdef", $good_salt), undef, "good salt $good_salt");
}
