#!/usr/bin/perl -Tw

BEGIN {
    if ($ENV{PERL_CORE}) {
	require Config; import Config;
	no warnings 'once';
	if ($Config{extensions} !~ /\bHash\/Util\b/) {
	    print "1..0 # Skip: Hash::Util was not built\n";
	    exit 0;
	}
    }
}

use strict;
use Test::More;

sub numbers_first { # Sort helper: All digit entries sort in front of others
                    # Makes sorting portable across ASCII/EBCDIC
    return $a cmp $b if ($a =~ /^\d+$/) == ($b =~ /^\d+$/);
    return -1 if $a =~ /^\d+$/;
    return 1;
}

my @Exported_Funcs;
BEGIN {
    @Exported_Funcs = qw(
                     fieldhash fieldhashes

                     all_keys
                     lock_keys unlock_keys
                     lock_value unlock_value
                     lock_hash unlock_hash
                     lock_keys_plus
                     hash_locked hash_unlocked
                     hashref_locked hashref_unlocked
                     hidden_keys legal_keys

                     lock_ref_keys unlock_ref_keys
                     lock_ref_value unlock_ref_value
                     lock_hashref unlock_hashref
                     lock_ref_keys_plus
                     hidden_ref_keys legal_ref_keys

                     hash_seed hash_value bucket_stats bucket_info bucket_array
                     hv_store
                     lock_hash_recurse unlock_hash_recurse
                     lock_hashref_recurse unlock_hashref_recurse
                    );
    plan tests => 250 + @Exported_Funcs;
    use_ok 'Hash::Util', @Exported_Funcs;
}
foreach my $func (@Exported_Funcs) {
    can_ok __PACKAGE__, $func;
}

my %hash = (foo => 42, bar => 23, locked => 'yep');
lock_keys(%hash);
eval { $hash{baz} = 99; };
like( $@, qr/^Attempt to access disallowed key 'baz' in a restricted hash/,
                                                       'lock_keys()');
is( $hash{bar}, 23, '$hash{bar} == 23' );
ok( !exists $hash{baz},'!exists $hash{baz}' );

delete $hash{bar};
ok( !exists $hash{bar},'!exists $hash{bar}' );
$hash{bar} = 69;
is( $hash{bar}, 69 ,'$hash{bar} == 69');

eval { () = $hash{i_dont_exist} };
like( $@, qr/^Attempt to access disallowed key 'i_dont_exist' in a restricted hash/,
      'Disallowed 1' );

lock_value(%hash, 'locked');
eval { print "# oops" if $hash{four} };
like( $@, qr/^Attempt to access disallowed key 'four' in a restricted hash/,
      'Disallowed 2' );

eval { $hash{"\x{2323}"} = 3 };
like( $@, qr/^Attempt to access disallowed key '(.*)' in a restricted hash/,
                                               'wide hex key' );

eval { delete $hash{locked} };
like( $@, qr/^Attempt to delete readonly key 'locked' from a restricted hash/,
                                           'trying to delete a locked key' );
eval { $hash{locked} = 42; };
like( $@, qr/^Modification of a read-only value attempted/,
                                           'trying to change a locked key' );
is( $hash{locked}, 'yep', '$hash{locked} is yep' );

eval { delete $hash{I_dont_exist} };
like( $@, qr/^Attempt to delete disallowed key 'I_dont_exist' from a restricted hash/,
                             'trying to delete a key that doesnt exist' );

ok( !exists $hash{I_dont_exist},'!exists $hash{I_dont_exist}' );

unlock_keys(%hash);
$hash{I_dont_exist} = 42;
is( $hash{I_dont_exist}, 42,    'unlock_keys' );

eval { $hash{locked} = 42; };
like( $@, qr/^Modification of a read-only value attempted/,
                             '  individual key still readonly' );
eval { delete $hash{locked} },
is( $@, '', '  but can be deleted :(' );

unlock_value(%hash, 'locked');
$hash{locked} = 42;
is( $hash{locked}, 42,  'unlock_value' );


{
    my %hash = ( foo => 42, locked => 23 );

    lock_keys(%hash);
    eval { %hash = ( wubble => 42 ) };  # we know this will bomb
    like( $@, qr/^Attempt to access disallowed key 'wubble'/,'Disallowed 3' );
    unlock_keys(%hash);
}

{
    my %hash = (KEY => 'val', RO => 'val');
    lock_keys(%hash);
    lock_value(%hash, 'RO');

    eval { %hash = (KEY => 1) };
    like( $@, qr/^Attempt to delete readonly key 'RO' from a restricted hash/,
        'attempt to delete readonly key from restricted hash' );
}

{
    my %hash = (KEY => 1, RO => 2);
    lock_keys(%hash);
    eval { %hash = (KEY => 1, RO => 2) };
    is( $@, '', 'No error message, as expected');
}

{
    my %hash = ();
    lock_keys(%hash, qw(foo bar));
    is( keys %hash, 0,  'lock_keys() w/keyset shouldnt add new keys' );
    $hash{foo} = 42;
    is( keys %hash, 1, '1 element in hash' );
    eval { $hash{wibble} = 42 };
    like( $@, qr/^Attempt to access disallowed key 'wibble' in a restricted hash/,
                        'write threw error (locked)');

    unlock_keys(%hash);
    eval { $hash{wibble} = 23; };
    is( $@, '', 'unlock_keys' );
}

{
    my %hash = (foo => 42, bar => undef, baz => 0);
    lock_keys(%hash, qw(foo bar baz up down));
    is( keys %hash, 3,   'lock_keys() w/keyset didnt add new keys' );
    is_deeply( \%hash, { foo => 42, bar => undef, baz => 0 },'is_deeply' );

    eval { $hash{up} = 42; };
    is( $@, '','No error 1' );

    eval { $hash{wibble} = 23 };
    like( $@, qr/^Attempt to access disallowed key 'wibble' in a restricted hash/,
          'locked "wibble"' );
}

{
    my %hash = (foo => 42, bar => undef);
    eval { lock_keys(%hash, qw(foo baz)); };
    like( $@, qr/^Hash has key 'bar' which is not in the new key set/,
                    'carp test' );
}

{
    my %hash = (foo => 42, bar => 23);
    lock_hash( %hash );
    ok( hashref_locked( \%hash ), 'hashref_locked' );
    ok( hash_locked( %hash ), 'hash_locked' );

    ok( Internals::SvREADONLY(%hash),'Was locked %hash' );
    ok( Internals::SvREADONLY($hash{foo}),'Was locked $hash{foo}' );
    ok( Internals::SvREADONLY($hash{bar}),'Was locked $hash{bar}' );

    unlock_hash ( %hash );
    ok( hashref_unlocked( { %hash } ), 'hashref_unlocked' );
    ok( hash_unlocked( %hash ), 'hash_unlocked' );

    ok( !Internals::SvREADONLY(%hash),'Was unlocked %hash' );
    ok( !Internals::SvREADONLY($hash{foo}),'Was unlocked $hash{foo}' );
    ok( !Internals::SvREADONLY($hash{bar}),'Was unlocked $hash{bar}' );
}

{
    my %hash = (foo => 42, bar => 23);
    ok( ! hashref_locked( { %hash } ), 'hashref_locked negated' );
    ok( ! hash_locked( %hash ), 'hash_locked negated' );

    lock_hash( %hash );
    ok( ! hashref_unlocked( \%hash ), 'hashref_unlocked negated' );
    ok( ! hash_unlocked( %hash ), 'hash_unlocked negated' );
}

lock_keys(%ENV);
eval { () = $ENV{I_DONT_EXIST} };
like(
    $@,
    qr/^Attempt to access disallowed key 'I_DONT_EXIST' in a restricted hash/,
    'locked %ENV'
);
unlock_keys(%ENV); # Test::Builder cannot print test failures otherwise

{
    my %hash;

    lock_keys(%hash, 'first');

    is (scalar keys %hash, 0, "place holder isn't a key");
    $hash{first} = 1;
    is (scalar keys %hash, 1, "we now have a key");
    delete $hash{first};
    is (scalar keys %hash, 0, "now no key");

    unlock_keys(%hash);

    $hash{interregnum} = 1.5;
    is (scalar keys %hash, 1, "key again");
    delete $hash{interregnum};
    is (scalar keys %hash, 0, "no key again");

    lock_keys(%hash, 'second');

    is (scalar keys %hash, 0, "place holder isn't a key");

    eval {$hash{zeroeth} = 0};
    like ($@,
          qr/^Attempt to access disallowed key 'zeroeth' in a restricted hash/,
          'locked key never mentioned before should fail');
    eval {$hash{first} = -1};
    like ($@,
          qr/^Attempt to access disallowed key 'first' in a restricted hash/,
          'previously locked place holders should also fail');
    is (scalar keys %hash, 0, "and therefore there are no keys");
    $hash{second} = 1;
    is (scalar keys %hash, 1, "we now have just one key");
    delete $hash{second};
    is (scalar keys %hash, 0, "back to zero");

    unlock_keys(%hash); # We have deliberately left a placeholder.

    $hash{void} = undef;
    $hash{nowt} = undef;

    is (scalar keys %hash, 2, "two keys, values both undef");

    lock_keys(%hash);

    is (scalar keys %hash, 2, "still two keys after locking");

    eval {$hash{second} = -1};
    like ($@,
          qr/^Attempt to access disallowed key 'second' in a restricted hash/,
          'previously locked place holders should fail');

    is ($hash{void}, undef,
        "undef values should not be misunderstood as placeholders");
    is ($hash{nowt}, undef,
        "undef values should not be misunderstood as placeholders (again)");
}

{
  # perl #18651 - tim@consultix-inc.com found a rather nasty data dependant
  # bug whereby hash iterators could lose hash keys (and values, as the code
  # is common) for restricted hashes.

  my @keys = qw(small medium large);

  # There should be no difference whether it is restricted or not
  foreach my $lock (0, 1) {
    # Try setting all combinations of the 3 keys
    foreach my $usekeys (0..7) {
      my @usekeys;
      for my $bits (0,1,2) {
	push @usekeys, $keys[$bits] if $usekeys & (1 << $bits);
      }
      my %clean = map {$_ => length $_} @usekeys;
      my %target;
      lock_keys ( %target, @keys ) if $lock;

      while (my ($k, $v) = each %clean) {
	$target{$k} = $v;
      }

      my $message
	= ($lock ? 'locked' : 'not locked') . ' keys ' . join ',', @usekeys;

      is (scalar keys %target, scalar keys %clean, "scalar keys for $message");
      is (scalar values %target, scalar values %clean,
	  "scalar values for $message");
      # Yes. All these sorts are necessary. Even for "identical hashes"
      # Because the data dependency of the test involves two of the strings
      # colliding on the same bucket, so the iterator order (output of keys,
      # values, each) depends on the addition order in the hash. And locking
      # the keys of the hash involves behind the scenes key additions.
      is_deeply( [sort keys %target] , [sort keys %clean],
		 "list keys for $message");
      is_deeply( [sort values %target] , [sort values %clean],
		 "list values for $message");

      is_deeply( [sort %target] , [sort %clean],
		 "hash in list context for $message");

      my (@clean, @target);
      while (my ($k, $v) = each %clean) {
	push @clean, $k, $v;
      }
      while (my ($k, $v) = each %target) {
	push @target, $k, $v;
      }

      is_deeply( [sort @target] , [sort @clean],
		 "iterating with each for $message");
    }
  }
}

# Check clear works on locked empty hashes - SEGVs on 5.8.2.
{
    my %hash;
    lock_hash(%hash);
    %hash = ();
    ok(keys(%hash) == 0, 'clear empty lock_hash() hash');
}
{
    my %hash;
    lock_keys(%hash);
    %hash = ();
    ok(keys(%hash) == 0, 'clear empty lock_keys() hash');
}

# Copy-on-write scalars should not be deletable after lock_hash;
{
    my %hash = (key=>__PACKAGE__);
    lock_hash(%hash);
    eval { delete $hash{key} };
    like $@, qr/^Attempt to delete readonly key /,
        'COW scalars are not exempt from lock_hash (delete)';
    eval { %hash = () };
    like $@, qr/^Attempt to delete readonly key /,
        'COW scalars are not exempt from lock_hash (clear)';
}

my $hash_seed = hash_seed();
ok(defined($hash_seed) && $hash_seed ne '', "hash_seed $hash_seed");

{
    package Minder;
    my $counter;
    sub DESTROY {
	--$counter;
    }
    sub new {
	++$counter;
	bless [], __PACKAGE__;
    }
    package main;

    for my $state ('', 'locked') {
	my $a = Minder->new();
	is ($counter, 1, "There is 1 object $state");
	my %hash;
	$hash{a} = $a;
	is ($counter, 1, "There is still 1 object $state");

	lock_keys(%hash) if $state;

	is ($counter, 1, "There is still 1 object $state");
	undef $a;
	is ($counter, 1, "Still 1 object $state");
	delete $hash{a};
	is ($counter, 0, "0 objects when hash key is deleted $state");
	$hash{a} = undef;
	is ($counter, 0, "Still 0 objects $state");
	%hash = ();
	is ($counter, 0, "0 objects after clear $state");
    }
}
{
    my %hash = map {$_,$_} qw(fwiffffff foosht teeoo);
    lock_keys(%hash);
    delete $hash{fwiffffff};
    is (scalar keys %hash, 2,"Count of keys after delete on locked hash");
    unlock_keys(%hash);
    is (scalar keys %hash, 2,"Count of keys after unlock");

    my ($first, $value) = each %hash;
    is ($hash{$first}, $value, "Key has the expected value before the lock");
    lock_keys(%hash);
    is ($hash{$first}, $value, "Key has the expected value after the lock");

    my ($second, $v2) = each %hash;

    is ($hash{$first}, $value, "Still correct after iterator advances");
    is ($hash{$second}, $v2, "Other key has the expected value");
}
{
    my $x='foo';
    my %test;
    hv_store(%test,'x',$x);
    is($test{x},'foo','hv_store() stored');
    $test{x}='bar';
    is($x,'bar','hv_store() aliased');
    is($test{x},'bar','hv_store() aliased and stored');
}

{
    my %hash=map { $_ => 1 } qw( a b c d e f);
    delete $hash{c};
    lock_keys(%hash);
    ok(Internals::SvREADONLY(%hash),'lock_keys DDS/t 1');
    delete @hash{qw(b e)};
    my @hidden=sort(hidden_keys(%hash));
    my @legal=sort(legal_keys(%hash));
    my @keys=sort(keys(%hash));
    #warn "@legal\n@keys\n";
    is("@hidden","b e",'lock_keys @hidden DDS/t');
    is("@legal","a b d e f",'lock_keys @legal DDS/t');
    is("@keys","a d f",'lock_keys @keys DDS/t');
}
{
    my %hash=(0..9);
    lock_keys(%hash);
    ok(Internals::SvREADONLY(%hash),'lock_keys DDS/t 2');
    Hash::Util::unlock_keys(%hash);
    ok(!Internals::SvREADONLY(%hash),'unlock_keys DDS/t 2');
}
{
    my %hash=(0..9);
    lock_keys(%hash,keys(%hash),'a'..'f');
    ok(Internals::SvREADONLY(%hash),'lock_keys args DDS/t');
    my @hidden=sort numbers_first hidden_keys(%hash);
    my @legal=sort numbers_first legal_keys(%hash);
    my @keys=sort numbers_first keys(%hash);
    is("@hidden","a b c d e f",'lock_keys() @hidden DDS/t 3');
    is("@legal","0 2 4 6 8 a b c d e f",'lock_keys() @legal DDS/t 3');
    is("@keys","0 2 4 6 8",'lock_keys() @keys');
}
{
    my %hash=map { $_ => 1 } qw( a b c d e f);
    delete $hash{c};
    lock_ref_keys(\%hash);
    ok(Internals::SvREADONLY(%hash),'lock_ref_keys DDS/t');
    delete @hash{qw(b e)};
    my @hidden=sort(hidden_keys(%hash));
    my @legal=sort(legal_keys(%hash));
    my @keys=sort(keys(%hash));
    #warn "@legal\n@keys\n";
    is("@hidden","b e",'lock_ref_keys @hidden DDS/t 1');
    is("@legal","a b d e f",'lock_ref_keys @legal DDS/t 1');
    is("@keys","a d f",'lock_ref_keys @keys DDS/t 1');
}
{
    my %hash=(0..9);
    lock_ref_keys(\%hash,keys %hash,'a'..'f');
    ok(Internals::SvREADONLY(%hash),'lock_ref_keys args DDS/t');
    my @hidden=sort numbers_first hidden_keys(%hash);
    my @legal=sort numbers_first legal_keys(%hash);
    my @keys=sort numbers_first keys(%hash);
    is("@hidden","a b c d e f",'lock_ref_keys() @hidden DDS/t 2');
    is("@legal","0 2 4 6 8 a b c d e f",'lock_ref_keys() @legal DDS/t 2');
    is("@keys","0 2 4 6 8",'lock_ref_keys() @keys DDS/t 2');
}
{
    my %hash=(0..9);
    lock_ref_keys_plus(\%hash,'a'..'f');
    ok(Internals::SvREADONLY(%hash),'lock_ref_keys_plus args DDS/t');
    my @hidden=sort numbers_first hidden_keys(%hash);
    my @legal=sort numbers_first legal_keys(%hash);
    my @keys=sort numbers_first keys(%hash);
    is("@hidden","a b c d e f",'lock_ref_keys_plus() @hidden DDS/t');
    is("@legal","0 2 4 6 8 a b c d e f",'lock_ref_keys_plus() @legal DDS/t');
    is("@keys","0 2 4 6 8",'lock_ref_keys_plus() @keys DDS/t');
}
{
    my %hash=(0..9, 'a' => 'alpha');
    lock_ref_keys_plus(\%hash,'a'..'f');
    ok(Internals::SvREADONLY(%hash),'lock_ref_keys_plus args overlap');
    my @hidden=sort numbers_first hidden_keys(%hash);
    my @legal=sort numbers_first legal_keys(%hash);
    my @keys=sort numbers_first keys(%hash);
    is("@hidden","b c d e f",'lock_ref_keys_plus() @hidden overlap');
    is("@legal","0 2 4 6 8 a b c d e f",'lock_ref_keys_plus() @legal overlap');
    is("@keys","0 2 4 6 8 a",'lock_ref_keys_plus() @keys overlap');
}
{
    my %hash=(0..9);
    lock_keys_plus(%hash,'a'..'f');
    ok(Internals::SvREADONLY(%hash),'lock_keys_plus args DDS/t');
    my @hidden=sort numbers_first hidden_keys(%hash);
    my @legal=sort numbers_first legal_keys(%hash);
    my @keys=sort numbers_first keys(%hash);
    is("@hidden","a b c d e f",'lock_keys_plus() @hidden DDS/t 3');
    is("@legal","0 2 4 6 8 a b c d e f",'lock_keys_plus() @legal DDS/t 3');
    is("@keys","0 2 4 6 8",'lock_keys_plus() @keys DDS/t 3');
}
{
    my %hash=(0..9, 'a' => 'alpha');
    lock_keys_plus(%hash,'a'..'f');
    ok(Internals::SvREADONLY(%hash),'lock_keys_plus args overlap non-ref');
    my @hidden=sort numbers_first hidden_keys(%hash);
    my @legal=sort numbers_first legal_keys(%hash);
    my @keys=sort numbers_first keys(%hash);
    is("@hidden","b c d e f",'lock_keys_plus() @hidden overlap non-ref');
    is("@legal","0 2 4 6 8 a b c d e f",'lock_keys_plus() @legal overlap non-ref');
    is("@keys","0 2 4 6 8 a",'lock_keys_plus() @keys overlap non-ref');
}

{
    my %hash = ('a'..'f');
    my @keys = ();
    my @ph = ();
    my @lock = ('a', 'c', 'e', 'g');
    lock_keys(%hash, @lock);
    my $ref = all_keys(%hash, @keys, @ph);
    my @crrack = sort(@keys);
    my @ooooff = qw(a c e);
    my @bam = qw(g);

    ok(ref $ref eq ref \%hash && $ref == \%hash, 
            "all_keys() - \$ref is a reference to \%hash");
    is_deeply(\@crrack, \@ooooff, "Keys are what they should be");
    is_deeply(\@ph, \@bam, "Placeholders in place");
}

{
    # lock_hash_recurse / unlock_hash_recurse
    my %hash = (
        a   => 'alpha',
        b   => [ qw( beta gamma delta ) ],
        c   => [ 'epsilon', { zeta => 'eta' }, ],
        d   => { theta => 'iota' },
    );
    lock_hash_recurse(%hash);
    ok( hash_locked(%hash),
        "lock_hash_recurse(): top-level hash locked" );
    ok( hash_locked(%{$hash{d}}),
        "lock_hash_recurse(): element which is hashref locked" );
    ok( ! hash_locked(%{$hash{c}[1]}),
        "lock_hash_recurse(): element which is hashref in array ref not locked" );

    unlock_hash_recurse(%hash);
    ok( hash_unlocked(%hash),
        "unlock_hash_recurse(): top-level hash unlocked" );
    ok( hash_unlocked(%{$hash{d}}),
        "unlock_hash_recurse(): element which is hashref unlocked" );
    {
        local $@;
        eval { $hash{d} = { theta => 'kappa' }; };
        ok(! $@, "No error; can assign to unlocked hash")
            or diag($@);
    }
    ok( hash_unlocked(%{$hash{c}[1]}),
        "unlock_hash_recurse(): element which is hashref in array ref not locked" );
}

{
    # lock_hashref_recurse / unlock_hashref_recurse
    my %hash = (
        a   => 'alpha',
        b   => [ qw( beta gamma delta ) ],
        c   => [ 'epsilon', { zeta => 'eta' }, ],
        d   => { theta => 'iota' },
    );
    Hash::Util::lock_hashref_recurse(\%hash);
    ok( hash_locked(%hash),
        "lock_hash_recurse(): top-level hash locked" );
    ok( hash_locked(%{$hash{d}}),
        "lock_hash_recurse(): element which is hashref locked" );
    ok( ! hash_locked(%{$hash{c}[1]}),
        "lock_hash_recurse(): element which is hashref in array ref not locked" );

    Hash::Util::unlock_hashref_recurse(\%hash);
    ok( hash_unlocked(%hash),
        "unlock_hash_recurse(): top-level hash unlocked" );
    ok( hash_unlocked(%{$hash{d}}),
        "unlock_hash_recurse(): element which is hashref unlocked" );
    {
        local $@;
        eval { $hash{d} = { theta => 'kappa' }; };
        ok(! $@, "No error; can assign to unlocked hash")
            or diag($@);
    }
    ok( hash_unlocked(%{$hash{c}[1]}),
        "unlock_hash_recurse(): element which is hashref in array ref not locked" );
}

{
    my $h1= hash_value("foo");
    my $h2= hash_value("bar");
    is( $h1, hash_value("foo") );
    is( $h2, hash_value("bar") );

    my $seed= hash_seed();
    my $h1s= hash_value("foo",$seed);
    my $h2s= hash_value("bar",$seed);

    is( $h1s, hash_value("foo",$seed) );
    is( $h2s, hash_value("bar",$seed) );

    $seed= join "", map { chr $_ } 1..length($seed);

    my $h1s2= hash_value("foo",$seed);
    my $h2s2= hash_value("bar",$seed);

    is( $h1s2, hash_value("foo",$seed) );
    is( $h2s2, hash_value("bar",$seed) );

    isnt($h1s,$h1s2);
    isnt($h1s,$h1s2);

}

{
    my @info1= bucket_info({});
    my @info2= bucket_info({1..10});
    my @stats1= bucket_stats({});
    my @stats2= bucket_stats({1..10});
    my $array1= bucket_array({});
    my $array2= bucket_array({1..10});
    is("@info1","0 8 0");
    like("@info2[0,1]",qr/5 (?:8|16)/);
    is("@stats1","0 8 0");
    like("@stats2[0,1]",qr/5 (?:8|16)/);
    my @keys1= sort map { ref $_ ? @$_ : () } @$array1;
    my @keys2= sort map { ref $_ ? @$_ : () } @$array2;
    is("@keys1","");
    is("@keys2","1 3 5 7 9");
}
