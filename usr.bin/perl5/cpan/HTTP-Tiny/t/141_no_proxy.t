#!perl
use strict;
use warnings;

use Test::More 0.88;

use HTTP::Tiny;

# blank slate for testing
delete $ENV{no_proxy};

{
    my $c = HTTP::Tiny->new();
    is_deeply( $c->no_proxy, [], "no no_proxy given" );
}

my @cases = (
    #<<< No perltidy
    {
        no_proxy => [
            undef,
            [],
        ],
        expect => [],
    },
    {
        no_proxy => [
            "localhost",
            ["localhost"],
        ],
        expect => ["localhost"],
    },
    {
        no_proxy => [
            "localhost,example.com",
            "localhost, example.com",
            [qw/localhost example.com/]
        ],
        expect   => [ "localhost", "example.com" ],
    },
    #>>>
);

for my $c (@cases) {
    for my $no_proxy ( @{ $c->{no_proxy} } ) {
        my $label =
           !defined $no_proxy ? 'undef'
          : ref $no_proxy     ? "[qw/@$no_proxy/]"
          :                     "'$no_proxy'";

        # Can't test environment with array ref (ENV stringifies in new perls)
        if ( ref $no_proxy ) {
            my $ht = HTTP::Tiny->new( no_proxy => $no_proxy );
            is_deeply( $ht->no_proxy, $c->{expect}, "new(no_proxy => $label)" );
        }
        else {
            {
                no warnings 'uninitialized';
                local $ENV{no_proxy} = $no_proxy;
                my $ht = HTTP::Tiny->new();
                is_deeply( $ht->no_proxy, $c->{expect}, "\$ENV{no_proxy} = $label" );
            }
            {
                local $ENV{no_proxy} = "Shouldnt,see,this";
                my $ht = HTTP::Tiny->new( no_proxy => $no_proxy );
                is_deeply( $ht->no_proxy, $c->{expect},
                    "new(no_proxy => $label) versus other \$ENV{no_proxy}" );
            }
        }
    }
}

done_testing();
