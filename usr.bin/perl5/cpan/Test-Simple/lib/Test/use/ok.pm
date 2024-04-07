package Test::use::ok;
use 5.005;

our $VERSION = '1.302194';


__END__

=head1 NAME

Test::use::ok - Alternative to Test::More::use_ok

=head1 SYNOPSIS

    use ok 'Some::Module';

=head1 DESCRIPTION

According to the B<Test::More> documentation, it is recommended to run
C<use_ok()> inside a C<BEGIN> block, so functions are exported at
compile-time and prototypes are properly honored.

That is, instead of writing this:

    use_ok( 'Some::Module' );
    use_ok( 'Other::Module' );

One should write this:

    BEGIN { use_ok( 'Some::Module' ); }
    BEGIN { use_ok( 'Other::Module' ); }

However, people often either forget to add C<BEGIN>, or mistakenly group
C<use_ok> with other tests in a single C<BEGIN> block, which can create subtle
differences in execution order.

With this module, simply change all C<use_ok> in test scripts to C<use ok>,
and they will be executed at C<BEGIN> time.  The explicit space after C<use>
makes it clear that this is a single compile-time action.

=head1 SEE ALSO

L<Test::More>

=head1 MAINTAINER

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=encoding utf8

=head1 CC0 1.0 Universal

To the extent possible under law, 唐鳳 has waived all copyright and related
or neighboring rights to L<Test-use-ok>.

This work is published from Taiwan.

L<http://creativecommons.org/publicdomain/zero/1.0>

=cut
