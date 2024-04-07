#!/usr/bin/perl

use strict;
use warnings;
use Test::More;
use Config;
use DynaLoader;
use ExtUtils::CBuilder;
use attributes;
use overload;

plan tests => 33;

my ($source_file, $obj_file, $lib_file);

require_ok( 'ExtUtils::ParseXS' );
ExtUtils::ParseXS->import('process_file');

chdir 't' if -d 't';
push @INC, '.';

use Carp; #$SIG{__WARN__} = \&Carp::cluck;

# See the comments about this in 001-basics.t
@INC = map { File::Spec->rel2abs($_) } @INC;

#########################

$source_file = 'XSMore.c';

# Try sending to file
ExtUtils::ParseXS->process_file(
	filename => 'XSMore.xs',
	output   => $source_file,
);
ok -e $source_file, "Create an output file";

my $quiet = $ENV{PERL_CORE} && !$ENV{HARNESS_ACTIVE};
my $b = ExtUtils::CBuilder->new(quiet => $quiet);

SKIP: {
  skip "no compiler available", 2
    if ! $b->have_compiler;
  $obj_file = $b->compile( source => $source_file );
  ok $obj_file, "ExtUtils::CBuilder::compile() returned true value";
  ok -e $obj_file, "Make sure $obj_file exists";
}

SKIP: {
  skip "no dynamic loading", 29
    if !$b->have_compiler || !$Config{usedl};
  my $module = 'XSMore';
  $lib_file = $b->link( objects => $obj_file, module_name => $module );
  ok $lib_file, "ExtUtils::CBuilder::link() returned true value";
  ok -e $lib_file,  "Make sure $lib_file exists";

  eval{
    package XSMore;
    our $VERSION = 42;
    our $boot_ok;
    DynaLoader::bootstrap_inherit(__PACKAGE__, $VERSION); # VERSIONCHECK disabled

    sub new{ bless {}, shift }
  };
  is $@, '', "No error message recorded, as expected";
  is ExtUtils::ParseXS::report_error_count(), 0, 'ExtUtils::ParseXS::errors()';

  is $XSMore::boot_ok, 100, 'the BOOT keyword';

  ok XSMore::include_ok(), 'the INCLUDE keyword';
  is prototype(\&XSMore::include_ok), "", 'the PROTOTYPES keyword';

  is prototype(\&XSMore::prototype_ssa), '$$@', 'the PROTOTYPE keyword';

  is_deeply [attributes::get(\&XSMore::attr_method)], [qw(method)], 'the ATTRS keyword';
  is prototype(\&XSMore::attr_method), '$;@', 'ATTRS with prototype';

  is XSMore::return_1(), 1, 'the CASE keyword (1)';
  is XSMore::return_2(), 2, 'the CASE keyword (2)';
  is prototype(\&XSMore::return_1), "", 'ALIAS with prototype (1)';
  is prototype(\&XSMore::return_2), "", 'ALIAS with prototype (2)';

  is XSMore::arg_init(200), 200, 'argument init';

  ok overload::Overloaded(XSMore->new), 'the FALLBACK keyword';
  is abs(XSMore->new), 42, 'the OVERLOAD keyword';

  my $overload_sub_name = "XSMore::More::(+";
  is prototype(\&$overload_sub_name), "", 'OVERLOAD following prototyped xsub';

  my @a;
  XSMore::hook(\@a);
  is_deeply \@a, [qw(INIT CODE POSTCALL CLEANUP)], 'the INIT & POSTCALL & CLEANUP keywords';

  is_deeply [XSMore::outlist()], [ord('a'), ord('b')], 'the OUTLIST keyword';

  is_deeply [XSMore::outlist_bool("a", "b")], [ !0, "ab" ],
             "OUTLIST with a bool RETVAL";

  is_deeply [XSMore::outlist_int("c", "d")], [ 11, "cd" ],
             "OUTLIST with an int RETVAL";

  # eval so compile-time sees any prototype
  is_deeply [ eval 'XSMore::outlist()' ], [ord('a'), ord('b')], 'OUTLIST prototypes';

  is XSMore::len("foo"), 3, 'the length keyword';

  is XSMore::sum(5, 9), 14, 'the INCLUDE_COMMAND directive';

  # Tests for embedded typemaps
  is XSMore::typemaptest1(), 42, 'Simple embedded typemap works';
  is XSMore::typemaptest2(), 42, 'Simple embedded typemap works with funny end marker';
  is XSMore::typemaptest3(12, 13, 14), 12, 'Simple embedded typemap works for input, too';
  is XSMore::typemaptest6(5), 5, '<<END; (with semicolon) matches delimiter "END"';

  # Win32 needs to close the DLL before it can unlink it, but unfortunately
  # dl_unload_file was missing on Win32 prior to perl change #24679!
  if ($^O eq 'MSWin32' and defined &DynaLoader::dl_unload_file) {
    for (my $i = 0; $i < @DynaLoader::dl_modules; $i++) {
      if ($DynaLoader::dl_modules[$i] eq $module) {
        DynaLoader::dl_unload_file($DynaLoader::dl_librefs[$i]);
        last;
      }
    }
  }
}

unless ($ENV{PERL_NO_CLEANUP}) {
  for ( $obj_file, $lib_file, $source_file) {
    next unless defined $_;
    1 while unlink $_;
  }
}
