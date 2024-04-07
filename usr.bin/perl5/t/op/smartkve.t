#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}
use strict;
use warnings;
no warnings 'experimental::refaliasing';
our ($data, $array, $values, $hash, $errpat);

plan 'no_plan';

my $empty;

sub set_errpat {
    # Checking for a comma after the line number ensures that we are using
    # yyerror for the error, rather than croak.  yyerror is preferable for
    # compile-time errors.
    $errpat =
       qr/Experimental $_[0] on scalar is now forbidden .* line 1\.(?x:
         ).*Type of arg 1 to $_[0] must be hash or array \(not (?x:
         ).*line 1,/s;
}

# Keys -- errors
set_errpat 'keys';

eval "keys undef";
like($@, $errpat,
  'Errors: keys undef throws error'
);

undef $empty;
eval q"keys $empty";
like($@, $errpat,
  'Errors: keys $undef throws error'
);

is($empty, undef, 'keys $undef does not vivify $undef');

eval "keys 3";
like($@, qr/Type of arg 1 to keys must be hash/,
  'Errors: keys CONSTANT throws error'
);

eval "keys qr/foo/";
like($@, $errpat,
  'Errors: keys qr/foo/ throws error'
);

eval q"keys $hash qw/fo bar/";
like($@, $errpat,
  'Errors: keys $hash, @stuff throws error'
) or print "# Got: $@";

# Values -- errors
set_errpat 'values';

eval "values undef";
like($@, $errpat,
  'Errors: values undef throws error'
);

undef $empty;
eval q"values $empty";
like($@, $errpat,
  'Errors: values $undef throws error'
);

is($empty, undef, 'values $undef does not vivify $undef');

eval "values 3";
like($@, qr/Type of arg 1 to values must be hash/,
  'Errors: values CONSTANT throws error'
);

eval "values qr/foo/";
like($@, $errpat,
  'Errors: values qr/foo/ throws error'
);

eval q"values $hash qw/fo bar/";
like($@, $errpat,
  'Errors: values $hash, @stuff throws error'
) or print "# Got: $@";

# Each -- errors
set_errpat 'each';

eval "each undef";
like($@, $errpat,
  'Errors: each undef throws error'
);

undef $empty;
eval q"each $empty";
like($@, $errpat,
  'Errors: each $undef throws error'
);

is($empty, undef, 'each $undef does not vivify $undef');

eval "each 3";
like($@, qr/Type of arg 1 to each must be hash/,
  'Errors: each CONSTANT throws error'
);

eval "each qr/foo/";
like($@, $errpat,
  'Errors: each qr/foo/ throws error'
);

eval q"each $hash qw/foo bar/";
like($@, $errpat,
  'Errors: each $hash, @stuff throws error'
) or print "# Got: $@";
