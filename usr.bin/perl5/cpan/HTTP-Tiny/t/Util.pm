package Util;

use strict;
use warnings;

use IO::File qw(SEEK_SET SEEK_END);
use IO::Dir;

BEGIN {
    our @EXPORT_OK = qw(
        rewind
        tmpfile
        dir_list
        slurp
        parse_case
        hashify
        sort_headers
        connect_args
        clear_socket_source
        set_socket_source
        monkey_patch
        $CRLF
        $LF
    );

    require Exporter;
    *import = \&Exporter::import;
}

our $CRLF = "\x0D\x0A";
our $LF   = "\x0A";

sub rewind(*) {
    seek($_[0], 0, SEEK_SET)
      || die(qq/Couldn't rewind file handle: '$!'/);
}

sub tmpfile {
    my $fh = IO::File->new_tmpfile
      || die(qq/Couldn't create a new temporary file: '$!'/);

    binmode($fh)
      || die(qq/Couldn't binmode temporary file handle: '$!'/);

    if (@_) {
        print({$fh} @_)
          || die(qq/Couldn't write to temporary file handle: '$!'/);

        seek($fh, 0, SEEK_SET)
          || die(qq/Couldn't rewind temporary file handle: '$!'/);
    }

    return $fh;
}

sub dir_list {
    my ($dir, $filter) = @_;
    $filter ||= qr/./;
    my $d = IO::Dir->new($dir)
        or return;
    return map { "$dir/$_" } sort grep { /$filter/ } grep { /^[^.]/ } $d->read;
}

sub slurp (*) {
    my ($fh) = @_;

    seek($fh, 0, SEEK_END)
      || die(qq/Couldn't navigate to EOF on file handle: '$!'/);

    my $exp = tell($fh);

    rewind($fh);

    binmode($fh)
      || die(qq/Couldn't binmode file handle: '$!'/);

    my $buf = do { local $/; <$fh> };
    my $got = length $buf;

    ($exp == $got)
      || die(qq[I/O read mismatch (expexted: $exp got: $got)]);

    return $buf;
}

sub parse_case {
    my ($case) = @_;
    my %args;
    my $key = '';
    my %seen;
    for my $line ( split "\n", $case ) {
        chomp $line;
        if ( substr($line,0,1) eq q{ } ) {
            $line =~ s/^\s+//;
            push @{$args{$key}}, $line;
        }
        else {
            $key = $line;
            $seen{$key}++;
        }
    }
    for my $k (keys %seen) {
        $args{$k}=undef unless exists $args{$k};
    }
    return \%args;
}

sub hashify {
    my ($lines) = @_;
    return unless $lines;
    my %hash;
    for my $line ( @$lines ) {
        my ($k,$v) = ($line =~ m{^([^:]+): (.*)$}g);
        $hash{$k} = [ $hash{$k} ] if exists $hash{$k} && ref $hash{$k} ne 'ARRAY';
        if ( ref($hash{$k}) eq 'ARRAY' ) {
            push @{$hash{$k}}, $v;
        }
        else {
            $hash{$k} = $v;
        }
    }
    return %hash;
}

sub sort_headers {
    my ($text) = shift;
    my @lines = split /$CRLF/, $text;
    my $request = shift(@lines) || '';
    my @headers;
    while (my $line = shift @lines) {
        last unless length $line;
        push @headers, $line;
    }
    @headers = sort @headers;
    return join($CRLF, $request, @headers, '', @lines);
}

{
    my (@req_fh, @res_fh, $monkey_host, $monkey_port);

    sub clear_socket_source {
        @req_fh = ();
        @res_fh = ();
    }

    sub set_socket_source {
        my ($req_fh, $res_fh) = @_;
        push @req_fh, $req_fh;
        push @res_fh, $res_fh;
    }

    sub connect_args { return ($monkey_host, $monkey_port) }

    sub monkey_patch {
        no warnings qw/redefine once/;
        *HTTP::Tiny::Handle::can_read = sub {1};
        *HTTP::Tiny::Handle::can_write = sub {1};
        *HTTP::Tiny::Handle::connect = sub {
            my ($self, $scheme, $host, $port, $peer) = @_;
            $self->{host}   = $monkey_host = $host;
            $self->{port}   = $monkey_port = $port;
            $self->{peer}   = $peer;
            $self->{scheme} = $scheme;
            $self->{fh} = shift @req_fh;
            $self->{pid} = $$;
            $self->{tid} = HTTP::Tiny::Handle::_get_tid();
            return $self;
        };
        my $original_write_request = \&HTTP::Tiny::Handle::write_request;
        *HTTP::Tiny::Handle::write_request = sub {
            my ($self, $request) = @_;
            $original_write_request->($self, $request);
            $self->{fh} = shift @res_fh;
        };
        *HTTP::Tiny::Handle::close = sub { 1 }; # don't close our temps
        *HTTP::Tiny::Handle::connected = sub { 1 };

        # don't try to proxy in mock-mode
        delete $ENV{$_} for map { $_, uc($_) } qw/http_proxy https_proxy all_proxy/;
    }
}

1;


# vim: et ts=4 sts=4 sw=4:
