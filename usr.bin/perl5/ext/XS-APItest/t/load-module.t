#!perl -w
use strict;

# Test the load_module() core API function.
#
# Note that this function can be passed arbitrary and illegal module
# names which would already have been caught if a require statement had
# been compiled. So check that load_module() can catch such bad things.

use Test::More;
use XS::APItest;

# This isn't complete yet. In particular, we don't test import lists, or
# the other flags. But it's better than nothing.

is($INC{'less.pm'}, undef, "less isn't loaded");
load_module(PERL_LOADMOD_NOIMPORT, 'less');
like($INC{'less.pm'}, qr!(?:\A|/)lib/less\.pm\z!, "less is now loaded");

delete $INC{'less.pm'};
delete $::{'less::'};

is(eval { load_module(PERL_LOADMOD_NOIMPORT, 'less', 1); 1}, undef,
   "expect load_module() to fail");
like($@, qr/less version 1 required--this is only version 0\./,
     'with the correct error message');

is(eval { load_module(PERL_LOADMOD_NOIMPORT, 'less', 0.03); 1}, 1,
   "expect load_module() not to fail");

#
# Check for illegal module names

for (["", qr!\ABareword in require maps to empty filename!],
    ["::", qr!\ABareword in require must not start with a double-colon: "::"!],
    ["::::", qr!\ABareword in require must not start with a double-colon: "::::"!],
    ["::/", qr!\ABareword in require must not start with a double-colon: "::/!],
    ["/", qr!\ABareword in require maps to disallowed filename "/\.pm"!],
    ["::/WOOSH", qr!\ABareword in require must not start with a double-colon: "::/WOOSH!],
    [".WOOSH", qr!\ABareword in require maps to disallowed filename "\.WOOSH\.pm"!],
    ["::.WOOSH", qr!\ABareword in require must not start with a double-colon: "::.WOOSH!],
    ["WOOSH::.sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH::.sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH/.sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH/..sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH/../sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH::..::sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH::.::sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH::./sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH/./sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH/.::sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH/..::sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH::../sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH::../..::sock", qr!\ABareword in require contains "/\."!],
    ["WOOSH\0sock", qr!\ACan't locate WOOSH\\0sock.pm:!],
    )
{
    my ($module, $error) = @$_;
    my $module2 = $module; # load_module mangles its first argument
    no warnings 'syscalls';
    is(eval { load_module(PERL_LOADMOD_NOIMPORT, $module); 1}, undef,
       "expect load_module() for '$module2' to fail");
    like($@, $error, "check expected error for $module2");
}

done_testing();
