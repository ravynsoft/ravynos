# copied over from JSON::XS and modified to use JSON::PP

# copied over from JSON::DWIW and modified to use JSON::PP

# Creation date: 2007-02-20 19:51:06
# Authors: don

use strict;
use warnings;
use Test::More tests => 5;

# main
{
    BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

    my $data;

    #    my $expected_str = '{"var1":"val1","var2":["first_element",{"sub_element":"sub_val","sub_element2":"sub_val2"}],"var3":"val3"}';

    my $expected_str1 = '{"var1":"val1","var2":["first_element",{"sub_element":"sub_val","sub_element2":"sub_val2"}]}';
    my $expected_str2 = '{"var2":["first_element",{"sub_element":"sub_val","sub_element2":"sub_val2"}],"var1":"val1"}';
    my $expected_str3 = '{"var2":["first_element",{"sub_element2":"sub_val2","sub_element":"sub_val"}],"var1":"val1"}';
    my $expected_str4 = '{"var1":"val1","var2":["first_element",{"sub_element2":"sub_val2","sub_element":"sub_val"}]}';

    my $json_obj = JSON::PP->new->allow_nonref (1);
    my $json_str;
    # print STDERR "\n" . $json_str . "\n\n";

    my $expected_str;

    $data = 'stuff';
    $json_str = $json_obj->encode($data);
    ok($json_str eq '"stuff"');

    $data = "stu\nff";
    $json_str = $json_obj->encode($data);
    ok($json_str eq '"stu\nff"');

    $data = [ 1, 2, 3 ];
    $expected_str = '[1,2,3]';
    $json_str = $json_obj->encode($data);

    ok($json_str eq $expected_str);

    $data = { var1 => 'val1', var2 => 'val2' };
    $json_str = $json_obj->encode($data);

    ok($json_str eq '{"var1":"val1","var2":"val2"}'
       or $json_str eq '{"var2":"val2","var1":"val1"}');
    
    $data = { var1 => 'val1',
              var2 => [ 'first_element',
                        { sub_element => 'sub_val', sub_element2 => 'sub_val2' },
                      ],
              #                 var3 => 'val3',
            };

    $json_str = $json_obj->encode($data);

    ok($json_str eq $expected_str1 or $json_str eq $expected_str2
       or $json_str eq $expected_str3 or $json_str eq $expected_str4);
}

exit 0;

###############################################################################
# Subroutines

