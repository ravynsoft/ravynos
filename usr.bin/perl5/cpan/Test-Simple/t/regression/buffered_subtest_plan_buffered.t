use Test2::Tools::Tiny;
use strict;
use warnings;

use Test2::API qw/intercept test2_stack/;
use Data::Dumper;

sub hide_buffered { 0 }

sub write {
    my $self = shift;
    my ($e) = @_;

    push @{$self->{events}} => $e;
}

sub finalize { }

my $events;
intercept {
    my $hub = test2_stack()->top;
    my $formatter = bless({}, __PACKAGE__);
    $hub->format($formatter);
    tests xxx => sub {
        ok(1, "pass");
    };

    $events = $formatter->{events};
};

pop @$events;


for my $e (@$events) {
    ok($e->trace->buffered, "Buffered events are all listed as buffered") || diag(Dumper($e));
}

done_testing;

