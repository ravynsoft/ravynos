use strict;
use warnings;
use Test::More;
BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }
use JSON::PP;

#SKIP_ALL_UNLESS_XS4_COMPAT

package #
    Dummy::True;
*Dummy::True:: = *JSON::PP::Boolean::;

package #
    Dummy::False;
*Dummy::False:: = *JSON::PP::Boolean::;

package main;

my $dummy_true = bless \(my $dt = 1), 'Dummy::True';
my $dummy_false = bless \(my $df = 0), 'Dummy::False';

my @tests = ([$dummy_true, $dummy_false, 'Dummy::True', 'Dummy::False']);

# extra boolean classes
if (eval "require boolean; 1") {
    push @tests, [boolean::true(), boolean::false(), 'boolean', 'boolean', 1];
}
if (eval "require JSON::PP; 1") {
    push @tests, [JSON::PP::true(), JSON::PP::false(), 'JSON::PP::Boolean', 'JSON::PP::Boolean'];
}
if (eval "require Data::Bool; 1") {
    push @tests, [Data::Bool::true(), Data::Bool::false(), 'Data::Bool::Impl', 'Data::Bool::Impl'];
}
if (eval "require Types::Serialiser; 1") {
    push @tests, [Types::Serialiser::true(), Types::Serialiser::false(), 'Types::Serialiser::BooleanBase', 'Types::Serialiser::BooleanBase'];
}

plan tests => 15 * @tests;

my $json = JSON::PP->new;
for my $test (@tests) {
    my ($true, $false, $true_class, $false_class, $incompat) = @$test;

    my $ret = $json->boolean_values($false, $true);
    is $ret => $json, "returns the same object";
    my ($new_false, $new_true) = $json->get_boolean_values;
    ok defined $new_true, "new true class is defined";
    ok defined $new_false, "new false class is defined";
    ok $new_true->isa($true_class), "new true class is $true_class";
    ok $new_false->isa($false_class), "new false class is $false_class";
    SKIP: {
        skip "$true_class is not compatible with JSON::PP::Boolean", 2 if $incompat;
        ok $new_true->isa('JSON::PP::Boolean'), "new true class is also JSON::PP::Boolean";
        ok $new_false->isa('JSON::PP::Boolean'), "new false class is also JSON::PP::Boolean";
    }

    my $should_true = $json->allow_nonref(1)->decode('true');
    ok $should_true->isa($true_class), "JSON true turns into a $true_class object";

    my $should_false = $json->allow_nonref(1)->decode('false');
    ok $should_false->isa($false_class), "JSON false turns into a $false_class object";

    SKIP: {
        skip "$true_class is not compatible with JSON::PP::Boolean", 2 if $incompat;
        my $should_true_json = eval { $json->allow_nonref(1)->encode($new_true); };
        is $should_true_json => 'true', "A $true_class object turns into JSON true";

        my $should_false_json = eval { $json->allow_nonref(1)->encode($new_false); };
        is $should_false_json => 'false', "A $false_class object turns into JSON false";
    }

    $ret = $json->boolean_values();
    is $ret => $json, "returns the same object";
    ok !$json->get_boolean_values, "reset boolean values";

    $should_true = $json->allow_nonref(1)->decode('true');
    ok $should_true->isa('JSON::PP::Boolean'), "JSON true turns into a JSON::PP::Boolean object";

    $should_false = $json->allow_nonref(1)->decode('false');
    ok $should_false->isa('JSON::PP::Boolean'), "JSON false turns into a JSON::PP::Boolean object";
}
