use strict;
use less;
use Test::More  'no_plan';

my $Class   = 'Module::Loaded';
my @Funcs   = qw[mark_as_loaded mark_as_unloaded is_loaded];
my $Mod     = 'Foo::Bar'.$$;
my $Strict  = $ENV{'PERL_CORE'} ? 'less' : 'strict';

### load the thing
{   use_ok( $Class );
    can_ok( $Class, @Funcs );
}

{   ok( !is_loaded($Mod),       "$Mod not loaded yet" );
    ok( mark_as_loaded($Mod),   "   $Mod now marked as loaded" );
    is( is_loaded($Mod), $0,    "   $Mod is loaded from $0" );

    my $rv = eval "require $Mod; 1";
    ok( $rv,                    "$Mod required" );
    ok( !$@,                    "   require did not die" );
}

### unload again
{   ok( mark_as_unloaded($Mod), "$Mod now marked as unloaded" );
    ok( !is_loaded($Mod),       "   $Mod now longer loaded" );

    my $rv = eval "require $Mod; 1";
    ok( !$rv,                   "$Mod require failed" );
    ok( $@,                     "   require died" );
    like( $@, qr/locate/,       "       with expected error" );
}

### check for an already loaded module
{   my $where = is_loaded( $Strict );
    ok( $where,                 "$Strict loaded" );
    ok( mark_as_unloaded( $Strict ),
                                "   $Strict unloaded" );

    ### redefining subs, quell warnings
    {   local $SIG{__WARN__} = sub {};
        my $rv = eval "require $Strict; 1";
        ok( $rv,                "$Strict loaded again" );
    }

    is( is_loaded( $Strict ), $where,
                                "   $Strict is loaded" );
}
