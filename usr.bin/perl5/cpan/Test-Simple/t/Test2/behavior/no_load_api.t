use strict;
use warnings;
use Data::Dumper;
# HARNESS-NO-STREAM
# HARNESS-NO-PRELOAD

###############################################################################
#                                                                             #
# This test is to insure certain objects do not load Test2::API directly or   #
# indirectly when being required. It is ok for import() to load Test2::API if #
# necessary, but simply requiring the modules should not.                     #
#                                                                             #
###############################################################################

require Test2::Formatter;
require Test2::Formatter::TAP;

require Test2::Event;
require Test2::Event::Bail;
require Test2::Event::Diag;
require Test2::Event::Exception;
require Test2::Event::Note;
require Test2::Event::Ok;
require Test2::Event::Plan;
require Test2::Event::Skip;
require Test2::Event::Subtest;
require Test2::Event::Waiting;

require Test2::Util;
require Test2::Util::ExternalMeta;
require Test2::Util::HashBase;
require Test2::EventFacet::Trace;

require Test2::Hub;
require Test2::Hub::Interceptor;
require Test2::Hub::Subtest;
require Test2::Hub::Interceptor::Terminator;

my @loaded = grep { $INC{$_} } qw{
    Test2/API.pm
    Test2/API/Instance.pm
    Test2/API/Context.pm
    Test2/API/Stack.pm
};

require Test2::Tools::Tiny;

Test2::Tools::Tiny::ok(!@loaded, "Test2::API was not loaded")
    || Test2::Tools::Tiny::diag("Loaded: " . Dumper(\@loaded));

Test2::Tools::Tiny::done_testing();
