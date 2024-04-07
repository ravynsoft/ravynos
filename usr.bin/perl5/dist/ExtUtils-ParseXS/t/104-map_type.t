#!/usr/bin/perl
use strict;
use warnings;
use Test::More tests =>  7;
use ExtUtils::ParseXS;
use ExtUtils::ParseXS::Utilities qw(
  map_type
);

my ($self, $type, $varname);
my ($result, $expected);

$self = ExtUtils::ParseXS->new;

$type = 'struct DATA *';
$varname = 'RETVAL';
$self->{RetainCplusplusHierarchicalTypes} = 0;
$expected = "$type\t$varname";
$result = map_type($self, $type, $varname);
is( $result, $expected,
    "Got expected map_type for <$type>, <$varname>, <$self->{RetainCplusplusHierarchicalTypes}>" );

$type = 'Crypt::Shark';
$varname = undef;
$self->{RetainCplusplusHierarchicalTypes} = 0;
$expected = 'Crypt__Shark';
$result = map_type($self, $type, $varname);
is( $result, $expected,
    "Got expected map_type for <$type>, undef, <$self->{RetainCplusplusHierarchicalTypes}>" );

$type = 'Crypt::Shark';
$varname = undef;
$self->{RetainCplusplusHierarchicalTypes} = 1;
$expected = 'Crypt::Shark';
$result = map_type($self, $type, $varname);
is( $result, $expected,
    "Got expected map_type for <$type>, undef, <$self->{RetainCplusplusHierarchicalTypes}>" );

$type = 'Crypt::TC18';
$varname = 'RETVAL';
$self->{RetainCplusplusHierarchicalTypes} = 0;
$expected = "Crypt__TC18\t$varname";
$result = map_type($self, $type, $varname);
is( $result, $expected,
    "Got expected map_type for <$type>, <$varname>, <$self->{RetainCplusplusHierarchicalTypes}>" );

$type = 'Crypt::TC18';
$varname = 'RETVAL';
$self->{RetainCplusplusHierarchicalTypes} = 1;
$expected = "Crypt::TC18\t$varname";
$result = map_type($self, $type, $varname);
is( $result, $expected,
    "Got expected map_type for <$type>, <$varname>, <$self->{RetainCplusplusHierarchicalTypes}>" );

$type = 'array(alpha,beta) gamma';
$varname = 'RETVAL';
$self->{RetainCplusplusHierarchicalTypes} = 0;
$expected = "alpha *\t$varname";
$result = map_type($self, $type, $varname);
is( $result, $expected,
    "Got expected map_type for <$type>, <$varname>, <$self->{RetainCplusplusHierarchicalTypes}>" );

$type = '(*)';
$varname = 'RETVAL';
$self->{RetainCplusplusHierarchicalTypes} = 0;
$expected = "(* $varname )";
$result = map_type($self, $type, $varname);
is( $result, $expected,
    "Got expected map_type for <$type>, <$varname>, <$self->{RetainCplusplusHierarchicalTypes}>" );
