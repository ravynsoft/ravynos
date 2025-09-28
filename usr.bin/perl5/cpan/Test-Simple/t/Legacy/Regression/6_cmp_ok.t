use Test::More;

use Test2::API qw/intercept/;

my $events = intercept {
    local $SIG{__WARN__} = sub { 1 };
    my $foo = undef;
    cmp_ok($foo, "ne", "");
};

is($events->[-1]->message, <<EOT, "Got useful diag");
    undef
        ne
    ''
EOT

done_testing;
