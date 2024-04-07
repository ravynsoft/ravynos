#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

is( _num_to_alpha(-1), undef, 'Returns undef for negative numbers');
is( _num_to_alpha( 0), 'A', "Starts at 'A'");
is( _num_to_alpha( 1), 'B');

is( _num_to_alpha(26 - 1), 'Z', 'Last single letter return value');
is( _num_to_alpha(26    ), 'AA', 'First double letter return value');
is( _num_to_alpha(26 + 1), 'AB');

is( _num_to_alpha(26 + 26 - 2), 'AY');
is( _num_to_alpha(26 + 26 - 1), 'AZ');
is( _num_to_alpha(26 + 26    ), 'BA');
is( _num_to_alpha(26 + 26 + 1), 'BB');

is( _num_to_alpha(26 ** 2 - 1), 'YZ');
is( _num_to_alpha(26 ** 2    ), 'ZA');
is( _num_to_alpha(26 ** 2 + 1), 'ZB');

is( _num_to_alpha(26 ** 2 + 26 - 1), 'ZZ', 'Last double letter return value');
is( _num_to_alpha(26 ** 2 + 26    ), 'AAA', 'First triple letter return value');
is( _num_to_alpha(26 ** 2 + 26 + 1), 'AAB');

is( _num_to_alpha(26 ** 3 + 26 ** 2 + 26 - 1 ), 'ZZZ', 'Last triple letter return value');
is( _num_to_alpha(26 ** 3 + 26 ** 2 + 26     ), 'AAAA', 'First quadruple letter return value');
is( _num_to_alpha(26 ** 3 + 26 ** 2 + 26 + 1 ), 'AAAB');

note('Testing limit capabilities');

is( _num_to_alpha(26 - 1 , 1), 'Z', 'Largest return value for one letter');
is( _num_to_alpha(26     , 1), undef); # AA

is( _num_to_alpha(26 ** 2 + 26 - 1 , 2 ), 'ZZ', 'Largest return value for two letters');
is( _num_to_alpha(26 ** 2 + 26     , 2 ), undef); # AAA

is( _num_to_alpha(26 ** 3 + 26 ** 2 + 26 - 1 , 3 ), 'ZZZ', 'Largest return value for three letters');
is( _num_to_alpha(26 ** 3 + 26 ** 2 + 26     , 3 ), undef); # AAAA

done_testing();
