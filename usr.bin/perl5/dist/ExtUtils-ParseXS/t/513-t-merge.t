#!/usr/bin/perl
use strict;
use warnings;

use Test::More tests => 19;
use ExtUtils::Typemaps;
use File::Spec;
use File::Temp;

my $datadir = -d 't' ? File::Spec->catdir(qw/t data/) : 'data';

sub slurp {
  my $file = shift;
  open my $fh, '<', $file
    or die "Cannot open file '$file' for reading: $!";
  local $/ = undef;
  return <$fh>;
}

my $first_typemap_file         = File::Spec->catfile($datadir, 'simple.typemap');
my $second_typemap_file        = File::Spec->catfile($datadir, 'other.typemap');
my $combined_typemap_file      = File::Spec->catfile($datadir, 'combined.typemap');
my $conflicting_typemap_file   = File::Spec->catfile($datadir, 'conflicting.typemap');
my $confl_replace_typemap_file = File::Spec->catfile($datadir, 'confl_repl.typemap');
my $confl_skip_typemap_file    = File::Spec->catfile($datadir, 'confl_skip.typemap');

# test merging two typemaps
SCOPE: {
  my $first = ExtUtils::Typemaps->new(file => $first_typemap_file);
  isa_ok($first, 'ExtUtils::Typemaps');
  my $second = ExtUtils::Typemaps->new(file => $second_typemap_file);
  isa_ok($second, 'ExtUtils::Typemaps');

  $first->merge(typemap => $second);

  is($first->as_string(), slurp($combined_typemap_file), "merging produces expected output");
}

# test merging a typemap from file
SCOPE: {
  my $first = ExtUtils::Typemaps->new(file => $first_typemap_file);
  isa_ok($first, 'ExtUtils::Typemaps');

  $first->merge(file => $second_typemap_file);

  is($first->as_string(), slurp($combined_typemap_file), "merging produces expected output");
}


# test merging a typemap as string
SCOPE: {
  my $first = ExtUtils::Typemaps->new(file => $first_typemap_file);
  isa_ok($first, 'ExtUtils::Typemaps');
  my $second_str = slurp($second_typemap_file);

  $first->add_string(string => $second_str);

  is($first->as_string(), slurp($combined_typemap_file), "merging (string) produces expected output");
}

# test merging a conflicting typemap without "replace"
SCOPE: {
  my $second = ExtUtils::Typemaps->new(file => $second_typemap_file);
  isa_ok($second, 'ExtUtils::Typemaps');
  my $conflict = ExtUtils::Typemaps->new(file => $conflicting_typemap_file);
  isa_ok($conflict, 'ExtUtils::Typemaps');

  ok(
    !eval {
      $second->merge(typemap => $conflict);
      1;
    },
    "Merging conflicting typemap croaks"
  );
  ok(
    $@ =~ /Multiple definition/,
    "Conflicting typemap error as expected"
  );
}

# test merging a conflicting typemap with "replace"
SCOPE: {
  my $second = ExtUtils::Typemaps->new(file => $second_typemap_file);
  isa_ok($second, 'ExtUtils::Typemaps');
  my $conflict = ExtUtils::Typemaps->new(file => $conflicting_typemap_file);
  isa_ok($conflict, 'ExtUtils::Typemaps');

  ok(
    eval {
      $second->merge(typemap => $conflict, replace => 1);
      1;
    },
    "Conflicting typemap merge with 'replace' doesn't croak"
  );

  is($second->as_string(), slurp($confl_replace_typemap_file), "merging (string) produces expected output");
}

# test merging a conflicting typemap file with "skip"
SCOPE: {
  my $second = ExtUtils::Typemaps->new(file => $second_typemap_file);
  isa_ok($second, 'ExtUtils::Typemaps');
  my $conflict = ExtUtils::Typemaps->new(file => $conflicting_typemap_file);
  isa_ok($conflict, 'ExtUtils::Typemaps');

  ok(
    eval {
      $second->merge(typemap => $conflict, skip => 1);
      1;
    },
    "Conflicting typemap merge with 'skip' doesn't croak"
  );

  is($second->as_string(), slurp($confl_skip_typemap_file), "merging (string) produces expected output");
}

