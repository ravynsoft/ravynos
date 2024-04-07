package ok;
our $VERSION = '1.302194';

use strict;
use Test::More ();

sub import {
    shift;

    if (@_) {
        goto &Test::More::pass if $_[0] eq 'ok';
        goto &Test::More::use_ok;
    }

    # No argument list - croak as if we are prototyped like use_ok()
    my (undef, $file, $line) = caller();
    ($file =~ /^\(eval/) or die "Not enough arguments for 'use ok' at $file line $line\n";
}


__END__

=encoding UTF-8

=head1 NAME

ok - Alternative to Test::More::use_ok

=head1 SYNOPSIS

    use ok 'Some::Module';

=head1 DESCRIPTION

With this module, simply change all C<use_ok> in test scripts to C<use ok>,
and they will be executed at C<BEGIN> time.

Please see L<Test::use::ok> for the full description.

=head1 CC0 1.0 Universal

To the extent possible under law, 唐鳳 has waived all copyright and related
or neighboring rights to L<Test-use-ok>.

This work is published from Taiwan.

L<http://creativecommons.org/publicdomain/zero/1.0>

=cut
