use strict;
use warnings;
no warnings 'experimental::builtin';
use builtin qw(refaddr);

use Test::More;
use Hash::Util::FieldHash qw( :all);
my $ob_reg = Hash::Util::FieldHash::_ob_reg;

#########################

my $fieldhash_mode = 2;

# define ref types to use with some tests
# skipping CODE refs, they are differently scoped
my @test_types = qw(SCALAR ARRAY HASH GLOB);

### The id() function
{
    my $ref = [];
    is id( $ref), refaddr( $ref), "id is refaddr";
    my %h;
    Hash::Util::FieldHash::_fieldhash \ %h, $fieldhash_mode;
    $h{ $ref} = ();
    my ( $key) = keys %h;
    is id( $ref), $key, "id is FieldHash key";
    my $scalar = 'string';
    is id( $scalar), $scalar, "string passes unchanged";
    $scalar = 1234;
    is id( $scalar), $scalar, "number passes unchanged";
}

### idhash functionality
{
    Hash::Util::FieldHash::idhash my %h;
    my $ref = sub {};
    my $val = 123;
    $h{ $ref} = $val;
    my ( $key) = keys %h;
    is $key, id( $ref), "idhash key correct";
    is $h{ $ref}, $val, "value retrieved through ref";
    is scalar keys %$ob_reg, 0, "no auto-registry in idhash";
}

### the register() and id_2obj functions
{
    my $obj = {};
    my $id = id( $obj);
    is id_2obj( $id), undef, "unregistered object not retrieved";
    is scalar keys %$ob_reg, 0, "object registry empty";
    is register( $obj), $obj, "object returned by register";
    is scalar keys %$ob_reg, 1, "object registry nonempty";
    is id_2obj( $id), $obj, "registered object retrieved";
    my %hash;
    register( $obj, \ %hash);
    $hash{ $id} = 123;
    is scalar keys %hash, 1, "key present in registered hash";
    undef $obj;
    is scalar keys %hash, 0, "key collected from registered hash";
    is scalar keys %$ob_reg, 0, "object registry empty again";
    eval { register( 1234) };
    like $@, qr/^Attempt to register/, "registering non-ref is fatal";
}

### Object auto-registry
{
    {
        my $obj = {};
        {
            my $h = {};
            Hash::Util::FieldHash::_fieldhash $h, $fieldhash_mode;
            $h->{ $obj} = 123;
            is( keys %$ob_reg, 1, "one object registered");
        }
        # field hash stays alive until $obj dies
        is( keys %$ob_reg, 1, "object still registered");
    }
    is( keys %$ob_reg, 0, "object unregistered");
}

### existence/retrieval/deletion
{
    no warnings 'misc';
    my $val = 123;
    Hash::Util::FieldHash::_fieldhash \ my( %h), $fieldhash_mode;
    for ( [ str => 'abc'], [ ref => {}] ) {
        my ( $keytype, $key) = @$_;
        $h{ $key} = $val;
        ok( exists $h{ $key},  "existence ($keytype)");
        is( $h{ $key}, $val,   "retrieval ($keytype)");
        delete $h{ $key};
        is( keys %h, 0, "deletion ($keytype)");
    }
}

### id-action (stringification independent of bless)
{
    my( %f, %g, %h, %i);
    Hash::Util::FieldHash::_fieldhash \ %f, $fieldhash_mode;
    Hash::Util::FieldHash::_fieldhash \ %g, $fieldhash_mode;
    my $val = 123;
    my $key = [];
    $f{ $key} = $val;
    is( $f{ $key}, $val, "plain key set in field");
    my ( $id) = keys %f;
    my $refaddr = refaddr($key);
    is $id, $refaddr, "key is refaddr";
    bless $key;
    is( $f{ $key}, $val, "access through blessed");
    $key = [];
    $h{ $key} = $val;
    is( $h{ $key}, $val, "plain key set in hash");
    bless $key;
    isnt( $h{ $key}, $val, "no access through blessed");
}

# Garbage collection
{
    my %h;
    Hash::Util::FieldHash::_fieldhash \ %h, $fieldhash_mode;
    $h{ []} = 123;
    is( keys %h, 0, "blip");
}

for my $preload ( [], [ map {}, 1 .. 3] ) {
    my $pre = @$preload ? ' (preloaded)' : '';
    my %f;
    Hash::Util::FieldHash::_fieldhash \ %f, $fieldhash_mode;
    my @preval = map "$_", @$preload;
    @f{ @$preload} = @preval;
    # Garbage collection separately
    for my $type ( @test_types) {
        {
            my $ref = gen_ref( $type);
            $f{ $ref} = $type;
            my ( $val) = grep $_ eq $type, values %f;
            is( $val, $type, "$type visible$pre");
            is(
                keys %$ob_reg,
                1 + @$preload,
                "$type obj registered$pre"
            );
        }
        is( keys %f, @$preload, "$type gone$pre");
    }

    # Garbage collection collectively
    is( keys %$ob_reg, @$preload, "no objs remaining$pre");
    {
        my @refs = map gen_ref( $_), @test_types;
        @f{ @refs} = @test_types;
        is_deeply(
            [ sort values %f], [ sort ( @test_types, @preval) ],
            "all types present$pre",
        );
        is(
            keys %$ob_reg,
            @test_types + @$preload,
            "all types registered$pre",
        );
    }
    die "preload gone" unless defined $preload;
    is_deeply( [ sort values %f], [ sort @preval], "all types gone$pre");
    is( keys %$ob_reg, @$preload, "all types unregistered$pre");
}
is( keys %$ob_reg, 0, "preload gone after loop");

# autovivified key
{
    my %h;
    Hash::Util::FieldHash::_fieldhash \ %h, $fieldhash_mode;
    my $ref = {};
    my $x = $h{ $ref}->[ 0];
    is keys %h, 1, "autovivified key present";
    undef $ref;
    is keys %h, 0, "autovivified key collected";
}

# big key sets
{
    my $size = 10_000;
    my %f;
    Hash::Util::FieldHash::_fieldhash \ %f, $fieldhash_mode;
    {
        my @refs = map [], 1 .. $size;
        $f{ $_} = 1 for @refs;
        is( keys %f, $size, "many keys singly");
        is(
            keys %$ob_reg,
            $size,
            "many objects singly",
        );
    }
    is( keys %f, 0, "many keys singly gone");
    is(
        keys %$ob_reg,
        0,
        "many objects singly unregistered",
    );

    {
        my @refs = map [], 1 .. $size;
        @f{ @refs } = ( 1) x @refs;
        is( keys %f, $size, "many keys at once");
        is(
            keys %$ob_reg,
            $size,
            "many objects at once",
        );
    }
    is( keys %f, 0, "many keys at once gone");
    is(
        keys %$ob_reg,
        0,
        "many objects at once unregistered",
    );
}

# many field hashes
{
    my $n_fields = 1000;
    my @fields = map {}, $n_fields;
    Hash::Util::FieldHash::_fieldhash( $_, $fieldhash_mode) for @fields;
    my @obs = map gen_ref( $_), @test_types;
    my $n_obs = @obs;
    for my $field ( @fields ) {
        @{ $field }{ @obs} = map ref, @obs;
    }
    my $err = grep keys %$_ != @obs, @fields;
    is( $err, 0, "$n_obs entries in $n_fields fields");
    is( keys %$ob_reg, @obs, "$n_obs obs registered");
    pop @obs;
    $err = grep keys %$_ != @obs, @fields;
    is( $err, 0, "one entry gone from $n_fields fields");
    is( keys %$ob_reg, @obs, "one ob unregistered");
    @obs = ();
    $err = grep keys %$_ != @obs, @fields;
    is( $err, 0, "all entries gone from $n_fields fields");
    is( keys %$ob_reg, @obs, "all obs unregistered");
}


# direct hash assignment
{
    Hash::Util::FieldHash::_fieldhash( $_, $fieldhash_mode) for \ my( %f, %g, %h);
    my $size = 6;
    my @obs = map [], 1 .. $size;
    @f{ @obs} = ( 1) x $size;
    $g{ $_} = $f{ $_} for keys %f; # single assignment
    %h = %f;                       # wholesale assignment
    @obs = ();
    is keys %$ob_reg, 0, "all keys collected";
    is keys %f, 0, "orig garbage-collected";
    is keys %g, 0, "single-copy garbage-collected";
    is keys %h, 0, "wholesale-copy garbage-collected";
}

{
    # prototypes in place?
    my %proto_tab = (
        fieldhash   => '\\%',
        fieldhashes => '',
        idhash      => '\\%',
        idhashes    => '',
        id          => '$',
        id_2obj     => '$',
        register    => '$@',
    );


    my @notfound = grep !exists $proto_tab{ $_} =>
        @Hash::Util::FieldHash::EXPORT_OK;
    ok @notfound == 0, "All exports in table";
    is prototype( "Hash::Util::FieldHash::$_") || '', $proto_tab{ $_},
        "$_ has prototype ($proto_tab{ $_})" for
            @Hash::Util::FieldHash::EXPORT_OK;
}

{
    Hash::Util::FieldHash::_fieldhash \ my( %h), $fieldhash_mode;
    bless \ %h, 'abc'; # this bus-errors with a certain bug
    ok( 1, "no bus error on bless")
}

#######################################################################

use Symbol qw( gensym);

BEGIN {
    my %gen = (
        SCALAR => sub { \ my $o },
        ARRAY  => sub { [] },
        HASH   => sub { {} },
        GLOB   => sub { gensym },
        CODE   => sub { sub {} },
    );

    sub gen_ref { $gen{ shift()}->() }
}

done_testing;
