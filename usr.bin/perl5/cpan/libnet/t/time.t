#!perl

use 5.008001;

use strict;
use warnings;

use Test::More;

BEGIN {
    if (!eval { require Socket }) {
        plan skip_all => "no Socket";
    }
    elsif (ord('A') == 193 && !eval { require Convert::EBCDIC }) {
        plan skip_all => "EBCDIC but no Convert::EBCDIC";
    }
    else {
        plan tests => 12;
    }

    $INC{'IO/Socket.pm'} = 1;
    $INC{'IO/Select.pm'} = 1;
    $INC{'IO/Socket/INET.pm'} = 1;
}

# cannot use(), otherwise it will use IO::Socket and IO::Select
eval{ require Net::Time; };
ok( !$@, 'should be able to require() Net::Time safely' );
ok( exists $INC{'Net/Time.pm'}, 'should be able to use Net::Time' );

# force the socket to fail
make_fail('IO::Socket::INET', 'new');
my $badsock = Net::Time::_socket('foo', 1, 'bar', 'baz');
is( $badsock, undef, '_socket() should fail if Socket creation fails' );

# if socket is created with protocol UDP (default), it will send a newline
my $sock = Net::Time::_socket('foo', 2, 'bar'); 
ok( $sock->isa('IO::Socket::INET'), 'should be an IO::Socket::INET object' );
is( $sock->{sent}, "\n", 'should send \n with UDP protocol set' );
is( $sock->{timeout}, 120, 'timeout should default to 120' );

# now try it with a custom timeout and a different protocol
$sock = Net::Time::_socket('foo', 3, 'bar', 'tcp', 11);
ok( $sock->isa('IO::Socket::INET'), 'should be an IO::Socket::INET object' );
is( $sock->{sent}, undef, '_socket() should send nothing unless UDP protocol' );
is( $sock->{PeerAddr}, 'bar', '_socket() should set PeerAddr in socket' );
is( $sock->{timeout}, 11, '_socket() should respect custom timeout value' );

# inet_daytime
# check for correct args (daytime, 13)
IO::Socket::INET::set_message('z');
is( Net::Time::inet_daytime('bob'), 'z', 'inet_daytime() should receive data' );

# magic numbers defined in Net::Time
my $offset = $^O eq 'MacOS' ?
        (4 * 31536000) : (70 * 31536000 + 17 * 86400);

# check for correct args (time, 13)
# pretend it is only six seconds since the offset, create a fake message
# inet_time
IO::Socket::INET::set_message(pack("N", $offset + 6));
is( Net::Time::inet_time('foo'), 6, 
        'inet_time() should calculate time since offset for time()' );


my %fail;

sub make_fail {
        my ($pack, $func, $num) = @_;
        $num = 1 unless defined $num;

        $fail{$pack}{$func} = $num;
}

package IO::Socket::INET;

$fail{'IO::Socket::INET'} = {
        new     => 0,
        'send'  => 0,
};

sub new {
        my $class = shift;
        return if $fail{$class}{new} and $fail{$class}{new}--;
        bless( { @_ }, $class );
}

sub send {
        my $self = shift;
        my $class = ref($self);
        return if $fail{$class}{'send'} and $fail{$class}{'send'}--;
        $self->{sent} .= shift;
}

my $msg;
sub set_message {
        if (ref($_[0])) {
                $_[0]->{msg} = $_[1];
        } else {
                $msg = shift;
        }
}

sub do_recv  {
        my ($len, $msg) = @_[1,2];
        $_[0] .= substr($msg, 0, $len);
}

sub recv {
        my ($self, $buf, $length, $flags) = @_;
        my $message = exists $self->{msg} ?
                $self->{msg} : $msg;

        if (defined($message)) {
                do_recv($_[1], $length, $message);
        }
        1;
}

package IO::Select;

sub new {
        my $class = shift;
        return if defined $fail{$class}{new} and $fail{$class}{new}--;
        bless({sock => shift}, $class);
}

sub can_read {
        my ($self, $timeout) = @_;
        my $class = ref($self);
        return if defined $fail{$class}{can_read} and $fail{class}{can_read}--;
        $self->{sock}{timeout} = $timeout;
        1;
}

1;
