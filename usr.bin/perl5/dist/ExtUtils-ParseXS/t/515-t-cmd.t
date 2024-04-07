#!/usr/bin/perl
use strict;
use warnings;

# tests for the quick-n-dirty interface for XS inclusion

use Test::More tests => 6;
use File::Spec;
use ExtUtils::Typemaps::Cmd;

my $datadir = -d 't' ? File::Spec->catdir(qw/t data/) : 'data';
my $libdir = -d 't' ? File::Spec->catdir(qw/t lib/) : 'lib';

unshift @INC, $libdir;

sub slurp {
  my $file = shift;
  open my $fh, '<', $file
    or die "Cannot open file '$file' for reading: $!";
  local $/ = undef;
  return <$fh>;
}

sub permute (&@) {
  my $code = shift;
  my @idx = 0..$#_;
  while ( $code->(@_[@idx]) ) {
    my $p = $#idx;
    --$p while $idx[$p-1] > $idx[$p];
    my $q = $p or return;
    push @idx, reverse splice @idx, $p;
    ++$q while $idx[$p-1] > $idx[$q];
    @idx[$p-1,$q]=@idx[$q,$p-1];
  }
}


SCOPE: {
  no warnings 'once';
  ok(defined(*embeddable_typemap{CODE}), "function exported");
}

my $start = "TYPEMAP: <<END_TYPEMAP;\n";
my $end = "\nEND_TYPEMAP\n";
is(
  embeddable_typemap(),
  "${start}TYPEMAP\n$end",
  "empty call to embeddable_typemap"
);

my $typemap_file = File::Spec->catfile($datadir, "simple.typemap");
is(
  embeddable_typemap($typemap_file),
  $start . slurp($typemap_file) . $end,
  "embeddable typemap from file"
);

my $foo_content = <<HERE;
TYPEMAP
myfoo*	T_PV
HERE
is(
  embeddable_typemap("TypemapTest::Foo"),
  "$start$foo_content$end",
  "embeddable typemap from full module name"
);


my $test_content = <<HERE;
TYPEMAP
mytype*	T_SV
HERE
is(
  embeddable_typemap("Test"),
  "$start$test_content$end",
  "embeddable typemap from relative module name"
);

SCOPE: {
  my $combined = embeddable_typemap("Test", "TypemapTest::Foo");
  my @lines = (
    'myfoo*	T_PV',
    'mytype*	T_SV',
  );
  my @exp = map {"TYPEMAP\n" . join("\n", @$_) . "\n"}
            (\@lines, [reverse @lines]);
  ok(scalar(grep "$start$_$end" eq $combined, @exp), "combined both modules")
    or note("Actual output: '$combined'");
}

# in theory, we should test
# embeddable_typemap($typemap_file, "Test", "TypemapTest::Foo"),
# but I can't be bothered.
