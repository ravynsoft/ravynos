use strict;
use warnings;
use Test2::Tools::Tiny;
use Test2::API qw( context_do );

$SIG{__WARN__} = sub {
    context_do { shift->throw("oops\n"); }
    $_[0];
};

my $array_var = [];
eval { warn "trigger warning" };
my $err = $@;
like(
    $err,
    qr/oops/,
    "Got expected error"
);

done_testing();
