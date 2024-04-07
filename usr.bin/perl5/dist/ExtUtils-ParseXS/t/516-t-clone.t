#!/usr/bin/perl
use strict;
use warnings;

use Test::More tests => 6;
use ExtUtils::Typemaps;

# Test that cloning typemap object shallowly or deeply both
# works as designed.

SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_typemap(ctype => 'unsigned int', xstype => 'T_UV');
  $map->add_inputmap(xstype => 'T_UV', code => '$var = ($type)SvUV($arg);');
  $map->add_outputmap(xstype => 'T_UV', code => 'sv_setuv($arg, (UV)$var);');

  $map->add_typemap(ctype => 'int', xstype => 'T_IV');
  $map->add_inputmap(xstype => 'T_IV', code => '$var = ($type)SvIV($arg);');
  $map->add_outputmap(xstype => 'T_IV', code => 'sv_setiv($arg, (IV)$var);');

  my $clone = $map->clone;
  my $shallow = $map->clone(shallow => 1);

  is_deeply($clone, $map, "Full clone equivalent to original");
  is_deeply($shallow, $map, "Shallow clone equivalent to original");
  
  $map->add_typemap(ctype => "foo", xstype => "bar");

  ok(!$clone->get_typemap(ctype => 'foo'), "New typemap not propagated to full clone");
  ok(!$shallow->get_typemap(ctype => 'foo'), "New typemap not propagated to shallow clone");
  
  my $t = $map->get_typemap(ctype => 'unsigned int');
  $t->{blubb} = 1;

  ok(!$clone->get_typemap(ctype => 'unsigned int')->{blubb}, "Direct modification does not propagate to full clone");
  ok($shallow->get_typemap(ctype => 'unsigned int')->{blubb}, "Direct modification does propagate to shallow clone");
}

