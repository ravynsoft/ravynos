use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta::Prereqs;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

sub dies_ok (&@) {
  my ($code, $qr, $comment) = @_;

  if (eval { $code->(); 1 }) {
    fail("$comment: did not die");
  } else {
    like($@, $qr, $comment);
  }
}

my $prereqs_struct = {
  runtime => {
    requires => {
      'Config' => '1.234',
      'Cwd'    => '876.5',
      'IO::File'   => 0,
      'perl'       => '5.005_03',
    },
    recommends => {
      'Pod::Text' => 0,
      'YAML'      => '0.35',
    },
  },
  build => {
    requires => {
      'Test' => 0,
    },
  }
};

my $prereqs = CPAN::Meta::Prereqs->new($prereqs_struct);

isa_ok($prereqs, 'CPAN::Meta::Prereqs');

$prereqs->finalize;

ok($prereqs->is_finalized, 'cloned obj is not finalized');

is_deeply($prereqs->as_string_hash, $prereqs_struct, '...and still round-trip');

$prereqs->requirements_for(qw(runtime requires))->add_minimum(Cwd => 10);

pass('...we can add a minimum if it has no effect');

dies_ok
  { $prereqs->requirements_for(qw(runtime requires))->add_minimum(Cwd => 1000) }
  qr{finalized req},
  '...but we die if it would alter a finalized prereqs';

$prereqs->requirements_for(qw(develop suggests));

pass('...we can get a V:R object for a previously unconfigured phase');

dies_ok
  { $prereqs->requirements_for(qw(develop suggests))->add_minimum(Foo => 1) }
  qr{finalized req},
  '...but we die if we try to put anything in it';

my $clone = $prereqs->clone;

isa_ok($clone, 'CPAN::Meta::Prereqs', 'cloned prereqs obj');

ok(! $clone->is_finalized, 'cloned obj is not finalized');

is_deeply($clone->as_string_hash, $prereqs_struct, '...it still round-trips');

$clone->requirements_for(qw(runtime requires))->add_minimum(Cwd => 10);

pass('...we can add minimum if it has no effect');

$clone->requirements_for(qw(runtime requires))->add_minimum(Cwd => 1000);

pass('...or if it has an effect');

$clone->requirements_for(qw(develop suggests));

pass('...we can get a V:R object for a previously unconfigured phase');

$clone->requirements_for(qw(develop suggests))->add_minimum(Foo => 1);

pass('...and we can add stuff to it');

done_testing;
# vim: ts=2 sts=2 sw=2 et :
