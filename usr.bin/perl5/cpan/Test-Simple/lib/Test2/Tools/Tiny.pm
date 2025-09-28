package Test2::Tools::Tiny;
use strict;
use warnings;

BEGIN {
    if ($] lt "5.008") {
        require Test::Builder::IO::Scalar;
    }
}

use Scalar::Util qw/blessed/;

use Test2::Util qw/try/;
use Test2::API qw/context run_subtest test2_stack/;

use Test2::Hub::Interceptor();
use Test2::Hub::Interceptor::Terminator();

our $VERSION = '1.302194';

BEGIN { require Exporter; our @ISA = qw(Exporter) }
our @EXPORT = qw{
    ok is isnt like unlike is_deeply diag note skip_all todo plan done_testing
    warnings exception tests capture
};

sub ok($;$@) {
    my ($bool, $name, @diag) = @_;
    my $ctx = context();

    return $ctx->pass_and_release($name) if $bool;
    return $ctx->fail_and_release($name, @diag);
}

sub is($$;$@) {
    my ($got, $want, $name, @diag) = @_;
    my $ctx = context();

    my $bool;
    if (defined($got) && defined($want)) {
        $bool = "$got" eq "$want";
    }
    elsif (defined($got) xor defined($want)) {
        $bool = 0;
    }
    else {    # Both are undef
        $bool = 1;
    }

    return $ctx->pass_and_release($name) if $bool;

    $got  = '*NOT DEFINED*' unless defined $got;
    $want = '*NOT DEFINED*' unless defined $want;
    unshift @diag => (
        "GOT:      $got",
        "EXPECTED: $want",
    );

    return $ctx->fail_and_release($name, @diag);
}

sub isnt($$;$@) {
    my ($got, $want, $name, @diag) = @_;
    my $ctx = context();

    my $bool;
    if (defined($got) && defined($want)) {
        $bool = "$got" ne "$want";
    }
    elsif (defined($got) xor defined($want)) {
        $bool = 1;
    }
    else {    # Both are undef
        $bool = 0;
    }

    return $ctx->pass_and_release($name) if $bool;

    unshift @diag => "Strings are the same (they should not be)"
        unless $bool;

    return $ctx->fail_and_release($name, @diag);
}

sub like($$;$@) {
    my ($thing, $pattern, $name, @diag) = @_;
    my $ctx = context();

    my $bool;
    if (defined($thing)) {
        $bool = "$thing" =~ $pattern;
        unshift @diag => (
            "Value: $thing",
            "Does not match: $pattern"
        ) unless $bool;
    }
    else {
        $bool = 0;
        unshift @diag => "Got an undefined value.";
    }

    return $ctx->pass_and_release($name) if $bool;
    return $ctx->fail_and_release($name, @diag);
}

sub unlike($$;$@) {
    my ($thing, $pattern, $name, @diag) = @_;
    my $ctx = context();

    my $bool;
    if (defined($thing)) {
        $bool = "$thing" !~ $pattern;
        unshift @diag => (
            "Unexpected pattern match (it should not match)",
            "Value:   $thing",
            "Matches: $pattern"
        ) unless $bool;
    }
    else {
        $bool = 0;
        unshift @diag => "Got an undefined value.";
    }

    return $ctx->pass_and_release($name) if $bool;
    return $ctx->fail_and_release($name, @diag);
}

sub is_deeply($$;$@) {
    my ($got, $want, $name, @diag) = @_;
    my $ctx = context();

    no warnings 'once';
    require Data::Dumper;

    # Otherwise numbers might be unquoted
    local $Data::Dumper::Useperl  = 1;

    local $Data::Dumper::Sortkeys = 1;
    local $Data::Dumper::Deparse  = 1;
    local $Data::Dumper::Freezer  = 'XXX';
    local *UNIVERSAL::XXX         = sub {
        my ($thing) = @_;
        if (ref($thing)) {
            $thing = {%$thing}  if "$thing" =~ m/=HASH/;
            $thing = [@$thing]  if "$thing" =~ m/=ARRAY/;
            $thing = \"$$thing" if "$thing" =~ m/=SCALAR/;
        }
        $_[0] = $thing;
    };

    my $g = Data::Dumper::Dumper($got);
    my $w = Data::Dumper::Dumper($want);

    my $bool = $g eq $w;

    return $ctx->pass_and_release($name) if $bool;
    return $ctx->fail_and_release($name, $g, $w, @diag);
}

sub diag {
    my $ctx = context();
    $ctx->diag(join '', @_);
    $ctx->release;
}

sub note {
    my $ctx = context();
    $ctx->note(join '', @_);
    $ctx->release;
}

sub skip_all {
    my ($reason) = @_;
    my $ctx = context();
    $ctx->plan(0, SKIP => $reason);
    $ctx->release if $ctx;
}

sub todo {
    my ($reason, $sub) = @_;
    my $ctx = context();

    # This code is mostly copied from Test2::Todo in the Test2-Suite
    # distribution.
    my $hub    = test2_stack->top;
    my $filter = $hub->pre_filter(
        sub {
            my ($active_hub, $event) = @_;
            if ($active_hub == $hub) {
                $event->set_todo($reason) if $event->can('set_todo');
                $event->add_amnesty({tag => 'TODO', details => $reason});
            }
            else {
                $event->add_amnesty({tag => 'TODO', details => $reason, inherited => 1});
            }
            return $event;
        },
        inherit => 1,
        todo    => $reason,
    );
    $sub->();
    $hub->pre_unfilter($filter);

    $ctx->release if $ctx;
}

sub plan {
    my ($max) = @_;
    my $ctx = context();
    $ctx->plan($max);
    $ctx->release;
}

sub done_testing {
    my $ctx = context();
    $ctx->done_testing;
    $ctx->release;
}

sub warnings(&) {
    my $code = shift;
    my @warnings;
    local $SIG{__WARN__} = sub { push @warnings => @_ };
    $code->();
    return \@warnings;
}

sub exception(&) {
    my $code = shift;
    local ($@, $!, $SIG{__DIE__});
    my $ok = eval { $code->(); 1 };
    my $error = $@ || 'SQUASHED ERROR';
    return $ok ? undef : $error;
}

sub tests {
    my ($name, $code) = @_;
    my $ctx = context();

    my $be = caller->can('before_each');

    $be->($name) if $be;

    my $bool = run_subtest($name, $code, 1);

    $ctx->release;

    return $bool;
}

sub capture(&) {
    my $code = shift;

    my ($err, $out) = ("", "");

    my $handles = test2_stack->top->format->handles;
    my ($ok, $e);
    {
        my ($out_fh, $err_fh);

        ($ok, $e) = try {
          # Scalar refs as filehandles were added in 5.8.
          if ($] ge "5.008") {
            open($out_fh, '>', \$out) or die "Failed to open a temporary STDOUT: $!";
            open($err_fh, '>', \$err) or die "Failed to open a temporary STDERR: $!";
          }
          # Emulate scalar ref filehandles with a tie.
          else {
            $out_fh = Test::Builder::IO::Scalar->new(\$out) or die "Failed to open a temporary STDOUT";
            $err_fh = Test::Builder::IO::Scalar->new(\$err) or die "Failed to open a temporary STDERR";
          }

            test2_stack->top->format->set_handles([$out_fh, $err_fh, $out_fh]);

            $code->();
        };
    }
    test2_stack->top->format->set_handles($handles);

    die $e unless $ok;

    $err =~ s/ $/_/mg;
    $out =~ s/ $/_/mg;

    return {
        STDOUT => $out,
        STDERR => $err,
    };
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Tools::Tiny - Tiny set of tools for unfortunate souls who cannot use
L<Test2::Suite>.

=head1 DESCRIPTION

You should really look at L<Test2::Suite>. This package is some very basic
essential tools implemented using L<Test2>. This exists only so that L<Test2>
and other tools required by L<Test2::Suite> can be tested. This is the package
L<Test2> uses to test itself.

=head1 USE Test2::Suite INSTEAD

Use L<Test2::Suite> if at all possible.

=head1 EXPORTS

=over 4

=item ok($bool, $name)

=item ok($bool, $name, @diag)

Run a simple assertion.

=item is($got, $want, $name)

=item is($got, $want, $name, @diag)

Assert that 2 strings are the same.

=item isnt($got, $do_not_want, $name)

=item isnt($got, $do_not_want, $name, @diag)

Assert that 2 strings are not the same.

=item like($got, $regex, $name)

=item like($got, $regex, $name, @diag)

Check that the input string matches the regex.

=item unlike($got, $regex, $name)

=item unlike($got, $regex, $name, @diag)

Check that the input string does not match the regex.

=item is_deeply($got, $want, $name)

=item is_deeply($got, $want, $name, @diag)

Check 2 data structures. Please note that this is a I<DUMB> implementation that
compares the output of L<Data::Dumper> against both structures.

=item diag($msg)

Issue a diagnostics message to STDERR.

=item note($msg)

Issue a diagnostics message to STDOUT.

=item skip_all($reason)

Skip all tests.

=item todo $reason => sub { ... }

Run a block in TODO mode.

=item plan($count)

Set the plan.

=item done_testing()

Set the plan to the current test count.

=item $warnings = warnings { ... }

Capture an arrayref of warnings from the block.

=item $exception = exception { ... }

Capture an exception.

=item tests $name => sub { ... }

Run a subtest.

=item $output = capture { ... }

Capture STDOUT and STDERR output.

Result looks like this:

    {
        STDOUT => "...",
        STDERR => "...",
    }

=back

=head1 SOURCE

The source code repository for Test2 can be found at
F<http://github.com/Test-More/test-more/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2020 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
