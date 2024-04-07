package Hints_test;
use strict;
use warnings;

use Exporter 5.57 'import';

our @EXPORT_OK = qw(
    fail_on_empty fail_on_false fail_on_undef
);

use autodie::hints;

# Create some dummy subs that just return their arguments.

sub fail_on_empty { return @_; }
sub fail_on_false { return @_; }
sub fail_on_undef { return @_; }

# Set them to different failure modes when used with autodie.

autodie::hints->set_hints_for(
    \&fail_on_empty, { 
        list => autodie::hints::EMPTY_ONLY ,
        scalar => autodie::hints::EMPTY_ONLY 
    }
);

autodie::hints->set_hints_for(
    \&fail_on_false, {
        list => autodie::hints::EMPTY_OR_FALSE ,
        scalar => autodie::hints::EMPTY_OR_FALSE
    }
);

autodie::hints->set_hints_for(
    \&fail_on_undef, {
        list => autodie::hints::EMPTY_OR_UNDEF ,
        scalar => autodie::hints::EMPTY_OR_UNDEF 
    }
);

1;
