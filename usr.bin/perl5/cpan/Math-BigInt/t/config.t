# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 72;

# test whether Math::BigInt->config() and Math::BigFloat->config() work

use Math::BigInt lib => 'Calc';
use Math::BigFloat;

my $mbi = 'Math::BigInt';
my $mbf = 'Math::BigFloat';

my @defaults =
  ([ 'lib',         'Math::BigInt::Calc'           ],
   [ 'lib_version', $Math::BigInt::Calc::VERSION, ],
   [ 'upgrade',     undef,  ],
   [ 'div_scale',   40,     ],
   [ 'precision',   undef,  ],
   [ 'accuracy',    undef,  ],
   [ 'round_mode',  'even', ],
   [ 'trap_nan',    0,      ],
   [ 'trap_inf',    0,      ]);

##############################################################################
# Math::BigInt

{
    can_ok($mbi, 'config');

    my @table = @defaults;
    unshift @table, ['class', $mbi ];

    # Test getting via the new-style $class->($key):

    for (my $i = 0 ; $i <= $#table ; ++ $i) {
        my $key = $table[$i][0];
        my $val = $table[$i][1];
        is($mbi->config($key), $val, qq|$mbi->config("$key")|);
    }

    # Test getting via the old-style $class->()->{$key}, which is still
    # supported:

    my $cfg = $mbi->config();
    is(ref($cfg), 'HASH', 'ref() of output from $mbi->config()');

    for (my $i = 0 ; $i <= $#table ; ++ $i) {
        my $key = $table[$i][0];
        my $val = $table[$i][1];
        is($cfg->{$key}, $val, qq|$mbi->config()->{$key}|);
    }

    # can set via hash ref?
    $cfg = $mbi->config({ trap_nan => 1 });
    is($cfg->{trap_nan}, 1, 'can set "trap_nan" via hash ref');

    # reset for later
    $mbi->config(trap_nan => 0);
}

##############################################################################
# Math::BigFloat

{
    can_ok($mbf, 'config');

    my @table = @defaults;
    unshift @table, ['class', $mbf ];

    # Test getting via the new-style $class->($key):

    for (my $i = 0 ; $i <= $#table ; ++ $i) {
        my $key = $table[$i][0];
        my $val = $table[$i][1];
        is($mbf->config($key), $val, qq|$mbf->config("$key")|);
    }

    # Test getting via the old-style $class->()->{$key}, which is still
    # supported:

    my $cfg = $mbf->config();
    is(ref($cfg), 'HASH', 'ref() of output from $mbf->config()');

    for (my $i = 0 ; $i <= $#table ; ++ $i) {
        my $key = $table[$i][0];
        my $val = $table[$i][1];
        is($cfg->{$key}, $val, qq|$mbf->config()->{$key}|);
    }

    # can set via hash ref?
    $cfg = $mbf->config({ trap_nan => 1 });
    is($cfg->{trap_nan}, 1, 'can set "trap_nan" via hash ref');

    # reset for later
    $mbf->config(trap_nan => 0);
}

##############################################################################
# test setting values

my $test = {
    trap_nan   => 1,
    trap_inf   => 1,
    accuracy   => 2,
    precision  => 3,
    round_mode => 'zero',
    div_scale  => '100',
    upgrade    => 'Math::BigInt::SomeClass',
    downgrade  => 'Math::BigInt::SomeClass',
};

my $cfg;

foreach my $key (keys %$test) {

    # see if setting in MBI works
    eval { $mbi->config($key => $test->{$key}); };
    $cfg = $mbi->config();
    is("$key = $cfg->{$key}", "$key = $test->{$key}", "$key = $test->{$key}");
    $cfg = $mbf->config();

    # see if setting it in MBI leaves MBF alone
    ok(($cfg->{$key} || 0) ne $test->{$key},
       "$key ne \$cfg->{$key}");

    # see if setting in MBF works
    eval { $mbf->config($key => $test->{$key}); };
    $cfg = $mbf->config();
    is("$key = $cfg->{$key}", "$key = $test->{$key}", "$key = $test->{$key}");
}

##############################################################################
# test setting illegal keys (should croak)

eval { $mbi->config('some_garbage' => 1); };
like($@,
     qr/ ^ Illegal \s+ key\(s\) \s+ 'some_garbage' \s+ passed \s+ to \s+
         Math::BigInt->config\(\) \s+ at
       /x,
     'Passing invalid key to Math::BigInt->config() causes an error.');

eval { $mbf->config('some_garbage' => 1); };
like($@,
     qr/ ^ Illegal \s+ key\(s\) \s+ 'some_garbage' \s+ passed \s+ to \s+
         Math::BigFloat->config\(\) \s+ at
       /x,
     'Passing invalid key to Math::BigFloat->config() causes an error.');
