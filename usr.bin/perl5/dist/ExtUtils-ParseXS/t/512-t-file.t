#!/usr/bin/perl
use strict;
use warnings;

use Test::More tests => 6;
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

my $cmp_typemap_file = File::Spec->catfile($datadir, 'simple.typemap');
my $cmp_typemap_str  = slurp($cmp_typemap_file);

my $map = ExtUtils::Typemaps->new();
$map->add_typemap(ctype => 'unsigned int', xstype => 'T_UV');
$map->add_inputmap(xstype => 'T_UV', code => '$var = ($type)SvUV($arg);');
$map->add_outputmap(xstype => 'T_UV', code => 'sv_setuv($arg, (UV)$var);');
$map->add_typemap(ctype => 'int', xstype => 'T_IV');
$map->add_inputmap(xstype => 'T_IV', code => '$var = ($type)SvIV($arg);');
$map->add_outputmap(xstype => 'T_IV', code => 'sv_setiv($arg, (IV)$var);');

is($map->as_string(), $cmp_typemap_str, "Simple typemap matches reference file");

my $tmpdir = File::Temp::tempdir(CLEANUP => 1, TMPDIR => 1);
my $tmpfile = File::Spec->catfile($tmpdir, 'simple.typemap');

$map->write(file => $tmpfile);
is($map->as_string(), slurp($tmpfile), "Simple typemap write matches as_string");
is(ExtUtils::Typemaps->new(file => $cmp_typemap_file)->as_string(), $cmp_typemap_str, "Simple typemap roundtrips");
is(ExtUtils::Typemaps->new(file => $tmpfile)->as_string(), $cmp_typemap_str, "Simple typemap roundtrips (2)");

SCOPE: {
  local $map->{file} = $cmp_typemap_file;
  is_deeply(ExtUtils::Typemaps->new(file => $cmp_typemap_file), $map, "Simple typemap roundtrips (in memory)");
}

# test that we can also create them from a string
my $map_from_str = ExtUtils::Typemaps->new(string => $map->as_string());
is_deeply($map_from_str, $map);

