#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 42;

$^R = undef;
like( 'a',  qr/^a(?{1})(?:b(?{2}))?/, 'a =~ ab?' );
cmp_ok( $^R, '==', 1, '..$^R after a =~ ab?' );

$^R = undef;
unlike( 'abc', qr/^a(?{3})(?:b(?{4}))$/, 'abc !~ a(?:b)$' );
ok( !defined $^R, '..$^R after abc !~ a(?:b)$' );

$^R = undef;
like( 'ab', qr/^a(?{5})b(?{6})/, 'ab =~ ab' );
cmp_ok( $^R, '==', 6, '..$^R after ab =~ ab' );

$^R = undef;
like( 'ab', qr/^a(?{7})(?:b(?{8}))?/, 'ab =~ ab?' );

cmp_ok( $^R, '==', 8, '..$^R after ab =~ ab?' );

$^R = undef;
like( 'ab', qr/^a(?{9})b?(?{10})/, 'ab =~ ab? (2)' );
cmp_ok( $^R, '==', 10, '..$^R after ab =~ ab? (2)' );

$^R = undef;
like( 'ab', qr/^(a(?{11})(?:b(?{12})))?/, 'ab =~ (ab)? (3)' );
cmp_ok( $^R, '==', 12, '..$^R after ab =~ ab? (3)' );

$^R = undef;
unlike( 'ac', qr/^a(?{13})b(?{14})/, 'ac !~ ab' );
ok( !defined $^R, '..$^R after ac !~ ab' );

$^R = undef;
like( 'ac', qr/^a(?{15})(?:b(?{16}))?/, 'ac =~ ab?' );
cmp_ok( $^R, '==', 15, '..$^R after ac =~ ab?' );

my @ar;
like( 'ab', qr/^a(?{push @ar,101})(?:b(?{push @ar,102}))?/, 'ab =~ ab? with code push' );
cmp_ok( scalar(@ar), '==', 2, '..@ar pushed' );
cmp_ok( $ar[0], '==', 101, '..first element pushed' );
cmp_ok( $ar[1], '==', 102, '..second element pushed' );

$^R = undef;
unlike( 'a', qr/^a(?{103})b(?{104})/, 'a !~ ab with code push' );
ok( !defined $^R, '..$^R after a !~ ab with code push' );

@ar = ();
unlike( 'a', qr/^a(?{push @ar,105})b(?{push @ar,106})/, 'a !~ ab (push)' );
cmp_ok( scalar(@ar), '==', 0, '..nothing pushed' );

@ar = ();
unlike( 'abc', qr/^a(?{push @ar,107})b(?{push @ar,108})$/, 'abc !~ ab$ (push)' );
cmp_ok( scalar(@ar), '==', 0, '..still nothing pushed' );

our @var;

like( 'ab', qr/^a(?{push @var,109})(?:b(?{push @var,110}))?/, 'ab =~ ab? push to package var' );
cmp_ok( scalar(@var), '==', 2, '..@var pushed' );
cmp_ok( $var[0], '==', 109, '..first element pushed (package)' );
cmp_ok( $var[1], '==', 110, '..second element pushed (package)' );

@var = ();
unlike( 'a', qr/^a(?{push @var,111})b(?{push @var,112})/, 'a !~ ab (push package var)' );
cmp_ok( scalar(@var), '==', 0, '..nothing pushed (package)' );

@var = ();
unlike( 'abc', qr/^a(?{push @var,113})b(?{push @var,114})$/, 'abc !~ ab$ (push package var)' );
cmp_ok( scalar(@var), '==', 0, '..still nothing pushed (package)' );

{
    local $^R = undef;
    ok( 'ac' =~ /^a(?{30})(?:b(?{31})|c(?{32}))?/, 'ac =~ a(?:b|c)?' );
    ok( $^R == 32, '$^R == 32' );
}
{
    local $^R = undef;
    ok( 'abbb' =~ /^a(?{36})(?:b(?{37})|c(?{38}))+/, 'abbbb =~ a(?:b|c)+' );
    ok( $^R == 37, '$^R == 37' ) or print "# \$^R=$^R\n";
}

# Broken temporarily by the jumbo re-eval rewrite in 5.17.1; fixed in .6
{
    use re 'eval';
    $x = "(?{})";
    is eval { "a" =~ /a++(?{})+$x/x } || $@, '1', '/a++(?{})+$code_block/'
}

# [perl #78194] $_ in code block aliasing op return values
"$_" =~ /(?{ is \$_, \$_,
               '[perl #78194] \$_ == \$_ when $_ aliases "$x"' })/;

@a = 1..3;
like eval { qr/@a(?{})/ }, qr/1 2 3\(\?\{\}\)/, 'qr/@a(?{})/';

# Not a code block, but looks a bit like one.  (Failed an assertion from
# 5.17.1 to 5.21.6.)
ok "(?{" =~ qr/\Q(?{/, 'qr/\Q(?{/';
