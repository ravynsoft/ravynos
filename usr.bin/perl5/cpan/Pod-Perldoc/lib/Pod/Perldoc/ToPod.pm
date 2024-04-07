package Pod::Perldoc::ToPod;
use strict;
use warnings;
use parent qw(Pod::Perldoc::BaseTo);

use vars qw($VERSION);
$VERSION = '3.28';

sub is_pageable        { 1 }
sub write_with_binmode { 0 }
sub output_extension   { 'pod' }

sub new { return bless {}, ref($_[0]) || $_[0] }

sub parse_from_file {
  my( $self, $in, $outfh ) = @_;

  open(IN, "<", $in) or $self->die( "Can't read-open $in: $!\nAborting" );

  my $cut_mode = 1;

  # A hack for finding things between =foo and =cut, inclusive
  local $_;
  while (<IN>) {
    if(  m/^=(\w+)/s ) {
      if($cut_mode = ($1 eq 'cut')) {
        print $outfh "\n=cut\n\n";
         # Pass thru the =cut line with some harmless
         #  (and occasionally helpful) padding
      }
    }
    next if $cut_mode;
    print $outfh $_ or $self->die( "Can't print to $outfh: $!" );
  }

  close IN or $self->die( "Can't close $in: $!" );
  return;
}

1;
__END__

=head1 NAME

Pod::Perldoc::ToPod - let Perldoc render Pod as ... Pod!

=head1 SYNOPSIS

  perldoc -opod Some::Modulename

(That's currently the same as the following:)

  perldoc -u Some::Modulename

=head1 DESCRIPTION

This is a "plug-in" class that allows Perldoc to display Pod source as
itself!  Pretty Zen, huh?

Currently this class works by just filtering out the non-Pod stuff from
a given input file.

=head1 SEE ALSO

L<Pod::Perldoc>

=head1 COPYRIGHT AND DISCLAIMERS

Copyright (c) 2002 Sean M. Burke.  All rights reserved.

This library is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

This program is distributed in the hope that it will be useful, but
without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.

=head1 AUTHOR

Current maintainer: Mark Allen C<< <mallencpan.org> >>

Past contributions from:
brian d foy C<< <bdfoy@cpan.org> >>
Adriano R. Ferreira C<< <ferreira@cpan.org> >>,
Sean M. Burke C<< <sburke@cpan.org> >>

=cut

