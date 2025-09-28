use strict;
use warnings;
use Test::More;

use XS::APItest;

our $sv = 'Moo';
my $foo = 'affe';
my $bar = 'tiger';

ok !mg_find_foo($sv), 'no foo magic yet';
ok !mg_find_bar($sv), 'no bar magic yet';

sv_magic_foo($sv, $foo);
is mg_find_foo($sv), $foo, 'foo magic attached';
ok !mg_find_bar($sv), '... but still no bar magic';

{
	local $sv = 'Emu';
	sv_magic_foo($sv, $foo);
	is mg_find_foo($sv), $foo, 'foo magic attached to localized value';
	ok !mg_find_bar($sv), '... but still no bar magic to localized value';
}

sv_magic_bar($sv, $bar);
is mg_find_foo($sv), $foo, 'foo magic still attached';
is mg_find_bar($sv), $bar, '... and bar magic is there too';

sv_unmagic_foo($sv);
ok !mg_find_foo($sv), 'foo magic removed';
is mg_find_bar($sv), $bar, '... but bar magic is still there';

sv_unmagic_bar($sv);
ok !mg_find_foo($sv), 'foo magic still removed';
ok !mg_find_bar($sv), '... and bar magic is removed too';

sv_magic_baz($sv, $bar);
is mg_find_baz($sv), $bar, 'baz magic attached';
ok !mg_find_bar($sv), '';
{
	local $sv = 'Emu';
	ok !mg_find_baz($sv), '';
}

is(test_get_vtbl(), 0, 'get_vtbl(-1) returns NULL');

eval { sv_magic(\!0, $foo) };
is $@, "", 'PERL_MAGIC_ext is permitted on read-only things';

# assigning to an array/hash with only set magic should call that magic

{
    my (@a, %h, $i);

    sv_magic_myset(\@a, $i);
    sv_magic_myset(\%h, $i);

    $i = 0;
    @a = (1,2);
    is($i, 2, "array with set magic");

    $i = 0;
    @a = ();
    is($i, 0, "array () with set magic");

    {
        local $TODO = "HVs don't call set magic - not sure if should";

        $i = 0;
        %h = qw(a 1 b 2);
        is($i, 4, "hash with set magic");
    }

    $i = 0;
    %h = qw();
    is($i, 0, "hash () with set magic");
}

{
    # check if set magic triggered by av_store() via aassign results in
    # unreferenced scalars being freed. IOW, results in a double store
    # without a corresponding refcount bump. If things work properly this
    # should not warn. If there is an issue it will.
    my @warn;
    local $SIG{__WARN__}= sub { push @warn, $_[0] };
    {
        my (@a, $i);
        sv_magic_myset_dies(\@a, $i);
        eval {
            $i = 0;
            @a = (1);
        };
    }
    is(0+@warn, 0,
        "If AV set magic dies via aassign it should not warn about double free");
    @warn = ();
    {
        my (@a, $i, $j);
        sv_magic_myset_dies(\@a, $i);
        eval {
            $j = "blorp";
            my_av_store(\@a,0,$j);
        };

        # Evaluate this boolean as a separate statement, so the two
        # temporary \ refs are freed before we start comparing reference
        # counts
        my $is_same_SV = \$a[0] == \$j;

        if ($is_same_SV) {
            # in this case we expect to have 2 refcounts,
            # one from $a[0] and one from $j itself.
            is( sv_refcnt($j), 2,
                "\$a[0] is \$j, so refcount(\$j) should be 2");
        } else {
            # Note this branch isn't exercised. Whether by design
            # or not. I leave it here because it is a possible valid
            # outcome. It is marked TODO so if we start going down
            # this path we do so knowingly.
            diag "av_store has changed behavior - please review this test";
            TODO:{
                local $TODO = "av_store bug stores even if it dies during magic";
                # in this case we expect to have only 1 refcount,
                # from $j itself.
                is( sv_refcnt($j), 1,
                    "\$a[0] is not \$j, so refcount(\$j) should be 1");
            }
        }
    }
    is(0+@warn, 0,
        "AV set magic that dies via av_store should not warn about double free");
}

done_testing;
