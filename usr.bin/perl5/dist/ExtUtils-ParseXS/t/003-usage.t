#!/usr/bin/perl

use strict;
use Test::More;
use Config;
use DynaLoader;
use ExtUtils::CBuilder;

if ( $] < 5.008 ) {
  plan skip_all => "INTERFACE keyword support broken before 5.8";
}
else {
  plan tests => 24;
}

my ($source_file, $obj_file, $lib_file, $module);

require_ok( 'ExtUtils::ParseXS' );

chdir('t') if -d 't';
push @INC, '.';

use Carp; #$SIG{__WARN__} = \&Carp::cluck;

# See the comments about this in 001-basics.t
@INC = map { File::Spec->rel2abs($_) } @INC;

#########################

$source_file = 'XSUsage.c';

# Try sending to file
ExtUtils::ParseXS->process_file(filename => 'XSUsage.xs', output => $source_file);
ok -e $source_file, "Create an output file";

# TEST doesn't like extraneous output
my $quiet = $ENV{PERL_CORE} && !$ENV{HARNESS_ACTIVE};

# Try to compile the file!  Don't get too fancy, though.
my $b = ExtUtils::CBuilder->new(quiet => $quiet);

SKIP: {
  skip "no compiler available", 2
    if ! $b->have_compiler;
  $module = 'XSUsage';

  $obj_file = $b->compile( source => $source_file );
  ok $obj_file;
  ok -e $obj_file, "Make sure $obj_file exists";
}
SKIP: {
  skip "no dynamic loading", 20 
    if !$b->have_compiler || !$Config{usedl};

  $lib_file = $b->link( objects => $obj_file, module_name => $module );
  ok $lib_file;
  ok -e $lib_file, "Make sure $lib_file exists";

  eval {require XSUsage};
  is $@, '';

  # The real tests here - for each way of calling the functions, call with the
  # wrong number of arguments and check the Usage line is what we expect

  eval { XSUsage::one(1) };
  ok $@;
  ok $@ =~ /^Usage: XSUsage::one/;

  eval { XSUsage::two(1) };
  ok $@;
  ok $@ =~ /^Usage: XSUsage::two/;

  eval { XSUsage::two_x(1) };
  ok $@;
  ok $@ =~ /^Usage: XSUsage::two_x/;

  eval { FOO::two(1) };
  ok $@;
  ok $@ =~ /^Usage: FOO::two/;

  eval { XSUsage::three(1) };
  ok $@;
  ok $@ =~ /^Usage: XSUsage::three/;

  eval { XSUsage::four(1) };
  ok !$@;

  eval { XSUsage::five() };
  ok $@;
  ok $@ =~ /^Usage: XSUsage::five/;

  eval { XSUsage::six() };
  ok !$@;

  eval { XSUsage::six(1) };
  ok !$@;

  eval { XSUsage::six(1,2) };
  ok $@;
  ok $@ =~ /^Usage: XSUsage::six/;

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

