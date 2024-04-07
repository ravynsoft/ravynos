use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta::Prereqs;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

my $prereq_struct_1 = {
  runtime => {
    requires => {
      'Config' => 0,
      'Cwd'    => 0,
      'perl'   => '5.005_03',
    },
    recommends => {
      'Pod::Text' => 0,
      'YAML'      => 0.35,
    },
  },
  build => {
    requires => {
      'Test' => 0,
    },
    x_type => {
      'Config' => 1,
    },
  },
  x_phase => {
    x_type => {
      'POSIX' => '1.23',
    },
  },
};

my $prereq_1 = CPAN::Meta::Prereqs->new($prereq_struct_1);

isa_ok($prereq_1, 'CPAN::Meta::Prereqs', 'first prereq');

is_deeply($prereq_1->as_string_hash, $prereq_struct_1, '...and it round trips');

my $prereq_struct_2 = {
  develop => {
    requires => {
      'Dist::Mothra' => '1.230',
    },
    suggests => {
      'Blort::Blortex' => '== 10.20',
    },
  },
  runtime => {
    requires => {
      'Config' => 1,
      'perl'       => '< 6',
    },
  },
  build => {
    suggests => {
      'Module::Build::Bob' => '20100101',
    },
  },
  x_phase => {
    requires => {
      'JSON::PP' => '2.34',
    },
  },
};

my $prereq_2 = CPAN::Meta::Prereqs->new($prereq_struct_2);

isa_ok($prereq_2, 'CPAN::Meta::Prereqs', 'second prereq');

is_deeply($prereq_1->as_string_hash, $prereq_struct_1, '...and it round trips');

my $merged = $prereq_1->with_merged_prereqs($prereq_2);

my $want = {
  develop => {
    requires => {
      'Dist::Mothra' => '1.230',
    },
    suggests => {
      'Blort::Blortex' => '== 10.20',
    },
  },
  runtime => {
    requires => {
      'Config' => 1,
      'Cwd'    => 0,
      'perl'   => '>= 5.005_03, < 6',
    },
    recommends => {
      'Pod::Text' => 0,
      'YAML'      => 0.35,
    },
  },
  build => {
    requires => {
      'Test' => 0,
    },
    suggests => {
      'Module::Build::Bob' => '20100101',
    },
    x_type => {
      'Config' => 1,
    },
  },
  x_phase => {
    requires => {
      'JSON::PP' => '2.34',
    },
    x_type => {
      'POSIX' => '1.23',
    },
  },
};

is_deeply(
  $merged->as_string_hash,
  $want,
  "we get the right result of merging two prereqs",
);

is_deeply(
  $prereq_2->with_merged_prereqs($prereq_1)->as_string_hash,
  $want,
  "...and the merge works the same in reverse",
);

done_testing;
# vim: ts=2 sts=2 sw=2 et :
