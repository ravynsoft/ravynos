use warnings;
no warnings 'once';
use Test::More tests => 7;

use Carp;

use Data::Dumper ();
sub _dump ($) {
  Data::Dumper->new(\@_)->Indent(1)->Terse(1)->Dump;
}

my $o = Stringable->new(key => 'Baz');

my $msg = call(\&with_longmess, $o, {bar => 'buzz'});
like($msg, qr/, Stringable=HASH\(0x[[:xdigit:]]+\),/,
	    "Stringable object not overload stringified");
like($msg, qr/, HASH\(0x[[:xdigit:]]+\)\)/, "HASH *not* stringified");

{
    my $called;

    local $Carp::RefArgFormatter = sub {
        $called++;
        join '', _dump $_[0];
    };

    $msg = call(\&with_longmess, $o, {bar => 'buzz'});
    ok($called, "Called private formatter");
    like($msg, qr/bar.*buzz/m, 'HASH stringified');
}

$o = CarpTracable->new(key => 'Bax');
$msg = call(\&with_longmess, $o, {bar => 'buzz'});
ok($o->{called}, "CARP_TRACE called");
like($msg, qr/, TRACE:CarpTracable=Bax, /, "CARP_TRACE output used") or diag _dump $msg;
like($msg, qr/, HASH\(0x[[:xdigit:]]+\)\)/, "HASH not stringified again");

sub call
{
    my $func = shift;
    $func->(@_);
}

sub with_longmess
{
    my $g = shift;
    Carp::longmess("longmess:\n");
}

package Stringable;

use overload
    q[""] => 'as_string';

sub new { my $class = shift; return bless {@_}, $class }

sub as_string
{
    my $self = shift;
    join '=', ref $self, $self->{key} || '<no key>';
}

package CarpTracable;

# need to set inheritance of new() etc before the
# CarpTracable->new calls higher up
BEGIN { our @ISA = 'Stringable' }

sub CARP_TRACE
{
    my $self = shift;
    $self->{called}++;
    "TRACE:" . $self; # use string overload
}

