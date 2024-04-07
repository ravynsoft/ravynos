
require 5;
package Pod::Simple::Text;
use strict;
use Carp ();
use Pod::Simple::Methody ();
use Pod::Simple ();
use vars qw( @ISA $VERSION $FREAKYMODE);
$VERSION = '3.43';
@ISA = ('Pod::Simple::Methody');
BEGIN { *DEBUG = defined(&Pod::Simple::DEBUG)
          ? \&Pod::Simple::DEBUG
          : sub() {0}
      }

use Text::Wrap 98.112902 ();
$Text::Wrap::huge = 'overflow';

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

sub new {
  my $self = shift;
  my $new = $self->SUPER::new(@_);
  $new->{'output_fh'} ||= *STDOUT{IO};
  $new->accept_target_as_text(qw( text plaintext plain ));
  $new->nix_X_codes(1);
  $new->nbsp_for_S(1);
  $new->{'Thispara'} = '';
  $new->{'Indent'} = 0;
  $new->{'Indentstring'} = '   ';
  return $new;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

sub handle_text {  $_[0]{'Thispara'} .= $_[1] }

sub start_Para  {  $_[0]{'Thispara'} = '' }
sub start_head1 {  $_[0]{'Thispara'} = '' }
sub start_head2 {  $_[0]{'Thispara'} = '' }
sub start_head3 {  $_[0]{'Thispara'} = '' }
sub start_head4 {  $_[0]{'Thispara'} = '' }

sub start_Verbatim    { $_[0]{'Thispara'} = ''   }
sub start_item_bullet { $_[0]{'Thispara'} = $FREAKYMODE ? '' : '* ' }
sub start_item_number { $_[0]{'Thispara'} = $FREAKYMODE ? '' : "$_[1]{'number'}. "  }
sub start_item_text   { $_[0]{'Thispara'} = ''   }

sub start_over_bullet  { ++$_[0]{'Indent'} }
sub start_over_number  { ++$_[0]{'Indent'} }
sub start_over_text    { ++$_[0]{'Indent'} }
sub start_over_block   { ++$_[0]{'Indent'} }

sub   end_over_bullet  { --$_[0]{'Indent'} }
sub   end_over_number  { --$_[0]{'Indent'} }
sub   end_over_text    { --$_[0]{'Indent'} }
sub   end_over_block   { --$_[0]{'Indent'} }


# . . . . . Now the actual formatters:

sub end_head1       { $_[0]->emit_par(-4) }
sub end_head2       { $_[0]->emit_par(-3) }
sub end_head3       { $_[0]->emit_par(-2) }
sub end_head4       { $_[0]->emit_par(-1) }
sub end_Para        { $_[0]->emit_par( 0) }
sub end_item_bullet { $_[0]->emit_par( 0) }
sub end_item_number { $_[0]->emit_par( 0) }
sub end_item_text   { $_[0]->emit_par(-2) }
sub start_L         { $_[0]{'Link'} = $_[1] if $_[1]->{type} eq 'url' }
sub end_L           {
    if (my $link = delete $_[0]{'Link'}) {
        # Append the URL to the output unless it's already present.
        $_[0]{'Thispara'} .= " <$link->{to}>"
            unless $_[0]{'Thispara'} =~ /\b\Q$link->{to}/;
    }
}

sub emit_par {
  my($self, $tweak_indent) = splice(@_,0,2);
  my $indent = ' ' x ( 2 * $self->{'Indent'} + 4 + ($tweak_indent||0) );
   # Yes, 'STRING' x NEGATIVE gives '', same as 'STRING' x 0

  $self->{'Thispara'} =~ s/$Pod::Simple::shy//g;
  my $out = Text::Wrap::wrap($indent, $indent, $self->{'Thispara'} .= "\n");
  $out =~ s/$Pod::Simple::nbsp/ /g;
  print {$self->{'output_fh'}} $out, "\n";
  $self->{'Thispara'} = '';
  
  return;
}

# . . . . . . . . . . And then off by its lonesome:

sub end_Verbatim  {
  my $self = shift;
  $self->{'Thispara'} =~ s/$Pod::Simple::nbsp/ /g;
  $self->{'Thispara'} =~ s/$Pod::Simple::shy//g;

  my $i = ' ' x ( 2 * $self->{'Indent'} + 4);
  #my $i = ' ' x (4 + $self->{'Indent'});
  
  $self->{'Thispara'} =~ s/^/$i/mg;
  
  print { $self->{'output_fh'} }   '', 
    $self->{'Thispara'},
    "\n\n"
  ;
  $self->{'Thispara'} = '';
  return;
}

#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
1;


__END__

=head1 NAME

Pod::Simple::Text -- format Pod as plaintext

=head1 SYNOPSIS

  perl -MPod::Simple::Text -e \
   "exit Pod::Simple::Text->filter(shift)->any_errata_seen" \
   thingy.pod

=head1 DESCRIPTION

This class is a formatter that takes Pod and renders it as
wrapped plaintext.

Its wrapping is done by L<Text::Wrap>, so you can change
C<$Text::Wrap::columns> as you like.

This is a subclass of L<Pod::Simple> and inherits all its methods.

=head1 SEE ALSO

L<Pod::Simple>, L<Pod::Simple::TextContent>, L<Pod::Text>

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
