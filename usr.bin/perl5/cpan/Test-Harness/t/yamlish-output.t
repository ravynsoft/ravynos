#!/usr/bin/perl -wT

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 9;

use TAP::Parser::YAMLish::Writer;

my $out = [
    "---",
    "bill-to:",
    "  address:",
    "    city: \"Royal Oak\"",
    "    lines: \"458 Walkman Dr.\\nSuite #292\\n\"",
    "    postal: 48046",
    "    state: MI",
    "  family: Dumars",
    "  given: Chris",
    "comments: \"Late afternoon is best. Backup contact is Nancy Billsmer \@ 338-4338\\n\"",
    "date: 2001-01-23",
    "invoice: 34843",
    "product:",
    "  -",
    "    description: Basketball",
    "    price: 450.00",
    "    quantity: 4",
    "    sku: BL394D",
    "  -",
    "    description: \"Super Hoop\"",
    "    price: 2392.00",
    "    quantity: 1",
    "    sku: BL4438H",
    "tax: 251.42",
    "total: 4443.52",
    "...",
];

my $in = {
    'bill-to' => {
        'given'   => 'Chris',
        'address' => {
            'city'   => 'Royal Oak',
            'postal' => '48046',
            'lines'  => "458 Walkman Dr.\nSuite #292\n",
            'state'  => 'MI'
        },
        'family' => 'Dumars'
    },
    'invoice' => '34843',
    'date'    => '2001-01-23',
    'tax'     => '251.42',
    'product' => [
        {   'sku'         => 'BL394D',
            'quantity'    => '4',
            'price'       => '450.00',
            'description' => 'Basketball'
        },
        {   'sku'         => 'BL4438H',
            'quantity'    => '1',
            'price'       => '2392.00',
            'description' => 'Super Hoop'
        }
    ],
    'comments' =>
      "Late afternoon is best. Backup contact is Nancy Billsmer @ 338-4338\n",
    'total' => '4443.52'
};

my @buf1 = ();
my @buf2 = ();
my $buf3 = '';

my @destination = (
    {   name        => 'Array reference',
        destination => \@buf1,
        normalise   => sub { return \@buf1 },
    },
    {   name        => 'Closure',
        destination => sub { push @buf2, shift },
        normalise => sub { return \@buf2 },
    },
    {   name        => 'Scalar',
        destination => \$buf3,
        normalise   => sub {
            my @ar = split( /\n/, $buf3 );
            return \@ar;
        },
    },
);

for my $dest (@destination) {
    my $name = $dest->{name};
    ok my $yaml = TAP::Parser::YAMLish::Writer->new, "$name: Created";
    isa_ok $yaml, 'TAP::Parser::YAMLish::Writer';

    $yaml->write( $in, $dest->{destination} );
    my $got = $dest->{normalise}->();
    is_deeply $got, $out, "$name: Result matches";
}
