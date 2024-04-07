#!/usr/bin/perl
use strict;
use warnings;

use Test::More tests => 43;
use ExtUtils::Typemaps;

# empty typemap
SCOPE: {
  ok(ExtUtils::Typemaps->new()->is_empty(), "This is an empty typemap");
}

# typemap only
SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_typemap(ctype => 'unsigned int', xstype => 'T_IV');
  ok(!$map->is_empty(), "This is not an empty typemap");

  is($map->as_string(), <<'HERE', "Simple typemap matches expectations");
TYPEMAP
unsigned int	T_IV
HERE

  my $type = $map->get_typemap(ctype => 'unsigned int');
  isa_ok($type, 'ExtUtils::Typemaps::Type');
  is($type->ctype, 'unsigned int');
  is($type->xstype, 'T_IV');
  is($type->tidy_ctype, 'unsigned int');


  # test failure
  ok(!$map->get_typemap(ctype => 'foo'), "Access to nonexistent typemap doesn't die");
  ok(!$map->get_inputmap(ctype => 'foo'), "Access to nonexistent inputmap via ctype doesn't die");
  ok(!$map->get_outputmap(ctype => 'foo'), "Access to nonexistent outputmap via ctype doesn't die");
  ok(!$map->get_inputmap(xstype => 'foo'), "Access to nonexistent inputmap via xstype doesn't die");
  ok(!$map->get_outputmap(xstype => 'foo'), "Access to nonexistent outputmap via xstype doesn't die");
  ok(!eval{$map->get_typemap('foo')} && $@, "Access to typemap with positional params dies");
  ok(!eval{$map->get_inputmap('foo')} && $@, "Access to inputmap with positional params dies");
  ok(!eval{$map->get_outputmap('foo')} && $@, "Access to outputmap with positional params dies");
}

# typemap & input
SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_inputmap(xstype => 'T_UV', code => '$var = ($type)SvUV($arg);');
  ok(!$map->is_empty(), "This is not an empty typemap");
  $map->add_typemap(ctype => 'unsigned int', xstype => 'T_UV');
  is($map->as_string(), <<'HERE', "Simple typemap (with input) matches expectations");
TYPEMAP
unsigned int	T_UV

INPUT
T_UV
	$var = ($type)SvUV($arg);
HERE

  my $type = $map->get_typemap(ctype => 'unsigned int');
  isa_ok($type, 'ExtUtils::Typemaps::Type');
  is($type->ctype, 'unsigned int');
  is($type->xstype, 'T_UV');
  is($type->tidy_ctype, 'unsigned int');

  my $in = $map->get_inputmap(xstype => 'T_UV');
  isa_ok($in, 'ExtUtils::Typemaps::InputMap');
  is($in->xstype, 'T_UV');

  # test fetching inputmap by ctype
  my $in2 = $map->get_inputmap(ctype => 'unsigned int');
  is_deeply($in2, $in, "get_inputmap returns the same typemap for ctype and xstype");
}


# typemap & output
SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_outputmap(xstype => 'T_UV', code => 'sv_setuv($arg, (UV)$var);');
  ok(!$map->is_empty(), "This is not an empty typemap");
  $map->add_typemap(ctype => 'unsigned int', xstype => 'T_UV');
  is($map->as_string(), <<'HERE', "Simple typemap (with output) matches expectations");
TYPEMAP
unsigned int	T_UV

OUTPUT
T_UV
	sv_setuv($arg, (UV)$var);
HERE

  my $type = $map->get_typemap(ctype => 'unsigned int');
  isa_ok($type, 'ExtUtils::Typemaps::Type');
  is($type->ctype, 'unsigned int');
  is($type->xstype, 'T_UV');
  is($type->tidy_ctype, 'unsigned int');

  my $in = $map->get_outputmap(xstype => 'T_UV');
  isa_ok($in, 'ExtUtils::Typemaps::OutputMap');
  is($in->xstype, 'T_UV');
}

# typemap & input & output
SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_typemap(ctype => 'unsigned int', xstype => 'T_UV');
  $map->add_inputmap(xstype => 'T_UV', code => '$var = ($type)SvUV($arg);');
  $map->add_outputmap(xstype => 'T_UV', code => 'sv_setuv($arg, (UV)$var);');
  ok(!$map->is_empty(), "This is not an empty typemap");
  is($map->as_string(), <<'HERE', "Simple typemap (with in- & output) matches expectations");
TYPEMAP
unsigned int	T_UV

INPUT
T_UV
	$var = ($type)SvUV($arg);

OUTPUT
T_UV
	sv_setuv($arg, (UV)$var);
HERE
}

# two typemaps & input & output
SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_typemap(ctype => 'unsigned int', xstype => 'T_UV');
  $map->add_inputmap(xstype => 'T_UV', code => '$var = ($type)SvUV($arg);');
  $map->add_outputmap(xstype => 'T_UV', code => 'sv_setuv($arg, (UV)$var);');

  $map->add_typemap(ctype => 'int', xstype => 'T_IV');
  $map->add_inputmap(xstype => 'T_IV', code => '$var = ($type)SvIV($arg);');
  $map->add_outputmap(xstype => 'T_IV', code => 'sv_setiv($arg, (IV)$var);');
  is($map->as_string(), <<'HERE', "Simple typemap (with in- & output) matches expectations");
TYPEMAP
unsigned int	T_UV
int	T_IV

INPUT
T_UV
	$var = ($type)SvUV($arg);
T_IV
	$var = ($type)SvIV($arg);

OUTPUT
T_UV
	sv_setuv($arg, (UV)$var);
T_IV
	sv_setiv($arg, (IV)$var);
HERE
  my $type = $map->get_typemap(ctype => 'unsigned int');
  isa_ok($type, 'ExtUtils::Typemaps::Type');
  is($type->ctype, 'unsigned int');
  is($type->xstype, 'T_UV');
  is($type->tidy_ctype, 'unsigned int');

  my $in = $map->get_outputmap(xstype => 'T_UV');
  isa_ok($in, 'ExtUtils::Typemaps::OutputMap');
  is($in->xstype, 'T_UV');
  $in = $map->get_outputmap(xstype => 'T_IV');
  isa_ok($in, 'ExtUtils::Typemaps::OutputMap');
  is($in->xstype, 'T_IV');
}

