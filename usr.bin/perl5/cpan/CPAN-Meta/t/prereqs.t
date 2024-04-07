use strict;
use warnings;
use Test::More 0.88;

use CPAN::Meta::Prereqs;

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_BACKEND};
delete $ENV{CPAN_META_JSON_DECODER};

my $prereq_struct = {
  runtime => {
    requires => {
      'Config' => 0,
      'Cwd'    => 0,
      'Data::Dumper' => 0,
      'ExtUtils::Install' => 0,
      'File::Basename' => 0,
      'File::Compare'  => 0,
      'File::Copy' => 0,
      'File::Find' => 0,
      'File::Path' => 0,
      'File::Spec' => 0,
      'IO::File'   => 0,
      'perl'       => '5.005_03',
    },
    recommends => {
      'Archive::Tar' => '1.00',
      'ExtUtils::Install' => 0.3,
      'ExtUtils::ParseXS' => 2.02,
      'Pod::Text' => 0,
      'YAML' => 0.35,
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
    requires => {
      'JSON::PP' => '2.34',
    },
    x_type => {
      'POSIX' => '1.23',
    },
  },
};

my $prereq = CPAN::Meta::Prereqs->new($prereq_struct);

isa_ok($prereq, 'CPAN::Meta::Prereqs');

is_deeply($prereq->as_string_hash, $prereq_struct, "round-trip okay");

{
  my $req = $prereq->requirements_for(qw(runtime requires));
  my @req_mod = $req->required_modules;

  ok(
    (grep { 'Cwd' eq $_ } @req_mod),
    "we got the runtime requirements",
  );

  ok(
    (! grep { 'YAML' eq $_ } @req_mod),
    "...but not the runtime recommendations",
  );

  ok(
    (! grep { 'Test' eq $_ } @req_mod),
    "...nor the build requirements",
  );
}

{
  my $req = $prereq->requirements_for(qw(runtime requires));
  my $rec = $prereq->requirements_for(qw(runtime recommends));

  my $merged = $req->clone->add_requirements($rec);

  my @req_mod = $merged->required_modules;

  ok(
    (grep { 'Cwd' eq $_ } @req_mod),
    "we got the runtime requirements",
  );

  ok(
    (grep { 'YAML' eq $_ } @req_mod),
    "...and the runtime recommendations",
  );

  ok(
    (! grep { 'Test' eq $_ } @req_mod),
    "...but not the build requirements",
  );

}

{
  my $req = $prereq->requirements_for(qw(runtime suggests));
  my @req_mod = $req->required_modules;

  is(@req_mod, 0, "empty set of runtime/suggests requirements");
}

{
  my $req = $prereq->requirements_for(qw(develop suggests));
  my @req_mod = $req->required_modules;

  is(@req_mod, 0, "empty set of develop/suggests requirements");
}

{
  my $new_prereq = CPAN::Meta::Prereqs->new;

  $new_prereq
    ->requirements_for(qw(runtime requires))
    ->add_minimum(Foo => '1.000');

  $new_prereq
    ->requirements_for(qw(runtime requires))
    ->add_minimum(Bar => '2.976');

  $new_prereq
    ->requirements_for(qw(test requires))
    ->add_minimum(Baz => '3.1416');

  $new_prereq
    ->requirements_for(qw(build recommends))
    ->add_minimum(Bar => '3.000');

  my $expect = {
      runtime => { requires => { Foo => '1.000', Bar => '2.976' } },
      test => { requires => { Baz => '3.1416' } },
      build => { recommends => { Bar => '3.000' } },
  };

  is_deeply(
    $new_prereq->as_string_hash,
    $expect,
    'we can accumulate new requirements on a prereq object',
  );

  my $merged_requires = {
      Foo => '1.000',
      Bar => '2.976',
      Baz => '3.1416',
  };

  my $merged_all = {
      Foo => '1.000',
      Bar => '3.000',
      Baz => '3.1416',
  };

  is_deeply(
    $new_prereq->merged_requirements(
        [qw/runtime test build/], [qw/requires/]
    )->as_string_hash,
    $merged_requires,
    "we can merge requirements for phases/types"
  );

  is_deeply(
    $new_prereq->merged_requirements->as_string_hash,
    $merged_all,
    "default merging is runtime/build/test for requires/recommends"
  );
}

done_testing;
# vim: ts=2 sts=2 sw=2 et :
