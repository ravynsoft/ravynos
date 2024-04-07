# copied over from JSON::XS and modified to use JSON::PP

# copied over from JSON::DWIW and modified to use JSON::PP

# Creation date: 2007-02-20 21:54:09
# Authors: don

use strict;
use warnings;
use Test::More tests => 7;

# main
{
    BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

    my $json_str = '{"var1":"val1","var2":["first_element",{"sub_element":"sub_val","sub_element2":"sub_val2"}],"var3":"val3"}';

    my $json_obj = JSON::PP->new->allow_nonref(1);
    my $data = $json_obj->decode($json_str);

    my $pass = 1;
    if ($data->{var1} eq 'val1' and $data->{var3} eq 'val3') {
        if ($data->{var2}) {
            my $array = $data->{var2};
            if (ref($array) eq 'ARRAY') {
                if ($array->[0] eq 'first_element') {
                    my $hash = $array->[1];
                    if (ref($hash) eq 'HASH') {
                        unless ($hash->{sub_element} eq 'sub_val'
                                and $hash->{sub_element2} eq 'sub_val2') {
                            $pass = 0;
                        }
                    }
                    else {
                        $pass = 0;
                    }
                }
                else {
                    $pass = 0;
                }
            }
            else {
                $pass = 0;
            }
        }
        else {
            $pass = 0;
        }
    }
    
    ok($pass);

    $json_str = '"val1"';
    $data = $json_obj->decode($json_str);
    ok($data eq 'val1');

    $json_str = '567';
    $data = $json_obj->decode($json_str);
    ok($data == 567);

    $json_str = "5e1";
    $data = $json_obj->decode($json_str);
    ok($data == 50);

    $json_str = "5e3";
    $data = $json_obj->decode($json_str);
    ok($data == 5000);

    $json_str = "5e+1";
    $data = $json_obj->decode($json_str);
    ok($data == 50);

    $json_str = "5e-1";
    $data = $json_obj->decode($json_str);
    ok($data == 0.5);



    
#     use Data::Dumper;
#     print STDERR Dumper($test_data) . "\n\n";

}

exit 0;

###############################################################################
# Subroutines

