use strict; use warnings;
use Memoize;
use Test::More tests => 11;

sub n_null { '' }

{ my $I = 0;
  sub n_diff { $I++ }
}

{ my $I = 0;
  sub a1 { $I++; "$_[0]-$I"  }
  my $J = 0;
  sub a2 { $J++; "$_[0]-$J"  }
  my $K = 0;
  sub a3 { $K++; "$_[0]-$K"  }
}

my $a_normal =  memoize('a1', INSTALL => undef);
my $a_nomemo =  memoize('a2', INSTALL => undef, NORMALIZER => 'n_diff');
my $a_allmemo = memoize('a3', INSTALL => undef, NORMALIZER => 'n_null');

my @ARGS;
@ARGS = (1, 2, 3, 2, 1);

is_deeply [map $a_normal->($_),  @ARGS], [qw(1-1 2-2 3-3 2-2 1-1)], 'no normalizer';
is_deeply [map $a_nomemo->($_),  @ARGS], [qw(1-1 2-2 3-3 2-4 1-5)], 'n_diff';
is_deeply [map $a_allmemo->($_), @ARGS], [qw(1-1 1-1 1-1 1-1 1-1)], 'n_null';

# Test fully-qualified name and installation
my $COUNT;
$COUNT = 0;
sub parity { $COUNT++; $_[0] % 2 }
sub parnorm { $_[0] % 2 }
memoize('parity', NORMALIZER =>  'main::parnorm');
is_deeply [map parity($_), @ARGS], [qw(1 0 1 0 1)], 'parity normalizer';
is $COUNT, 2, '... with the expected number of calls';

# Test normalization with reference to normalizer function
$COUNT = 0;
sub par2 { $COUNT++; $_[0] % 2 }
memoize('par2', NORMALIZER =>  \&parnorm);
is_deeply [map par2($_), @ARGS], [qw(1 0 1 0 1)], '... also installable by coderef';
is $COUNT, 2, '... still with the expected number of calls';

$COUNT = 0;
sub count_uninitialized { $COUNT += join('', @_) =~ /\AUse of uninitialized value / }
my $war1 = memoize(sub {1}, NORMALIZER => sub {undef});
{ local $SIG{__WARN__} = \&count_uninitialized; $war1->() }
is $COUNT, 0, 'no warning when normalizer returns undef';

# Context propagated correctly to normalizer?
sub n {
  my $which = wantarray ? 'list' : 'scalar';
  local $Test::Builder::Level = $Test::Builder::Level + 2;
  is $_[0], $which, "$which context propagates properly";
}
sub f { 1 }
memoize('f', NORMALIZER => 'n');
my $s = f 'scalar';
my @a = f 'list';

sub args { scalar @_ }
sub null_args { join chr(28), splice @_ }
memoize('args', NORMALIZER => 'null_args');
ok args(1), 'original @_ is protected from normalizer';
