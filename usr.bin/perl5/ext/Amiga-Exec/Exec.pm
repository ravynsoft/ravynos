package Amiga::Exec;

use 5.016000;
use strict;
use warnings;
use Carp;

use Exporter 'import';

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration       use Amiga::Exec ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
Wait
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
);

our $VERSION = '0.04';

require XSLoader;
XSLoader::load('Amiga::Exec', $VERSION);


sub Wait
{
    my %params = @_;
    my $signalmask = 0;
    my $timeout = 0;

    if(exists $params{'SignalMask'})
    {
    	$signalmask = $params{'SignalMask'};
    }
    if(exists $params{'TimeOut'})
    {
    	$timeout = $params{'TimeOut'};
    }

    my $result = Amiga::Exec::_Wait($signalmask,$timeout);
    return $result;
}



# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Amiga::Exec - Perl extension for low level amiga support

=head1 ABSTRACT

This a  perl class / module to enables you to use various low level Amiga features such as waiting on an Exec signal

=head1 SYNOPSIS

    # Wait for signla

    use Amiga::Exec;
    my $result = Amiga::ARexx->Wait('SignalMask' => $signalmask,
                                    'TimeOut' => $timeoutinusecs);

=head1 DESCRIPTION

The interface to  Exec in entirely encapsulated within the perl class, there
is no need to access the low level methods directly and they are not exported by default.

=head1 Amiga::ARexx METHODS

=head2 Wait

 $signals = Amiga::Exec->Wait('SignalMask' => $signalmask,
                              'TimeOut' => $timeoutinusecs );

Wait on a signal set with optional timeout. The result ($signals) should be checked to
determine which signal was raised. It will be 0 for timeout.

=head3 Signal

The signal Exec signal mask

=head3 TimeOut

optional time out in microseconds.

=head2 EXPORT

None by default.

=head2 Exportable constants

None

=head1 AUTHOR

Andy Broad <andy@broad.ology.org.uk>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2013 by Andy Broad.


=cut



