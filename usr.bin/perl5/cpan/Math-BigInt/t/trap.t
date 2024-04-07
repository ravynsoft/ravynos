# -*- mode: perl; -*-

# test that config ( trap_nan => 1, trap_inf => 1) really works/dies

use strict;
use warnings;

use Test::More tests => 43;

use Math::BigInt;
use Math::BigFloat;

my $mbi = 'Math::BigInt';
my $mbf = 'Math::BigFloat';
my ($cfg, $x);

foreach my $class ($mbi, $mbf) {
    # can do and defaults are okay?
    ok($class->can('config'), 'can config()');
    is($class->config("trap_nan"), 0, 'trap_nan defaults to 0');
    is($class->config("trap_inf"), 0, 'trap_inf defaults to 0');

    # can set?
    $cfg = $class->config( trap_nan => 1 );
    is($cfg->{trap_nan}, 1, 'trap_nan now true');

    # also test that new() still works normally
    eval ("\$x = \$class->new('42'); \$x->bnan();");
    like($@, qr/^Tried to create/, 'died');
    is($x, 42, '$x after new() never modified');

    # can reset?
    $cfg = $class->config( trap_nan => 0 );
    is($cfg->{trap_nan}, 0, 'trap_nan disabled');

    # can set?
    $cfg = $class->config( trap_inf => 1 );
    is($cfg->{trap_inf}, 1, 'trap_inf enabled');

    eval ("\$x = \$class->new('4711'); \$x->binf();");
    like($@, qr/^Tried to create/, 'died');
    is($x, 4711, '$x after new() never modified');

    eval ("\$x = \$class->new('inf');");
    like($@, qr/^Tried to create/, 'died');
    is($x, 4711, '$x after new() never modified');

    eval ("\$x = \$class->new('-inf');");
    like($@, qr/^Tried to create/, 'died');
    is($x, 4711, '$x after new() never modified');

    # +$x/0 => +inf
    eval ("\$x = \$class->new('4711'); \$x->bdiv(0);");
    like($@, qr/^Tried to create/, 'died');
    is($x, 4711, '$x after new() never modified');

    # -$x/0 => -inf
    eval ("\$x = \$class->new('-0815'); \$x->bdiv(0);");
    like($@, qr/^Tried to create/, 'died');
    is($x, '-815', '$x after new not modified');

    $cfg = $class->config( trap_nan => 1 );
    # 0/0 => NaN
    eval ("\$x = \$class->new('0'); \$x->bdiv(0);");
    like($@, qr/^Tried to create/, 'died');
    is($x, '0', '$x after new not modified');
}

##############################################################################
# Math::BigInt

$x = Math::BigInt->new(2);
eval ("\$x = \$mbi->new('0.1');");
is($x, 2, 'never modified since it dies');
eval ("\$x = \$mbi->new('0a.1');");
is($x, 2, 'never modified since it dies');

##############################################################################
# Math::BigFloat

$x = Math::BigFloat->new(2);
eval ("\$x = \$mbf->new('0.1a');");
is($x, 2, 'never modified since it dies');

# all tests done
