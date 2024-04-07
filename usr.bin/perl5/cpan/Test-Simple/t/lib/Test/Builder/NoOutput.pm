package Test::Builder::NoOutput;

use strict;
use warnings;

use Symbol qw(gensym);
use base qw(Test::Builder);


=head1 NAME

Test::Builder::NoOutput - A subclass of Test::Builder which prints nothing

=head1 SYNOPSIS

    use Test::Builder::NoOutput;

    my $tb = Test::Builder::NoOutput->new;

    ...test as normal...

    my $output = $tb->read;

=head1 DESCRIPTION

This is a subclass of Test::Builder which traps all its output.
It is mostly useful for testing Test::Builder.

=head3 read

    my $all_output = $tb->read;
    my $output     = $tb->read($stream);

Returns all the output (including failure and todo output) collected
so far.  It is destructive, each call to read clears the output
buffer.

If $stream is given it will return just the output from that stream.
$stream's are...

    out         output()
    err         failure_output()
    todo        todo_output()
    all         all outputs

Defaults to 'all'.

=cut

my $Test = __PACKAGE__->new;

sub create {
    my $class = shift;
    my $self = $class->SUPER::create(@_);

    require Test::Builder::Formatter;
    $self->{Stack}->top->format(Test::Builder::Formatter->new);

    my %outputs = (
        all  => '',
        out  => '',
        err  => '',
        todo => '',
    );
    $self->{_outputs} = \%outputs;

    my($out, $err, $todo) = map { gensym() } 1..3;
    tie *$out,  "Test::Builder::NoOutput::Tee", \$outputs{all}, \$outputs{out};
    tie *$err,  "Test::Builder::NoOutput::Tee", \$outputs{all}, \$outputs{err};
    tie *$todo, "Test::Builder::NoOutput::Tee", \$outputs{all}, \$outputs{todo};

    $self->output($out);
    $self->failure_output($err);
    $self->todo_output($todo);

    return $self;
}


sub read {
    my $self = shift;
    my $stream = @_ ? shift : 'all';

    my $out = $self->{_outputs}{$stream};

    $self->{_outputs}{$stream} = '';

    # Clear all the streams if 'all' is read.
    if( $stream eq 'all' ) {
        my @keys = keys %{$self->{_outputs}};
        $self->{_outputs}{$_} = '' for @keys;
    }

    return $out;
}


package Test::Builder::NoOutput::Tee;

# A cheap implementation of IO::Tee.

sub TIEHANDLE {
    my($class, @refs) = @_;

    my @fhs;
    for my $ref (@refs) {
        my $fh = Test::Builder->_new_fh($ref);
        push @fhs, $fh;
    }

    my $self = [@fhs];
    return bless $self, $class;
}

sub PRINT {
    my $self = shift;

    print $_ @_ for @$self;
}

sub PRINTF {
    my $self   = shift;
    my $format = shift;

    printf $_ @_ for @$self;
}

1;
