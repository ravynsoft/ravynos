#!/usr/bin/perl -w

use Test::More tests => 8;
use Symbol;
use Test::Builder;
use Test::Builder::Tester;

use strict;

# argh! now we need to test the thing we're testing.  Basically we need
# to pretty much reimplement the whole code again.  This is very
# annoying but can't be avoided.  And onward with the cut and paste

# My brain is melting.  My brain is melting.  ETOOMANYLAYERSOFTESTING

# create some private file handles
my $output_handle = gensym;
my $error_handle  = gensym;

# and tie them to this package
my $out = tie *$output_handle, "Test::Builder::Tester::Tie", "STDOUT";
my $err = tie *$error_handle,  "Test::Builder::Tester::Tie", "STDERR";

# ooooh, use the test suite
my $t = Test::Builder->new;

# remember the testing outputs
my $original_output_handle;
my $original_failure_handle;
my $original_todo_handle;
my $original_harness_env;
my $testing_num;

sub start_testing
{
    # remember what the handles were set to
    $original_output_handle  = $t->output();
    $original_failure_handle = $t->failure_output();
    $original_todo_handle    = $t->todo_output();
    $original_harness_env    = $ENV{HARNESS_ACTIVE};

    # switch out to our own handles
    $t->output($output_handle);
    $t->failure_output($error_handle);
    $t->todo_output($error_handle);

    $ENV{HARNESS_ACTIVE} = 0;

    # clear the expected list
    $out->reset();
    $err->reset();

    # remember that we're testing
    $testing_num = $t->current_test;
    $t->current_test(0);
}

# each test test is actually two tests.  This is bad and wrong
# but makes blood come out of my ears if I don't at least simplify
# it a little this way

sub my_test_test
{
  my $text = shift;
  local $^W = 0;

  # reset the outputs 
  $t->output($original_output_handle);
  $t->failure_output($original_failure_handle);
  $t->todo_output($original_todo_handle);
  $ENV{HARNESS_ACTIVE} = $original_harness_env;

  # reset the number of tests
  $t->current_test($testing_num);

  # check we got the same values
  my $got;
  my $wanted;

  # stdout
  $t->ok($out->check, "STDOUT $text");

  # stderr
  $t->ok($err->check, "STDERR $text");
}

####################################################################
# Meta meta tests
####################################################################

# this is a quick test to check the hack that I've just implemented
# actually does a cut down version of Test::Builder::Tester

start_testing();
$out->expect("ok 1 - foo");
pass("foo");
my_test_test("basic meta meta test");

start_testing();
$out->expect("not ok 1 - foo");
$err->expect("#     Failed test ($0 at line ".line_num(+1).")");
fail("foo");
my_test_test("basic meta meta test 2");

start_testing();
$out->expect("ok 1 - bar");
test_out("ok 1 - foo");
pass("foo");
test_test("bar");
my_test_test("meta meta test with tbt");

start_testing();
$out->expect("ok 1 - bar");
test_out("not ok 1 - foo");
test_err("#     Failed test ($0 at line ".line_num(+1).")");
fail("foo");
test_test("bar");
my_test_test("meta meta test with tbt2 ");

####################################################################
