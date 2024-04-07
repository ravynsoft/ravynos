package Amiga::ARexx;

use 5.016000;
use strict;
use warnings;
use Carp;

use Exporter 'import';

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration       use Amiga::Classes::ARexx ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
DoRexx
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
);

our $VERSION = '0.06';

require XSLoader;
XSLoader::load('Amiga::ARexx', $VERSION);

sub new
{
    my $class = shift;
    my $self = bless {}, $class;
    return $self->__init(@_);
}

sub __init
{
    my $self = shift;
    my %params = @_;
    my @tags = ();

    if(exists $params{'HostName'})
    {
        $self->{'__hostname'} = $params{'HostName'};
    } else { croak "HostName required";}

    $self->{'__host'} = Amiga::ARexx::Host_init($self->{'__hostname'});
    if (defined $self->{'__host'} && $self->{'__host'} != 0)
    {
    }
    else
    {
        croak "Unabel to initialise Arexx Host";
    }
    return $self;
}

sub wait
{
	my $self = shift;
	my %params = @_;
	my $timeout = -1;
	if ((exists $params{'TimeOut'}) && (defined $params{'TimeOut'}))
	{
		$timeout = $params{'TimeOut'};
		$timeout += 0; # force number
	}
	Amiga::ARexx::Host_wait($self->{'__host'},$timeout);

}

sub signal
{
	my $self = shift;
	return Amiga::ARexx::Host_signal($self->{'__host'});
}

sub getmsg
{
    my $self = shift;
    my $msg;
    my $msgobj;

    if(defined $self->{'__host'})
    {
    	$msg = Amiga::ARexx::Host_getmsg($self->{'__host'});
    	if($msg)
    	{
    	    $msgobj = Amiga::ARexx::Msg->new('Message' => $msg);
    	}
    }
    return $msgobj;
}

sub DESTROY
{
    my $self = shift;
    if(exists $self->{'__host'} && defined $self->{'__host'})
    {
        Amiga::ARexx::Host_delete($self->{'__host'});
        delete $self->{'__host'};
    }
}

sub DoRexx($$)
{
    my ($port,$command) = @_;
    my $rc = 0;
    my $rc2 = 0;
    my $result = Amiga::ARexx::_DoRexx($port,$command,$rc,$rc2);
    return ($rc,$rc2,$result);
}

package Amiga::ARexx::Msg;

use strict;
use warnings;
use Carp;

sub new
{
    my $class = shift;
    my $self = bless {}, $class;
    return $self->__init(@_);
}

sub __init
{
    my $self = shift;
    my %params = @_;

    if(exists $params{'Message'})
    {
        $self->{'__msg'} = $params{'Message'};
    } else { croak "Message required";}

    $self->{'__message'} = Amiga::ARexx::Msg_argstr($self->{'__msg'});
    return $self;
}

sub message
{
    my $self = shift;
    return $self->{'__message'};
}

sub reply($$$$)
{
    my ($self,$rc,$rc2,$result) = @_;
    if(exists $self->{'__msg'} && defined $self->{'__msg'})
    {
        Amiga::ARexx::Msg_reply($self->{'__msg'},$rc,$rc2,$result);
    }
}

sub setvar($$$)
{
    my ($self,$varname,$value) = @_;
    if(exists $self->{'__msg'} && defined $self->{'__msg'})
    {
        Amiga::ARexx::Msg_setvar($self->{'__msg'},$varname,$value);
    }
}

sub getvar($$)
{
    my ($self,$varname) = @_;
    if(exists $self->{'__msg'} && defined $self->{'__msg'})
    {
    	return Amiga::ARexx::Msg_getvar($self->{'__msg'},$varname);
    }
}

sub DESTROY
{
    my $self = shift;
    if(exists $self->{'__msg'} && defined $self->{'__msg'})
    {
        Amiga::ARexx::Msg_delete($self->{'__msg'});
        delete $self->{'__msg'};
    }
}

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Amiga::ARexx - Perl extension for ARexx support

=head1 ABSTRACT

This a  perl class / module to enable you to use  ARexx  with
your perlscript. Creating a function host or executing scripts in other hosts.
The API is loosley modeled on the python arexx module supplied by with AmigaOS4.1

=head1 SYNOPSIS

    # Create a new host

    use Amiga::ARexx;
    my $host = Amiga::ARexx->new('HostName' => "PERLREXX" );

    # Wait for and process rexxcommands

    my $alive = 1;

    while ($alive)
    {
        $host->wait();
        my $msg = $host->getmsg();
        while($msg)
        {
            my $rc = 0;
            my $rc2 = 0;
            my $result = "";

            print $msg->message . "\n";
            given($msg->message)
            {
                when ("QUIT")
                {
                    $alive = 0;
                    $result = "quitting!";
                }
                default {
                    $rc = 10;
                    $rc2 = 22;
                }
            }
            $msg->reply($rc,$rc2,$result);

            $msg = $host->getmsg();
        }

    }

    # Send a command to a host

    my $port = "SOMEHOST";
    my $command = "SOMECOMMAND";
    my ($rc,$rc2,$result) = Amiga::ARexx->DoRexx($port,$command);



=head1 DESCRIPTION

The interface to the arexx.class in entirely encapsulated within the perl class, there
is no need to access the low level methods directly and they are not exported by default.

=head1 Amiga::ARexx METHODS

=head2 new

    my $host = Amiga::ARexx->new( HostName => "PERLREXX");


Create an ARexx host for your script / program.

=head3 HostName

The HostName for the hosts command port. This is madatory, the program will fail if not
provided.


=head2 wait

	$host->wait('TimeOut' => $timeoutinusecs );

Wait for a message to arive at the port.

=head3 TimeOut

optional time out in microseconds.


=head2 getmsg

    $msg = $host->getmsg();


Fetch an ARexx message from the host port. Returns an objrct of class Amiga::ARexx::Msg

=head2 signal

    $signal = $host->signal()

Retrieve the signal mask for the host port for use with Amiga::Exec Wait()

=head2 DoRexx

    ($rc,$rc2,$result) = DoRexx("desthost","commandstring");

Send the "commandstring" to host "desthost" for execution. Commandstring might be a specific command or scriptname.

=head1 Amiga::ARexx::Msg METHODS

=head2 message

	$m = $msg->message();

Retrieve the message "command" as a string;


=head2 reply

	$msg->reply($rc,$rc2,$result)

Reply the message returning the results of any command. Set $rc = 0 for success and $result  to the result string if appropriate.

Set $rc to non zero for error and $rc2 for an additional error code if appropriate.

=head2 setvar

	$msg->setvar($varname,$value)

Set a variable in the language context sending this message.

=head2 getvar

    $value = $msg->getvar($varname)

Get the value of a variable in the language context sending this message.


=head2 EXPORT

None by default.

=head2 Exportable constants

None

=head1 AUTHOR

Andy Broad <andy@broad.ology.org.uk>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2013 by Andy Broad.

=cut



