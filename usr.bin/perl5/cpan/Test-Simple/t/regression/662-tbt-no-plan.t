use Test::Builder::Tester;
use Test::More tests => 1;
use strict;
use warnings;

BEGIN {
    package Example::Tester;

    use base 'Test::Builder::Module';
    $INC{'Example/Tester.pm'} = 1;

    sub import {
        my $package    = shift;
        my %args       = @_;
        my $callerpack = caller;
        my $tb         = __PACKAGE__->builder;
        $tb->exported_to($callerpack);
        local $SIG{__WARN__} = sub { };
        $tb->no_plan;
    }
}

test_out('ok 1 - use Example::Tester;');
use_ok('Example::Tester');
test_test("use Example::Tester;");
