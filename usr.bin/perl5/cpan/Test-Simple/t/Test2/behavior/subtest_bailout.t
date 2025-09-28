use Test2::Tools::Tiny;
use strict;
use warnings;

use Test2::API qw/context run_subtest intercept/;

sub subtest {
    my ($name, $code) = @_;
    my $ctx = context();
    my $pass = run_subtest($name, $code, {buffered => 1}, @_);
    $ctx->release;
    return $pass;
}

sub bail {
    my $ctx = context();
    $ctx->bail(@_);
    $ctx->release;
}

my $events = intercept {
    subtest outer => sub {
        subtest inner => sub {
            bail("bye!");
        };
    };
};

ok($events->[0]->isa('Test2::Event::Subtest'), "Got a subtest event when bail-out issued in a buffered subtest");
ok($events->[-1]->isa('Test2::Event::Bail'), "Bail-Out propogated");
ok(!$events->[-1]->facet_data->{trace}->{buffered}, "Final Bail-Out is not buffered");

ok($events->[0]->subevents->[-2]->isa('Test2::Event::Bail'), "Got bail out inside outer subtest");
ok($events->[0]->subevents->[-2]->facet_data->{trace}->{buffered}, "Bail-Out is buffered");

ok($events->[0]->subevents->[0]->subevents->[-2]->isa('Test2::Event::Bail'), "Got bail out inside inner subtest");
ok($events->[0]->subevents->[0]->subevents->[-2]->facet_data->{trace}->{buffered}, "Bail-Out is buffered");

done_testing;
