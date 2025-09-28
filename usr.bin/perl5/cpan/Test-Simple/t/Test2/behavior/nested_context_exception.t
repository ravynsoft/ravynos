use strict;
use warnings;
BEGIN { $Test2::API::DO_DEPTH_CHECK = 1 }
use Test2::Tools::Tiny;

use Test2::API qw/context/;

skip_all("known to fail on $]") if $] le "5.006002";

sub outer {
    my $code = shift;
    my $ctx = context();

    $ctx->note("outer");

    my $out = eval { $code->() };

    $ctx->release;

    return $out;
}

sub dies {
    my $ctx = context();
    $ctx->note("dies");
    die "Foo";
}

sub bad_store {
    my $ctx = context();
    $ctx->note("bad store");
    return $ctx; # Emulate storing it somewhere
}

sub bad_simple {
    my $ctx = context();
    $ctx->note("bad simple");
    return;
}

my @warnings;
{
    local $SIG{__WARN__} = sub { push @warnings => @_ };
    eval { dies() };
}
ok(!@warnings, "no warnings") || diag @warnings;

@warnings = ();
my $keep = bad_store();
eval { my $x = 1 }; # Ensure an eval changing $@ does not meddle.
{
    local $SIG{__WARN__} = sub { push @warnings => @_ };
    ok(1, "random event");
}
ok(@warnings, "got warnings");
like(
    $warnings[0],
    qr/context\(\) was called to retrieve an existing context/,
    "got expected warning"
);
$keep = undef;

{
    @warnings = ();
    local $SIG{__WARN__} = sub { push @warnings => @_ };
    bad_simple();
}
ok(@warnings, "got warnings");
like(
    $warnings[0],
    qr/A context appears to have been destroyed without first calling release/,
    "got expected warning"
);

@warnings = ();
outer(\&dies);
{
    local $SIG{__WARN__} = sub { push @warnings => @_ };
    ok(1, "random event");
}
ok(!@warnings, "no warnings") || diag @warnings;



@warnings = ();
{
    local $SIG{__WARN__} = sub { push @warnings => @_ };
    outer(\&bad_store);
}
ok(@warnings, "got warnings");
like(
    $warnings[0],
    qr/A context appears to have been destroyed without first calling release/,
    "got expected warning"
);



{
    @warnings = ();
    local $SIG{__WARN__} = sub { push @warnings => @_ };
    outer(\&bad_simple);
}
ok(@warnings, "got warnings") || diag @warnings;
like(
    $warnings[0],
    qr/A context appears to have been destroyed without first calling release/,
    "got expected warning"
);



done_testing;
