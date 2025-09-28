

require 5;
package Pod::Simple::TextContent;
use strict;
use Carp ();
use Pod::Simple ();
use vars qw( @ISA $VERSION );
$VERSION = '3.43';
@ISA = ('Pod::Simple');

sub new {
  my $self = shift;
  my $new = $self->SUPER::new(@_);
  $new->{'output_fh'} ||= *STDOUT{IO};
  $new->nix_X_codes(1);
  return $new;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

sub _handle_element_start {
  print {$_[0]{'output_fh'}} "\n"  unless $_[1] =~ m/^[A-Z]$/s;
  return;
}

sub _handle_text {
  $_[1] =~ s/$Pod::Simple::shy//g;
  $_[1] =~ s/$Pod::Simple::nbsp/ /g;
  print {$_[0]{'output_fh'}} $_[1];
  return;
}

sub _handle_element_end {
  print {$_[0]{'output_fh'}} "\n"  unless $_[1] =~ m/^[A-Z]$/s;
  return;
}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
1;


__END__

=head1 NAME

Pod::Simple::TextContent -- get the text content of Pod

=head1 SYNOPSIS

 TODO

  perl -MPod::Simple::TextContent -e \
   "exit Pod::Simple::TextContent->filter(shift)->any_errata_seen" \
   thingy.pod

=head1 DESCRIPTION

This class is that parses Pod and dumps just the text content.  It is
mainly meant for use by the Pod::Simple test suite, but you may find
some other use for it.

This is a subclass of L<Pod::Simple> and inherits all its methods.

=head1 SEE ALSO

L<Pod::Simple>, L<Pod::Simple::Text>, L<Pod::Spell>

=head1 SUPPORT

Questions or discussion about POD and Pod::Simple should be sent to the
pod-people@perl.org mail list. Send an empty email to
pod-people-subscribe@perl.org to subscribe.

This module is managed in an open GitHub repository,
L<https://github.com/perl-pod/pod-simple/>. Feel free to fork and contribute, or
to clone L<git://github.com/perl-pod/pod-simple.git> and send patches!

Patches against Pod::Simple are welcome. Please send bug reports to
<bug-pod-simple@rt.cpan.org>.

=head1 COPYRIGHT AND DISCLAIMERS

Copyright (c) 2002 Sean M. Burke.

This library is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

This program is distributed in the hope that it will be useful, but
without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.

=head1 AUTHOR

Pod::Simple was created by Sean M. Burke <sburke@cpan.org>.
But don't bother him, he's retired.

Pod::Simple is maintained by:

=over

=item * Allison Randal C<allison@perl.org>

=item * Hans Dieter Pearcey C<hdp@cpan.org>

=item * David E. Wheeler C<dwheeler@cpan.org>

=back

=cut
