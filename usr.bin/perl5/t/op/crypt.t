#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc( qw(. ../lib) );
    use Config;
}

if ( !$Config{d_crypt} ) {
    skip_all("crypt unimplemented");
}
else {
    plan(tests => 6);
}


# Can't assume too much about the string returned by crypt(),
# and about how many bytes of the encrypted (really, hashed)
# string matter.
#
# HISTORICALLY the results started with the first two bytes of the salt,
# followed by 11 bytes from the set [./0-9A-Za-z], and only the first
# eight characters mattered, but those are probably no more safe
# bets, given alternative encryption/hashing schemes like MD5,
# C2 (or higher) security schemes, and non-UNIX platforms.
#
# On platforms implementing FIPS mode, using a weak algorithm (including
# the default triple-DES algorithm) causes crypt(3) to return a null
# pointer, which Perl converts into undef. We assume for now that all
# such platforms support glibc-style selection of a different hashing
# algorithm.
# glibc supports MD5, but OpenBSD only supports Blowfish.
my $alg = '';       # Use default algorithm
if ( !defined(crypt("ab", $alg."cd")) ) {
    $alg = '$5$';   # Try SHA-256
}
if ( !defined(crypt("ab", $alg."cd")) ) {
    $alg = '$2b$12$FPWWO2RJ3CK4FINTw0Hi';  # Try Blowfish
}
if ( !defined(crypt("ab", $alg."cd")) ) {
    $alg = ''; # Nothing worked.  Back to default
}

SKIP: {
    skip ("VOS crypt ignores salt.", 1) if ($^O eq 'vos');
    ok(substr(crypt("ab", $alg."cd"), length($alg)+2) ne 
       substr(crypt("ab", $alg."ce"), length($alg)+2),
       "salt makes a difference");
}

$a = "a\xFF\x{100}";

eval {$b = crypt($a, $alg."cd")};
like($@, qr/Wide character in crypt/, "wide characters ungood");

chop $a; # throw away the wide character

eval {$b = crypt($a, $alg."cd")};
is($@, '',                   "downgrade to eight bit characters");
is($b, crypt("a\xFF", $alg."cd"), "downgrade results agree");

my $x = chr 256; # has to be lexical, and predeclared
# Assignment gets optimised away here:
$x = crypt "foo", ${\"bar"}; # ${\ } to defeat constant folding
is $x, crypt("foo", "bar"), 'crypt writing to utf8 target';
ok !utf8::is_utf8($x), 'crypt turns off utf8 on its target';
