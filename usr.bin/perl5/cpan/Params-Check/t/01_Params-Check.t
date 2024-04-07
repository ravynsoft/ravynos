use strict;
use Test::More 'no_plan';

### use && import ###
BEGIN {
    use_ok( 'Params::Check' );
    Params::Check->import(qw|check last_error allow|);
}

### verbose is good for debugging ###
$Params::Check::VERBOSE = $Params::Check::VERBOSE = $ARGV[0] ? 1 : 0;

### basic things first, allow function ###

use constant FALSE  => sub { 0 };
use constant TRUE   => sub { 1 };

### allow tests ###
{   ok( allow( 42, qr/^\d+$/ ), "Allow based on regex" );
    ok( allow( $0, $0),         "   Allow based on string" );
    ok( allow( 42, [0,42] ),    "   Allow based on list" );
    ok( allow( 42, [50,sub{1}]),"   Allow based on list containing sub");
    ok( allow( 42, TRUE ),      "   Allow based on constant sub" );
    ok(!allow( $0, qr/^\d+$/ ), "Disallowing based on regex" );
    ok(!allow( 42, $0 ),        "   Disallowing based on string" );
    ok(!allow( 42, [0,$0] ),    "   Disallowing based on list" );
    ok(!allow( 42, [50,sub{0}]),"   Disallowing based on list containing sub");
    ok(!allow( 42, FALSE ),     "   Disallowing based on constant sub" );

    ### check that allow short circuits where required
    {   my $sub_called;
        allow( 1, [ 1, sub { $sub_called++ } ] );
        ok( !$sub_called,       "Allow short-circuits properly" );
    }

    ### check if the subs for allow get what you expect ###
    for my $thing (1,'foo',[1]) {
        allow( $thing,
           sub { is_deeply(+shift,$thing,  "Allow coderef gets proper args") }
        );
    }
}
### default tests ###
{
    my $tmpl =  {
        foo => { default => 1 }
    };

    ### empty args first ###
    {   my $args = check( $tmpl, {} );

        ok( $args,              "check() call with empty args" );
        is( $args->{'foo'}, 1,  "   got default value" );
    }

    ### now provide an alternate value ###
    {   my $try  = { foo => 2 };
        my $args = check( $tmpl, $try );

        ok( $args,              "check() call with defined args" );
        is_deeply( $args, $try, "   found provided value in rv" );
    }

    ### now provide a different case ###
    {   my $try  = { FOO => 2 };
        my $args = check( $tmpl, $try );
        ok( $args,              "check() call with alternate case" );
        is( $args->{foo}, 2,    "   found provided value in rv" );
    }

    ### now see if we can strip leading dashes ###
    {   local $Params::Check::STRIP_LEADING_DASHES = 1;
        my $try  = { -foo => 2 };
        my $get  = { foo  => 2 };

        my $args = check( $tmpl, $try );
        ok( $args,              "check() call with leading dashes" );
        is_deeply( $args, $get, "   found provided value in rv" );
    }
}

### preserve case tests ###
{   my $tmpl = { Foo => { default => 1 } };

    for (1,0) {
        local $Params::Check::PRESERVE_CASE = $_;

        my $expect = $_ ? { Foo => 42 } : { Foo => 1 };

        my $rv = check( $tmpl, { Foo => 42 } );
        ok( $rv,                "check() call using PRESERVE_CASE: $_" );
        is_deeply($rv, $expect, "   found provided value in rv" );
    }
}


### unknown tests ###
{
    ### disallow unknowns ###
    {
        my $rv = check( {}, { foo => 42 } );

        is_deeply( $rv, {},     "check() call with unknown arguments" );
        like( last_error(), qr/^Key 'foo' is not a valid key/,
                                "   warning recorded ok" );
    }

    ### allow unknown ###
    {
        local   $Params::Check::ALLOW_UNKNOWN = 1;
        my $rv = check( {}, { foo => 42 } );

        is_deeply( $rv, { foo => 42 },
                                "check call() with unknown args allowed" );
    }
}

### store tests ###
{   my $foo;
    my $tmpl = {
        foo => { store => \$foo }
    };

    ### with/without store duplicates ###
    for( 1, 0 ) {
        local   $Params::Check::NO_DUPLICATES = $_;

        my $expect = $_ ? undef : 42;

        my $rv = check( $tmpl, { foo => 42 } );
        ok( $rv,                    "check() call with store key, no_dup: $_" );
        is( $foo, 42,               "   found provided value in variable" );
        is( $rv->{foo}, $expect,    "   found provided value in variable" );
    }
}

### no_override tests ###
{   my $tmpl = {
        foo => { no_override => 1, default => 42 },
    };

    my $rv = check( $tmpl, { foo => 13 } );
    ok( $rv,                    "check() call with no_override key" );
    is( $rv->{'foo'}, 42,       "   found default value in rv" );

    like( last_error(), qr/^You are not allowed to override key/,
                                "   warning recorded ok" );
}

### strict_type tests ###
{   my @list = (
        [ { strict_type => 1, default => [] },  0 ],
        [ { default => [] },                    1 ],
    );

    ### check for strict_type global, and in the template key ###
    for my $aref (@list) {

        my $tmpl = { foo => $aref->[0] };
        local   $Params::Check::STRICT_TYPE = $aref->[1];

        ### proper value ###
        {   my $rv = check( $tmpl, { foo => [] } );
            ok( $rv,                "check() call with strict_type enabled" );
            is( ref $rv->{foo}, 'ARRAY',
                                    "   found provided value in rv" );
        }

        ### improper value ###
        {   my $rv = check( $tmpl, { foo => {} } );
            ok( !$rv,               "check() call with strict_type violated" );
            like( last_error(), qr/^Key 'foo' needs to be of type 'ARRAY'/,
                                    "   warning recorded ok" );
        }
    }
}

### required tests ###
{   my $tmpl = {
        foo => { required => 1 }
    };

    ### required value provided ###
    {   my $rv = check( $tmpl, { foo => 42 } );
        ok( $rv,                    "check() call with required key" );
        is( $rv->{foo}, 42,         "   found provided value in rv" );
    }

    ### required value omitted ###
    {   my $rv = check( $tmpl, { } );
        ok( !$rv,                   "check() call with required key omitted" );
        like( last_error, qr/^Required option 'foo' is not provided/,
                                    "   warning recorded ok" );
    }
}

### defined tests ###
{   my @list = (
        [ { defined => 1, default => 1 },  0 ],
        [ { default => 1 },                1 ],
    );

    ### check for strict_type global, and in the template key ###
    for my $aref (@list) {

        my $tmpl = { foo => $aref->[0] };
        local   $Params::Check::ONLY_ALLOW_DEFINED = $aref->[1];

        ### value provided defined ###
        {   my $rv = check( $tmpl, { foo => 42 } );
            ok( $rv,                "check() call with defined key" );
            is( $rv->{foo}, 42,     "   found provided value in rv" );
        }

        ### value provided undefined ###
        {   my $rv = check( $tmpl, { foo => undef } );
            ok( !$rv,               "check() call with defined key undefined" );
            like( last_error, qr/^Key 'foo' must be defined when passed/,
                                    "   warning recorded ok" );
        }
    }
}

### check + allow tests ###
{   ### check if the subs for allow get what you expect ###
    for my $thing (1,'foo',[1]) {
        my $tmpl = {
            foo => { allow =>
                    sub { is_deeply(+shift,$thing,
                                    "   Allow coderef gets proper args") }
            }
        };

        my $rv = check( $tmpl, { foo => $thing } );
        ok( $rv,                    "check() call using allow key" );
    }
}

### invalid key tests
{   my $tmpl = { foo => { allow => sub { 0 } } };

    for my $val ( 1, 'foo', [], bless({},__PACKAGE__) ) {
        my $rv      = check( $tmpl, { foo => $val } );
        my $text    = "Key 'foo' ($val) is of invalid type";
        my $re      = quotemeta $text;

        ok(!$rv,                    "check() fails with unallowed value" );
        like(last_error(), qr/$re/, "   $text" );
    }
}

### warnings [rt.cpan.org #69626]
{
    local $Params::Check::WARNINGS_FATAL = 1;

    eval { check() };

    ok( $@,             "Call dies with fatal toggled" );
    like( $@,           qr/expects two arguments/,
                            "   error stored ok" );
}

### warnings fatal test
{   my $tmpl = { foo => { allow => sub { 0 } } };

    local $Params::Check::WARNINGS_FATAL = 1;

    eval { check( $tmpl, { foo => 1 } ) };

    ok( $@,             "Call dies with fatal toggled" );
    like( $@,           qr/invalid type/,
                            "   error stored ok" );
}

### store => \$foo tests
{   ### quell warnings
    local $SIG{__WARN__} = sub {};

    my $tmpl = { foo => { store => '' } };
    check( $tmpl, {} );

    my $re = quotemeta q|Store variable for 'foo' is not a reference!|;
    like(last_error(), qr/$re/, "Caught non-reference 'store' variable" );
}

### edge case tests ###
{   ### if key is not provided, and value is '', will P::C treat
    ### that correctly?
    my $tmpl = { foo => { default => '' } };
    my $rv   = check( $tmpl, {} );

    ok( $rv,                    "check() call with default = ''" );
    ok( exists $rv->{foo},      "   rv exists" );
    ok( defined $rv->{foo},     "   rv defined" );
    ok( !$rv->{foo},            "   rv false" );
    is( $rv->{foo}, '',         "   rv = '' " );
}

### big template test ###
{
    my $lastname;

    ### the template to check against ###
    my $tmpl = {
        firstname   => { required   => 1, defined => 1 },
        lastname    => { required   => 1, store => \$lastname },
        gender      => { required   => 1,
                         allow      => [qr/M/i, qr/F/i],
                    },
        married     => { allow      => [0,1] },
        age         => { default    => 21,
                         allow      => qr/^\d+$/,
                    },
        id_list     => { default        => [],
                         strict_type    => 1
                    },
        phone       => { allow          => sub { 1 if +shift } },
        bureau      => { default        => 'NSA',
                         no_override    => 1
                    },
    };

    ### the args to send ###
    my $try = {
        firstname   => 'joe',
        lastname    => 'jackson',
        gender      => 'M',
        married     => 1,
        age         => 21,
        id_list     => [1..3],
        phone       => '555-8844',
    };

    ### the rv we expect ###
    my $get = { %$try, bureau => 'NSA' };

    my $rv = check( $tmpl, $try );

    ok( $rv,                "elaborate check() call" );
    is_deeply( $rv, $get,   "   found provided values in rv" );
    is( $rv->{lastname}, $lastname,
                            "   found provided values in rv" );
}

### $Params::Check::CALLER_DEPTH test
{
    sub wrapper { check  ( @_ ) };
    sub inner   { wrapper( @_ ) };
    sub outer   { inner  ( @_ ) };
    outer( { dummy => { required => 1 }}, {} );

    like( last_error, qr/for .*::wrapper by .*::inner$/,
                            "wrong caller without CALLER_DEPTH" );

    local $Params::Check::CALLER_DEPTH = 1;
    outer( { dummy => { required => 1 }}, {} );

    like( last_error, qr/for .*::inner by .*::outer$/,
                            "right caller with CALLER_DEPTH" );
}

### test: #23824: Bug concerning the loss of the last_error
### message when checking recursively.
{   ok( 1,                      "Test last_error() on recursive check() call" );

    ### allow sub to call
    my $clear   = sub { check( {}, {} ) if shift; 1; };

    ### recursively call check() or not?
    for my $recurse ( 0, 1 ) {

        check(
            { a => { defined => 1 },
              b => { allow   => sub { $clear->( $recurse ) } },
            },
            { a => undef, b => undef }
        );

        ok( last_error(),       "   last_error() with recurse: $recurse" );
    }
}

