#!perl -w
use strict;
use Test::More tests => 9;

require_ok('Module::CoreList');

ok($Module::CoreList::deprecated{5.011000}, "5.011000 (deprecated list)");

ok(!exists $Module::CoreList::deprecated{5.011000}{'File::Spec'},
   "File::Spec not deprecated in 5.011000 (hash)"
);

ok(! Module::CoreList::is_deprecated('File::Spec'),
   "File::Spec currently is not deprecated (function)"
);

ok(exists $Module::CoreList::deprecated{5.011000}{'Switch'},
   "Switch deprecated in 5.011000 (hash)"
);

is(!! Module::CoreList::is_deprecated('Switch'), !! ($] >= 5.011 and $] < 5.013001 ),
   "Switch deprecated current perl (if pre-5.13.1.) (function)"
);

ok(! Module::CoreList::is_deprecated('Switch', 5.010000), 
   "Switch not deprecated in 5.010000 (function w/ perl version)"
);

is(Module::CoreList::deprecated_in('Switch'), 5.011000,
   "Switch was deprecated in 5.011000 (deprecated_in)"
);

ok(! Module::CoreList::deprecated_in('File::Spec'),
   "File::Spec currently is not deprecated (deprecated_in)"
);
