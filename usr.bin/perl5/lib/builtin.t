#!./perl -T

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use v5.36;
no warnings 'experimental::builtin';

package FetchStoreCounter {
    sub TIESCALAR($class, @args) { bless \@args, $class }

    sub FETCH($self)    { $self->[0]->$*++ }
    sub STORE($self, $) { $self->[1]->$*++ }
}

# booleans
{
    use builtin qw( true false is_bool );

    ok(true, 'true is true');
    ok(!false, 'false is false');

    ok(is_bool(true), 'true is bool');
    ok(is_bool(false), 'false is bool');
    ok(!is_bool(undef), 'undef is not bool');
    ok(!is_bool(1), '1 is not bool');
    ok(!is_bool(""), 'empty is not bool');

    my $truevar  = (5 == 5);
    my $falsevar = (5 == 6);

    ok(is_bool($truevar), '$truevar is bool');
    ok(is_bool($falsevar), '$falsevar is bool');

    ok(is_bool(is_bool(true)), 'is_bool true is bool');
    ok(is_bool(is_bool(123)),  'is_bool false is bool');

    # Invokes magic

    tie my $tied, FetchStoreCounter => (\my $fetchcount, \my $storecount);

    my $_dummy = is_bool($tied);
    is($fetchcount, 1, 'is_bool() invokes FETCH magic');

    $tied = is_bool(false);
    is($storecount, 1, 'is_bool() invokes STORE magic');

    is(prototype(\&builtin::is_bool), '$', 'is_bool prototype');
}

# weakrefs
{
    use builtin qw( is_weak weaken unweaken );

    my $arr = [];
    my $ref = $arr;

    ok(!is_weak($ref), 'ref is not weak initially');

    weaken($ref);
    ok(is_weak($ref), 'ref is weak after weaken()');

    unweaken($ref);
    ok(!is_weak($ref), 'ref is not weak after unweaken()');

    weaken($ref);
    undef $arr;
    is($ref, undef, 'ref is now undef after arr is cleared');

    is(prototype(\&builtin::weaken), '$', 'weaken prototype');
    is(prototype(\&builtin::unweaken), '$', 'unweaken prototype');
    is(prototype(\&builtin::is_weak), '$', 'is_weak prototype');
}

# reference queries
{
    use builtin qw( refaddr reftype blessed );

    my $arr = [];
    my $obj = bless [], "Object";

    is(refaddr($arr),        $arr+0, 'refaddr yields same as ref in numeric context');
    is(refaddr("not a ref"), undef,  'refaddr yields undef for non-reference');

    is(reftype($arr),        "ARRAY", 'reftype yields type string');
    is(reftype($obj),        "ARRAY", 'reftype yields basic container type for blessed object');
    is(reftype("not a ref"), undef,   'reftype yields undef for non-reference');

    is(blessed($arr), undef, 'blessed yields undef for non-object');
    is(blessed($obj), "Object", 'blessed yields package name for object');

    # blessed() as a boolean
    is(blessed($obj) ? "YES" : "NO", "YES", 'blessed in boolean context still works');

    # blessed() appears false as a boolean on package "0"
    is(blessed(bless [], "0") ? "YES" : "NO", "NO", 'blessed in boolean context handles "0" cornercase');

    is(prototype(\&builtin::blessed), '$', 'blessed prototype');
    is(prototype(\&builtin::refaddr), '$', 'refaddr prototype');
    is(prototype(\&builtin::reftype), '$', 'reftype prototype');
}

# created_as_...
{
    use builtin qw( created_as_string created_as_number );

    # some literal constants
    ok(!created_as_string(undef), 'undef created as !string');
    ok(!created_as_number(undef), 'undef created as !number');

    ok( created_as_string("abc"), 'abc created as string');
    ok(!created_as_number("abc"), 'abc created as number');

    ok(!created_as_string(123),   '123 created as !string');
    ok( created_as_number(123),   '123 created as !number');

    ok(!created_as_string(1.23),   '1.23 created as !string');
    ok( created_as_number(1.23),   '1.23 created as !number');

    ok(!created_as_string([]),    '[] created as !string');
    ok(!created_as_number([]),    '[] created as !number');

    ok(!created_as_string(builtin::true), 'true created as !string');
    ok(!created_as_number(builtin::true), 'true created as !number');

    ok(builtin::is_bool(created_as_string(0)), 'created_as_string returns bool');
    ok(builtin::is_bool(created_as_number(0)), 'created_as_number returns bool');

    # variables
    my $just_pv = "def";
    ok( created_as_string($just_pv), 'def created as string');
    ok(!created_as_number($just_pv), 'def created as number');

    my $just_iv = 456;
    ok(!created_as_string($just_iv), '456 created as string');
    ok( created_as_number($just_iv), '456 created as number');

    my $just_nv = 4.56;
    ok(!created_as_string($just_nv), '456 created as string');
    ok( created_as_number($just_nv), '456 created as number');

    # variables reused
    my $originally_pv = "1";
    my $pv_as_iv = $originally_pv + 0;
    ok( created_as_string($originally_pv), 'PV reused as IV created as string');
    ok(!created_as_number($originally_pv), 'PV reused as IV created as !number');
    ok(!created_as_string($pv_as_iv), 'New number from PV created as !string');
    ok( created_as_number($pv_as_iv), 'New number from PV created as number');

    my $originally_iv = 1;
    my $iv_as_pv = "$originally_iv";
    ok(!created_as_string($originally_iv), 'IV reused as PV created as !string');
    ok( created_as_number($originally_iv), 'IV reused as PV created as number');
    ok( created_as_string($iv_as_pv), 'New string from IV created as string');
    ok(!created_as_number($iv_as_pv), 'New string from IV created as !number');

    my $originally_nv = 1.1;
    my $nv_as_pv = "$originally_nv";
    ok(!created_as_string($originally_nv), 'NV reused as PV created as !string');
    ok( created_as_number($originally_nv), 'NV reused as PV created as number');
    ok( created_as_string($nv_as_pv), 'New string from NV created as string');
    ok(!created_as_number($nv_as_pv), 'New string from NV created as !number');

    # magic
    local $1;
    "hello" =~ m/(.*)/;
    ok(created_as_string($1), 'magic string');

    is(prototype(\&builtin::created_as_string), '$', 'created_as_string prototype');
    is(prototype(\&builtin::created_as_number), '$', 'created_as_number prototype');
}

# ceil, floor
{
    use builtin qw( ceil floor );

    cmp_ok(ceil(1.5), '==', 2, 'ceil(1.5) == 2');
    cmp_ok(floor(1.5), '==', 1, 'floor(1.5) == 1');

    # Invokes magic

    tie my $tied, FetchStoreCounter => (\my $fetchcount, \my $storecount);

    my $_dummy = ceil($tied);
    is($fetchcount, 1, 'ceil() invokes FETCH magic');

    $tied = ceil(1.1);
    is($storecount, 1, 'ceil() TARG invokes STORE magic');

    $fetchcount = $storecount = 0;
    tie $tied, FetchStoreCounter => (\$fetchcount, \$storecount);

    $_dummy = floor($tied);
    is($fetchcount, 1, 'floor() invokes FETCH magic');

    $tied = floor(1.1);
    is($storecount, 1, 'floor() TARG invokes STORE magic');

    is(prototype(\&builtin::ceil), '$', 'ceil prototype');
    is(prototype(\&builtin::floor), '$', 'floor prototype');
}

# imports are lexical; should not be visible here
{
    my $ok = eval 'true()'; my $e = $@;
    ok(!$ok, 'true() not visible outside of lexical scope');
    like($e, qr/^Undefined subroutine &main::true called at /, 'failure from true() not visible');
}

# lexical imports work fine in a variety of situations
{
    sub regularfunc {
        use builtin 'true';
        return true;
    }
    ok(regularfunc(), 'true in regular sub');

    my sub lexicalfunc {
        use builtin 'true';
        return true;
    }
    ok(lexicalfunc(), 'true in lexical sub');

    my $coderef = sub {
        use builtin 'true';
        return true;
    };
    ok($coderef->(), 'true in anon sub');

    sub recursefunc {
        use builtin 'true';
        return recursefunc() if @_;
        return true;
    }
    ok(recursefunc("rec"), 'true in self-recursive sub');

    my $recursecoderef = sub {
        use feature 'current_sub';
        use builtin 'true';
        return __SUB__->() if @_;
        return true;
    };
    ok($recursecoderef->("rec"), 'true in self-recursive anon sub');
}

{
    use builtin qw( true false );

    my $val = true;
    cmp_ok($val, $_, !!1, "true is equivalent to !!1 by $_") for qw( eq == );
    cmp_ok($val, $_,  !0, "true is equivalent to  !0 by $_") for qw( eq == );

    $val = false;
    cmp_ok($val, $_, !!0, "false is equivalent to !!0 by $_") for qw( eq == );
    cmp_ok($val, $_,  !1, "false is equivalent to  !1 by $_") for qw( eq == );
}

# indexed
{
    use builtin qw( indexed );

    # We don't have Test::More's is_deeply here

    ok(eq_array([indexed], [] ),
        'indexed on empty list');

    ok(eq_array([indexed "A"], [0, "A"] ),
        'indexed on singleton list');

    ok(eq_array([indexed "X" .. "Z"], [0, "X", 1, "Y", 2, "Z"] ),
        'indexed on 3-item list');

    my @orig = (1..3);
    $_++ for indexed @orig;
    ok(eq_array(\@orig, [1 .. 3]), 'indexed copies values, does not alias');

    {
        no warnings 'experimental::for_list';

        my $ok = 1;
        foreach my ($len, $s) (indexed "", "x", "xx") {
            length($s) == $len or undef $ok;
        }
        ok($ok, 'indexed operates nicely with multivar foreach');
    }

    {
        my %hash = indexed "a" .. "e";
        ok(eq_hash(\%hash, { 0 => "a", 1 => "b", 2 => "c", 3 => "d", 4 => "e" }),
            'indexed can be used to create hashes');
    }

    {
        no warnings 'scalar';

        my $count = indexed 'i', 'ii', 'iii', 'iv';
        is($count, 8, 'indexed in scalar context yields size of list it would return');
    }
}

# Vanilla trim tests
{
    use builtin qw( trim );

    is(trim("    Hello world!   ")      , "Hello world!"  , 'trim spaces');
    is(trim("\tHello world!\t")         , "Hello world!"  , 'trim tabs');
    is(trim("\n\n\nHello\nworld!\n")    , "Hello\nworld!" , 'trim \n');
    is(trim("\t\n\n\nHello world!\n \t"), "Hello world!"  , 'trim all three');
    is(trim("Perl")                     , "Perl"          , 'trim nothing');
    is(trim('')                         , ""              , 'trim empty string');

    is(prototype(\&builtin::trim), '$', 'trim prototype');
}

TODO: {
    my $warn = '';
    local $SIG{__WARN__} = sub { $warn .= join "", @_; };

    is(builtin::trim(undef), "", 'trim undef');
    like($warn    , qr/^Use of uninitialized value in subroutine entry at/,
         'trim undef triggers warning');
    local $main::TODO = "Currently uses generic value for the name of non-opcode builtins";
    like($warn    , qr/^Use of uninitialized value in trim at/,
         'trim undef triggers warning using actual name of builtin');
}

# Fancier trim tests against a regexp and unicode
{
    use builtin qw( trim );
    my $nbsp = chr utf8::unicode_to_native(0xA0);

    is(trim("   \N{U+2603}       "), "\N{U+2603}", 'trim with unicode content');
    is(trim("\N{U+2029}foobar\x{2028} "), "foobar",
            'trim with unicode whitespace');
    is(trim("$nbsp foobar$nbsp    "), "foobar", 'trim with latin1 whitespace');
}

# Test on a magical fetching variable
{
    use builtin qw( trim );

    my $str3 = "   Hello world!\t";
    $str3 =~ m/(.+Hello)/;
    is(trim($1), "Hello", "trim on a magical variable");
}

# Inplace edit, my, our variables
{
    use builtin qw( trim );

    my $str4 = "\t\tHello world!\n\n";
    $str4 = trim($str4);
    is($str4, "Hello world!", "trim on an inplace variable");

    our $str2 = "\t\nHello world!\t  ";
    is(trim($str2), "Hello world!", "trim on an our \$var");
}

# is_tainted
{
    use builtin qw( is_tainted );

    is(is_tainted($0), !!${^TAINT}, "\$0 is tainted (if tainting is supported)");
    ok(!is_tainted($1), "\$1 isn't tainted");

    # Invokes magic
    tie my $tied, FetchStoreCounter => (\my $fetchcount, \my $storecount);

    my $_dummy = is_tainted($tied);
    is($fetchcount, 1, 'is_tainted() invokes FETCH magic');

    $tied = is_tainted($0);
    is($storecount, 1, 'is_tainted() invokes STORE magic');

    is(prototype(\&builtin::is_tainted), '$', 'is_tainted prototype');
}

# Lexical export
{
    my $name;
    BEGIN {
        use builtin qw( export_lexically );

        $name = "message";
        export_lexically $name => sub { "Hello, world" };
    }

    is(message(), "Hello, world", 'Lexically exported sub is callable');
    ok(!__PACKAGE__->can("message"), 'Exported sub is not visible via ->can');

    is($name, "message", '$name argument was not modified by export_lexically');

    our ( $scalar, @array, %hash );
    BEGIN {
        use builtin qw( export_lexically );

        export_lexically
            '$SCALAR' => \$scalar,
            '@ARRAY'  => \@array,
            '%HASH'   => \%hash;
    }

    $::scalar = "value";
    is($SCALAR, "value", 'Lexically exported scalar is accessible');

    @::array = ('a' .. 'e');
    is(scalar @ARRAY, 5, 'Lexically exported array is accessible');

    %::hash = (key => "val");
    is($HASH{key}, "val", 'Lexically exported hash is accessible');
}

# vim: tabstop=4 shiftwidth=4 expandtab autoindent softtabstop=4

done_testing();
