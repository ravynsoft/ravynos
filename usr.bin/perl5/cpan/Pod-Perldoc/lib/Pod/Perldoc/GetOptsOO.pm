package Pod::Perldoc::GetOptsOO;
use strict;

use vars qw($VERSION);
$VERSION = '3.28';

BEGIN { # Make a DEBUG constant ASAP
  *DEBUG = defined( &Pod::Perldoc::DEBUG )
   ? \&Pod::Perldoc::DEBUG
   : sub(){10};
}


sub getopts {
  my($target, $args, $truth) = @_;

  $args ||= \@ARGV;

  $target->aside(
    "Starting switch processing.  Scanning arguments [@$args]\n"
  ) if $target->can('aside');

  return unless @$args;

  $truth = 1 unless @_ > 2;

  DEBUG > 3 and print "   Truth is $truth\n";


  my $error_count = 0;

  while( @$args  and  ($_ = $args->[0]) =~ m/^-(.)(.*)/s ) {
    my($first,$rest) = ($1,$2);
    if ($_ eq '--') {	# early exit if "--"
      shift @$args;
      last;
    }
    if ($first eq '-' and $rest) {      # GNU style long param names
      ($first, $rest) = split '=', $rest, 2;
    }
    my $method = "opt_${first}_with";
    if( $target->can($method) ) {  # it's argumental
      if($rest eq '') {   # like -f bar
        shift @$args;
        $target->warn( "Option $first needs a following argument!\n" ) unless @$args;
        $rest = shift @$args;
      } else {            # like -fbar  (== -f bar)
        shift @$args;
      }

      DEBUG > 3 and print " $method => $rest\n";
      $target->$method( $rest );

    # Otherwise, it's not argumental...
    } else {

      if( $target->can( $method = "opt_$first" ) ) {
        DEBUG > 3 and print " $method is true ($truth)\n";
        $target->$method( $truth );

      # Otherwise it's an unknown option...

      } elsif( $target->can('handle_unknown_option') ) {
        DEBUG > 3
         and print " calling handle_unknown_option('$first')\n";

        $error_count += (
          $target->handle_unknown_option( $first ) || 0
        );

      } else {
        ++$error_count;
        $target->warn( "Unknown option: $first\n" );
      }

      if($rest eq '') {   # like -f
        shift @$args
      } else {            # like -fbar  (== -f -bar )
        DEBUG > 2 and print "   Setting args->[0] to \"-$rest\"\n";
        $args->[0] = "-$rest";
      }
    }
  }


  $target->aside(
    "Ending switch processing.  Args are [@$args] with $error_count errors.\n"
  ) if $target->can('aside');

  $error_count == 0;
}

1;

__END__

=head1 NAME

Pod::Perldoc::GetOptsOO - Customized option parser for Pod::Perldoc

=head1 SYNOPSIS

    use Pod::Perldoc::GetOptsOO ();

    Pod::Perldoc::GetOptsOO::getopts( $obj, \@args, $truth )
       or die "wrong usage";


=head1 DESCRIPTION

Implements a customized option parser used for
L<Pod::Perldoc>.

Rather like Getopt::Std's getopts:

=over

=item Call Pod::Perldoc::GetOptsOO::getopts($object, \@ARGV, $truth)

=item Given -n, if there's a opt_n_with, it'll call $object->opt_n_with( ARGUMENT )
   (e.g., "-n foo" => $object->opt_n_with('foo').  Ditto "-nfoo")

=item Otherwise (given -n) if there's an opt_n, we'll call it $object->opt_n($truth)
   (Truth defaults to 1)

=item Otherwise we try calling $object->handle_unknown_option('n')
   (and we increment the error count by the return value of it)

=item If there's no handle_unknown_option, then we just warn, and then increment
   the error counter

=back

The return value of Pod::Perldoc::GetOptsOO::getopts is true if no errors,
otherwise it's false.

=head1 SEE ALSO

L<Pod::Perldoc>

=head1 COPYRIGHT AND DISCLAIMERS

Copyright (c) 2002-2007 Sean M. Burke.

This library is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

This program is distributed in the hope that it will be useful, but
without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.

=head1 AUTHOR

Current maintainer: Mark Allen C<< <mallen@cpan.org> >>

Past contributions from:
brian d foy C<< <bdfoy@cpan.org> >>
Adriano R. Ferreira C<< <ferreira@cpan.org> >>,
Sean M. Burke C<< <sburke@cpan.org> >>

=cut
