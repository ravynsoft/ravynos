package open;
use warnings;

our $VERSION = '1.13';

require 5.008001; # for PerlIO::get_layers()

my $locale_encoding;

sub _get_encname {
    return ($1, Encode::resolve_alias($1)) if $_[0] =~ /^:?encoding\((.+)\)$/;
    return;
}

sub croak {
    require Carp; goto &Carp::croak;
}

sub _drop_oldenc {
    # If by the time we arrive here there already is at the top of the
    # perlio layer stack an encoding identical to what we would like
    # to push via this open pragma, we will pop away the old encoding
    # (+utf8) so that we can push ourselves in place (this is easier
    # than ignoring pushing ourselves because of the way how ${^OPEN}
    # works).  So we are looking for something like
    #
    #   stdio encoding(xxx) utf8
    #
    # in the existing layer stack, and in the new stack chunk for
    #
    #   :encoding(xxx)
    #
    # If we find a match, we pop the old stack (once, since
    # the utf8 is just a flag on the encoding layer)
    my ($h, @new) = @_;
    return unless @new >= 1 && $new[-1] =~ /^:encoding\(.+\)$/;
    my @old = PerlIO::get_layers($h);
    return unless @old >= 3 &&
	          $old[-1] eq 'utf8' &&
                  $old[-2] =~ /^encoding\(.+\)$/;
    require Encode;
    my ($loname, $lcname) = _get_encname($old[-2]);
    unless (defined $lcname) { # Should we trust get_layers()?
	croak("open: Unknown encoding '$loname'");
    }
    my ($voname, $vcname) = _get_encname($new[-1]);
    unless (defined $vcname) {
	croak("open: Unknown encoding '$voname'");
    }
    if ($lcname eq $vcname) {
	binmode($h, ":pop"); # utf8 is part of the encoding layer
    }
}

sub import {
    my ($class,@args) = @_;
    croak("open: needs explicit list of PerlIO layers") unless @args;
    my $std;
    my ($in,$out) = split(/\0/,(${^OPEN} || "\0"), -1);
    while (@args) {
	my $type = shift(@args);
	my $dscp;
	if ($type =~ /^:?(utf8|locale|encoding\(.+\))$/) {
	    $type = 'IO';
	    $dscp = ":$1";
	} elsif ($type eq ':std') {
	    $std = 1;
	    next;
	} else {
	    $dscp = shift(@args) || '';
	}
	my @val;
	foreach my $layer (split(/\s+/,$dscp)) {
            $layer =~ s/^://;
	    if ($layer eq 'locale') {
		require Encode;
		require encoding;
		$locale_encoding = encoding::_get_locale_encoding()
		    unless defined $locale_encoding;
		(warnings::warnif("layer", "Cannot figure out an encoding to use"), last)
		    unless defined $locale_encoding;
                $layer = "encoding($locale_encoding)";
		$std = 1;
	    } else {
		my $target = $layer;		# the layer name itself
		$target =~ s/^(\w+)\(.+\)$/$1/;	# strip parameters

		unless(PerlIO::Layer::->find($target,1)) {
		    warnings::warnif("layer", "Unknown PerlIO layer '$target'");
		}
	    }
	    push(@val,":$layer");
	    if ($layer =~ /^(crlf|raw)$/) {
		$^H{"open_$type"} = $layer;
	    }
	}
	if ($type eq 'IN') {
	    _drop_oldenc(*STDIN, @val) if $std;
	    $in  = join(' ', @val);
	}
	elsif ($type eq 'OUT') {
	    if ($std) {
		_drop_oldenc(*STDOUT, @val);
		_drop_oldenc(*STDERR, @val);
	    }
	    $out = join(' ', @val);
	}
	elsif ($type eq 'IO') {
	    if ($std) {
		_drop_oldenc(*STDIN, @val);
		_drop_oldenc(*STDOUT, @val);
		_drop_oldenc(*STDERR, @val);
	    }
	    $in = $out = join(' ', @val);
	}
	else {
	    croak "Unknown PerlIO layer class '$type' (need IN, OUT or IO)";
	}
    }
    ${^OPEN} = join("\0", $in, $out);
    if ($std) {
	if ($in) {
	    binmode STDIN, $in;
	}
	if ($out) {
	    binmode(STDOUT, $out);
	    binmode(STDERR, $out);
	}
    }
}

1;
__END__

=head1 NAME

open - perl pragma to set default PerlIO layers for input and output

=head1 SYNOPSIS

    use open IN  => ':crlf', OUT => ':raw';
    open my $in, '<', 'foo.txt' or die "open failed: $!";
    my $line = <$in>; # CRLF translated
    close $in;
    open my $out, '>', 'bar.txt' or die "open failed: $!";
    print $out $line; # no translation of bytes
    close $out;

    use open OUT => ':encoding(UTF-8)';
    use open IN  => ':encoding(iso-8859-7)';

    use open IO  => ':locale';

    # IO implicit only for :utf8, :encoding, :locale
    use open ':encoding(UTF-8)';
    use open ':encoding(iso-8859-7)';
    use open ':locale';

    # with :std, also affect global standard handles
    use open ':std', ':encoding(UTF-8)';
    use open ':std', OUT => ':encoding(cp1252)';
    use open ':std', IO => ':raw :encoding(UTF-16LE)';

=head1 DESCRIPTION

Full-fledged support for I/O layers is now implemented provided
Perl is configured to use PerlIO as its IO system (which has been the
default since 5.8, and the only supported configuration since 5.16).

The C<open> pragma serves as one of the interfaces to declare default
"layers" (previously known as "disciplines") for all I/O. Any open(),
readpipe() (aka qx//) and similar operators found within the
lexical scope of this pragma will use the declared defaults via the
L<C<${^OPEN}>|perlvar/${^OPEN}> variable.

Layers are specified with a leading colon by convention. You can
specify a stack of multiple layers as a space-separated string.
See L<PerlIO> for more information on the available layers.

With the C<IN> subpragma you can declare the default layers
of input streams, and with the C<OUT> subpragma you can declare
the default layers of output streams.  With the C<IO> subpragma
(may be omitted for C<:utf8>, C<:locale>, or C<:encoding>) you
can control both input and output streams simultaneously.

When open() is given an explicit list of layers (with the three-arg
syntax), they override the list declared using this pragma.  open() can
also be given a single colon (:) for a layer name, to override this pragma
and use the default as detailed in
L<PerlIO/Defaults and how to override them>.

To translate from and to an arbitrary text encoding, use the C<:encoding>
layer.  The matching of encoding names in C<:encoding> is loose: case does
not matter, and many encodings have several aliases.  See
L<Encode::Supported> for details and the list of supported locales.

If you want to set your encoding layers based on your
locale environment variables, you can use the C<:locale> pseudo-layer.
For example:

    $ENV{LANG} = 'ru_RU.KOI8-R';
    # the :locale will probe the locale environment variables like LANG
    use open OUT => ':locale';
    open(my $out, '>', 'koi8') or die "open failed: $!";
    print $out chr(0x430); # CYRILLIC SMALL LETTER A = KOI8-R 0xc1
    close $out;
    open(my $in, '<', 'koi8') or die "open failed: $!";
    printf "%#x\n", ord(<$in>); # this should print 0xc1
    close $in;

The logic of C<:locale> is described in full in
L<encoding/The C<:locale> sub-pragma>,
but in short it is first trying nl_langinfo(CODESET) and then
guessing from the LC_ALL and LANG locale environment variables.
C<:locale> also implicitly turns on C<:std>.

C<:std> is not a layer but an additional subpragma.  When specified in the
import list, it activates an additional functionality of pushing the
layers selected for input/output handles to the standard filehandles
(STDIN, STDOUT, STDERR).  If the new layers and existing layer stack both
end with an C<:encoding> layer, the existing C<:encoding> layer will also
be removed.

For example, if both input and out are chosen to be C<:encoding(UTF-8)>, a
C<:std> will mean that STDIN, STDOUT, and STDERR will also have
C<:encoding(UTF-8)> set.  On the other hand, if only output is chosen to
be in C<:encoding(koi8r)>, a C<:std> will cause only the STDOUT and STDERR
to be in C<koi8r>.

The effect of C<:std> is not lexical as it modifies the layer stack of the
global handles.  If you wish to apply only this global effect and not the
effect on handles that are opened in that scope, you can isolate the call
to this pragma in its own lexical scope.

    { use open ':std', IO => ':encoding(UTF-8)' }

Before Perl 5.34, C<:std> would only apply the first layer provided that is
either C<:utf8> or has a layer argument, e.g. C<:encoding(UTF-8)>. Since
Perl 5.34 it will apply the same layer stack it provides to C<${^OPEN}>.

=head1 IMPLEMENTATION DETAILS

There is a class method in C<PerlIO::Layer> C<find> which is
implemented as XS code.  It is called by C<import> to validate the
layers:

   PerlIO::Layer::->find("perlio")

The return value (if defined) is a Perl object, of class
C<PerlIO::Layer> which is created by the C code in F<perlio.c>.  As
yet there is nothing useful you can do with the object at the perl
level.

=head1 SEE ALSO

L<perlfunc/"binmode">, L<perlfunc/"open">, L<perlunicode>, L<PerlIO>,
L<encoding>

=cut
