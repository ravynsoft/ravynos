#!/usr/bin/perl -w

# Things like the CPAN shell rely on the "MakeMaker Parameters" section of the
# Makefile to learn a module's dependencies so we'd damn well better test it.

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;

use ExtUtils::MakeMaker;
use Test::More tests => 6;

my $mm = bless {}, "MM";

sub process_cmp {
  my ($args, $expected, $label) = @_;
  my $got = join '',
    map "$_\n", $mm->_MakeMaker_Parameters_section($args || ());
  $got =~ s/^#\s*MakeMaker Parameters:\n+//;
  is $got, $expected, $label;
}

process_cmp undef, '', 'nothing';
process_cmp { NAME => "Foo" }, <<'EXPECT', "name only";
#     NAME => q[Foo]
EXPECT
process_cmp
  { NAME => "Foo", PREREQ_PM => { "Foo::Bar" => 0 } }, <<'EXPECT', "PREREQ v0";
#     NAME => q[Foo]
#     PREREQ_PM => { Foo::Bar=>q[0] }
EXPECT
process_cmp
  { NAME => "Foo", PREREQ_PM => { "Foo::Bar" => 1.23 } },
  <<'EXPECT', "PREREQ v-non-0";
#     NAME => q[Foo]
#     PREREQ_PM => { Foo::Bar=>q[1.23] }
EXPECT

process_cmp
  {
    NAME                => "Foo",
    PREREQ_PM           => { "Foo::Bar" => 1.23 },
    BUILD_REQUIRES      => { "Baz"      => 0.12 },
  },
  <<'EXPECT', "BUILD_REQUIRES";
#     BUILD_REQUIRES => { Baz=>q[0.12] }
#     NAME => q[Foo]
#     PREREQ_PM => { Baz=>q[0.12], Foo::Bar=>q[1.23] }
EXPECT

process_cmp
  {
    NAME                => "Foo",
    PREREQ_PM           => { "Foo::Bar" => 1.23, Long => 1.45, Short => 0 },
    BUILD_REQUIRES      => { "Baz"      => 0.12 },
  },
  <<'EXPECT', "ensure sorting";
#     BUILD_REQUIRES => { Baz=>q[0.12] }
#     NAME => q[Foo]
#     PREREQ_PM => { Baz=>q[0.12], Foo::Bar=>q[1.23], Long=>q[1.45], Short=>q[0] }
EXPECT
