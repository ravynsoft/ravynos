package IO::c55Capture;

use IO::Handle;

=head1 Name

t/lib/IO::c55Capture - a wafer-thin test support package

=head1 Why!?

Compatibility with 5.5.3 and no external dependencies.

=head1 Usage

Works with a global filehandle:

    # set a spool to write to
    tie local *STDOUT, 'IO::c55Capture';
    ...
    # clear and retrieve buffer list
    my @spooled = tied(*STDOUT)->dump();

Or, a lexical (and autocreated) filehandle:

    my $capture = IO::c55Capture->new_handle;
    ...
    my @output = tied($$capture)->dump;

Note the '$$' dereference.

=cut

# XXX actually returns an IO::Handle :-/
sub new_handle {
    my $class  = shift;
    my $handle = IO::Handle->new;
    tie $$handle, $class;
    return ($handle);
}

sub TIEHANDLE {
    return bless [], __PACKAGE__;
}

sub PRINT {
    my $self = shift;

    push @$self, @_;
}

sub PRINTF {
    my $self = shift;
    push @$self, sprintf(@_);
}

sub dump {
    my $self = shift;
    my @got  = @$self;
    @$self = ();
    return @got;
}

package util;

use IO::File;

# mostly stolen from Module::Build MBTest.pm

{    # backwards compatible temp filename recipe adapted from perlfaq
    my $tmp_count = 0;
    my $tmp_base_name = sprintf( "%d-%d", $$, time() );

    sub temp_file_name {
        sprintf( "%s-%04d", $tmp_base_name, ++$tmp_count );
    }
}
########################################################################

sub save_handle {
    my ( $handle, $subr ) = @_;
    my $outfile = temp_file_name();

    local *SAVEOUT;
    open SAVEOUT, ">&" . fileno($handle)
      or die "Can't save output handle: $!";
    open $handle, "> $outfile" or die "Can't create $outfile: $!";

    eval { $subr->() };
    my $err = $@;
    open $handle, ">&SAVEOUT" or die "Can't restore output: $!";

    my $ret = slurp($outfile);
    1 while unlink $outfile;
    $err and die $err;
    return $ret;
}

sub stdout_of { save_handle( \*STDOUT, @_ ) }
sub stderr_of { save_handle( \*STDERR, @_ ) }

sub stdout_stderr_of {
    my $subr = shift;
    my ( $stdout, $stderr );
    $stdout = stdout_of(
        sub {
            $stderr = stderr_of($subr);
        }
    );
    return ( $stdout, $stderr );
}

sub slurp {
    my $fh = IO::File->new( $_[0] ) or die "Can't open $_[0]: $!";
    local $/;
    return scalar <$fh>;
}

1;

# vim:ts=4:sw=4:et:sta
