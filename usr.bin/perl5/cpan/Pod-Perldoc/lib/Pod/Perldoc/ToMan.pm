require 5.006;
package Pod::Perldoc::ToMan;
use strict;
use warnings;
use parent qw(Pod::Perldoc::BaseTo);

use vars qw($VERSION);
$VERSION = '3.28';

use File::Spec::Functions qw(catfile);
use Pod::Man 2.18;
# This class is unlike ToText.pm et al, because we're NOT paging thru
# the output in our particular format -- we make the output and
# then we run nroff (or whatever) on it, and then page thru the
# (plaintext) output of THAT!

sub SUCCESS () { 1 }
sub FAILED  () { 0 }

sub is_pageable        { 1 }
sub write_with_binmode { 0 }
sub output_extension   { 'txt' }

sub __filter_nroff  { shift->_perldoc_elem('__filter_nroff'  , @_) }
sub __nroffer       { shift->_perldoc_elem('__nroffer'       , @_) }
sub __bindir        { shift->_perldoc_elem('__bindir'        , @_) }
sub __pod2man       { shift->_perldoc_elem('__pod2man'       , @_) }
sub __output_file   { shift->_perldoc_elem('__output_file'   , @_) }

sub center          { shift->_perldoc_elem('center'         , @_) }
sub date            { shift->_perldoc_elem('date'           , @_) }
sub fixed           { shift->_perldoc_elem('fixed'          , @_) }
sub fixedbold       { shift->_perldoc_elem('fixedbold'      , @_) }
sub fixeditalic     { shift->_perldoc_elem('fixeditalic'    , @_) }
sub fixedbolditalic { shift->_perldoc_elem('fixedbolditalic', @_) }
sub name            { shift->_perldoc_elem('name'           , @_) }
sub quotes          { shift->_perldoc_elem('quotes'         , @_) }
sub release         { shift->_perldoc_elem('release'        , @_) }
sub section         { shift->_perldoc_elem('section'        , @_) }

sub new {
	my( $either ) = shift;
	my $self = bless {}, ref($either) || $either;
	$self->init( @_ );
	return $self;
	}

sub init {
	my( $self, @args ) = @_;

	unless( $self->__nroffer ) {
		my $roffer = $self->_find_roffer( $self->_roffer_candidates );
		$self->debug( "Using $roffer\n" );
		$self->__nroffer( $roffer );
		}
    else {
	    $self->debug( "__nroffer is " . $self->__nroffer() . "\n" );
        }

	$self->_check_nroffer;
	}

sub _roffer_candidates {
	my( $self ) = @_;

	if( $self->is_openbsd || $self->is_freebsd || $self->is_bitrig ) { qw( mandoc groff nroff ) }
	else                    { qw( groff nroff mandoc ) }
	}

sub _find_roffer {
	my( $self, @candidates ) = @_;

	my @found = ();
	foreach my $candidate ( @candidates ) {
		push @found, $self->_find_executable_in_path( $candidate );
		}

	return wantarray ? @found : $found[0];
	}

sub _check_nroffer {
	return 1;
	# where is it in the PATH?

	# is it executable?

	# what is its real name?

	# what is its version?

	# does it support the flags we need?

	# is it good enough for us?
	}

sub _get_stty { `stty -a` }

sub _get_columns_from_stty {
	my $output = $_[0]->_get_stty;

	if(    $output =~ /\bcolumns\s+(\d+)/ )    { return $1 }
	elsif( $output =~ /;\s*(\d+)\s+columns;/ ) { return $1 }
	else                                       { return  0 }
	}

sub _get_columns_from_manwidth {
	my( $self ) = @_;

	return 0 unless defined $ENV{MANWIDTH};

	unless( $ENV{MANWIDTH} =~ m/\A\d+\z/ ) {
		$self->warn( "Ignoring non-numeric MANWIDTH ($ENV{MANWIDTH})\n" );
		return 0;
		}

	if( $ENV{MANWIDTH} == 0 ) {
		$self->warn( "Ignoring MANWIDTH of 0. Really? Why even run the program? :)\n" );
		return 0;
		}

	if( $ENV{MANWIDTH} =~ m/\A(\d+)\z/ ) { return $1 }

	return 0;
	}

sub _get_default_width {
	73
	}

sub _get_columns {
	$_[0]->_get_columns_from_manwidth ||
	$_[0]->_get_columns_from_stty     ||
	$_[0]->_get_default_width;
	}

sub _get_podman_switches {
	my( $self ) = @_;

	my @switches = map { $_, $self->{$_} } grep !m/^_/s, keys %$self;

    # There needs to be a cleaner way to handle setting
    # the UTF-8 flag, but for now, comment out this
    # line because it often does the wrong thing.
    #
    # See RT #77465
    #
    #push @switches, 'utf8' => 1;

	$self->debug( "Pod::Man switches are [@switches]\n" );

	return @switches;
	}

sub _parse_with_pod_man {
	my( $self, $file ) = @_;

	#->output_fh and ->output_string from Pod::Simple aren't
	# working, apparently, so there's this ugly hack:
	local *STDOUT;
	open STDOUT, '>', $self->{_text_ref};
	my $parser = Pod::Man->new( $self->_get_podman_switches );
	$self->debug( "Parsing $file\n" );
	$parser->parse_from_file( $file );
	$self->debug( "Done parsing $file\n" );
	close STDOUT;

	$self->die( "No output from Pod::Man!\n" )
		unless length $self->{_text_ref};

	$self->_save_pod_man_output if $self->debugging;

	return SUCCESS;
	}

sub _save_pod_man_output {
	my( $self, $fh ) = @_;

	$fh = do {
		my $file = "podman.out.$$.txt";
		$self->debug( "Writing $file with Pod::Man output\n" );
		open my $fh2, '>', $file;
		$fh2;
		} unless $fh;

	print { $fh } ${ $self->{_text_ref} };
	}

sub _have_groff_with_utf8 {
	my( $self ) = @_;

	return 0 unless $self->_is_groff;
	my $roffer = $self->__nroffer;

	my $minimum_groff_version = '1.20.1';

	my $version_string = `$roffer -v`;
	my( $version ) = $version_string =~ /\(?groff\)? version (\d+\.\d+(?:\.\d+)?)/;
	$self->debug( "Found groff $version\n" );

	# is a string comparison good enough?
	if( $version lt $minimum_groff_version ) {
		$self->warn(
			"You have an old groff." .
			" Update to version $minimum_groff_version for good Unicode support.\n" .
			"If you don't upgrade, wide characters may come out oddly.\n"
			 );
		}

	$version ge $minimum_groff_version;
	}

sub _have_mandoc_with_utf8 {
	my( $self ) = @_;

       $self->_is_mandoc and not system 'mandoc -Tlocale -V > /dev/null 2>&1';
	}

sub _collect_nroff_switches {
	my( $self ) = shift;

    my @render_switches = ('-man', $self->_get_device_switches);

	# Thanks to Brendan O'Dea for contributing the following block
	if( $self->_is_roff and -t STDOUT and my ($cols) = $self->_get_columns ) {
		my $c = $cols * 39 / 40;
		$cols = $c > $cols - 2 ? $c : $cols -2;
		push @render_switches, '-rLL=' . (int $c) . 'n' if $cols > 80;
		}

	# I hear persistent reports that adding a -c switch to $render
	# solves many people's problems.  But I also hear that some mans
	# don't have a -c switch, so that unconditionally adding it here
	# would presumably be a Bad Thing   -- sburke@cpan.org
    push @render_switches, '-c' if( $self->_is_roff and $self->is_cygwin );

	return @render_switches;
	}

sub _get_device_switches {
	my( $self ) = @_;

	   if( $self->_is_nroff  )             { qw()              }
	elsif( $self->_have_groff_with_utf8 )  { qw(-Kutf8 -Tutf8) }
	elsif( $self->_is_ebcdic )             { qw(-Tcp1047)      }
	elsif( $self->_have_mandoc_with_utf8 ) { qw(-Tlocale)      }
	elsif( $self->_is_mandoc )             { qw()              }
	else                                   { qw(-Tlatin1)      }
	}

sub _is_roff {
	my( $self ) = @_;

	$self->_is_nroff or $self->_is_groff;
	}

sub _is_nroff {
	my( $self ) = @_;

	$self->__nroffer =~ /\bnroff\b/;
	}

sub _is_groff {
	my( $self ) = @_;

	$self->__nroffer =~ /\bgroff\b/;
	}

sub _is_mandoc {
	my ( $self ) = @_;

	$self->__nroffer =~ /\bmandoc\b/;
	}

sub _is_ebcdic {
	my( $self ) = @_;

	return 0;
	}
	
sub _filter_through_nroff {
	my( $self ) = shift;
	$self->debug( "Filtering through " . $self->__nroffer() . "\n" );

    # Maybe someone set rendering switches as part of the opt_n value
    # Deal with that here.

    my ($render, $switches) = $self->__nroffer() =~ /\A([\/a-zA-Z0-9_\.-]+)\b(.+)?\z/;

    $self->die("no nroffer!?") unless $render;
    my @render_switches = $self->_collect_nroff_switches;

    if ( $switches ) {
        # Eliminate whitespace 
        $switches =~ s/\s//g;

        # Then separate the switches with a zero-width positive 
        # lookahead on the dash.
        #
        # See:
        # http://www.effectiveperlprogramming.com/blog/1411
        # for a good discussion of this technique

        push @render_switches, split(/(?=-)/, $switches);
        }

	$self->debug( "render is $render\n" );
	$self->debug( "render options are @render_switches\n" );

	require Symbol;
	require IPC::Open3;
	require IO::Handle;

	my $pid = IPC::Open3::open3(
		my $writer,
		my $reader,
		my $err = Symbol::gensym(),
		$render,
		@render_switches
		);

	$reader->autoflush(1);

	use IO::Select;
	my $selector = IO::Select->new( $reader );

	$self->debug( "Writing to pipe to $render\n" );

	my $offset = 0;
	my $chunk_size = 4096;
	my $length = length( ${ $self->{_text_ref} } );
	my $chunks = $length / $chunk_size;
	my $done;
	my $buffer;
	while( $offset <= $length ) {
		$self->debug( "Writing chunk $chunks\n" ); $chunks++;
		syswrite $writer, ${ $self->{_text_ref} }, $chunk_size, $offset
			or $self->die( $! );
		$offset += $chunk_size;
		$self->debug( "Checking read\n" );
		READ: {
			last READ unless $selector->can_read( 0.01 );
			$self->debug( "Reading\n" );
			my $bytes = sysread $reader, $buffer, 4096;
			$self->debug( "Read $bytes bytes\n" );
			$done .= $buffer;
			$self->debug( sprintf "Output is %d bytes\n",
				length $done
				);
			next READ;
			}
		}
	close $writer;
	$self->debug( "Done writing\n" );

	# read any leftovers
	$done .= do { local $/; <$reader> };
	$self->debug( sprintf "Done reading. Output is %d bytes\n",
		length $done
		);

	if( $? ) {
		$self->warn( "Error from pipe to $render!\n" );
		$self->debug( 'Error: ' . do { local $/; <$err> } );
		}


	close $reader;
	if( my $err = $? ) {
		$self->debug(
			"Nonzero exit ($?) while running `$render @render_switches`.\n" .
			"Falling back to Pod::Perldoc::ToPod\n"
			);
		return $self->_fallback_to_pod( @_ );
		}

	$self->debug( "Output:\n----\n$done\n----\n" );

	${ $self->{_text_ref} } = $done;

	return length ${ $self->{_text_ref} } ? SUCCESS : FAILED;
	}

sub parse_from_file {
	my( $self, $file, $outfh) = @_;

	# We have a pipeline of filters each affecting the reference
	# in $self->{_text_ref}
	$self->{_text_ref} = \my $output;

	$self->_parse_with_pod_man( $file );
	# so far, nroff is an external command so we ensure it worked
	my $result = $self->_filter_through_nroff;
	return $self->_fallback_to_pod( @_ ) unless $result == SUCCESS;

	$self->_post_nroff_processing;

	print { $outfh } $output or
		$self->die( "Can't print to $$self{__output_file}: $!" );

	return;
	}

sub _fallback_to_pod {
	my( $self, @args ) = @_;
	$self->warn( "Falling back to Pod because there was a problem!\n" );
	require Pod::Perldoc::ToPod;
	return  Pod::Perldoc::ToPod->new->parse_from_file(@_);
	}

# maybe there's a user setting we should check?
sub _get_tab_width { 4 }

sub _expand_tabs {
	my( $self ) = @_;

	my $tab_width = ' ' x $self->_get_tab_width;

	${ $self->{_text_ref} } =~ s/\t/$tab_width/g;
	}

sub _post_nroff_processing {
	my( $self ) = @_;

	if( $self->is_hpux ) {
	    $self->debug( "On HP-UX, I'm going to expand tabs for you\n" );
		# this used to be a pipe to `col -x` for HP-UX
		$self->_expand_tabs;
		}

	if( $self->{'__filter_nroff'} ) {
		$self->debug( "filter_nroff is set, so filtering\n" );
		$self->_remove_nroff_header;
		$self->_remove_nroff_footer;
		}
	else {
		$self->debug( "filter_nroff is not set, so not filtering\n" );
		}

	$self->_handle_unicode;

	return 1;
	}

# I don't think this does anything since there aren't two consecutive
# newlines in the Pod::Man output
sub _remove_nroff_header {
	my( $self ) = @_;
	$self->debug( "_remove_nroff_header is still a stub!\n" );
	return 1;

#  my @data = split /\n{2,}/, shift;
#  shift @data while @data and $data[0] !~ /\S/; # Go to header
#  shift @data if @data and $data[0] =~ /Contributed\s+Perl/; # Skip header
	}

# I don't think this does anything since there aren't two consecutive
# newlines in the Pod::Man output
sub _remove_nroff_footer {
	my( $self ) = @_;
	$self->debug( "_remove_nroff_footer is still a stub!\n" );
	return 1;
	${ $self->{_text_ref} } =~ s/\n\n+.*\w.*\Z//m;

#  my @data = split /\n{2,}/, shift;
#  pop @data if @data and $data[-1] =~ /^\w/; # Skip footer, like
        # 28/Jan/99 perl 5.005, patch 53 1
	}

sub _unicode_already_handled {
	my( $self ) = @_;

	$self->_have_groff_with_utf8 ||
	1  # so, we don't have a case that needs _handle_unicode
	;
	}

sub _handle_unicode {
# this is the job of preconv
# we don't need this with groff 1.20 and later.
	my( $self ) = @_;

	return 1 if $self->_unicode_already_handled;

	require Encode;

	# it's UTF-8 here, but we need character data
	my $text = Encode::decode( 'UTF-8', ${ $self->{_text_ref} } ) ;

# http://www.mail-archive.com/groff@gnu.org/msg01378.html
# http://linux.die.net/man/7/groff_char
# http://www.gnu.org/software/groff/manual/html_node/Using-Symbols.html
# http://lists.gnu.org/archive/html/groff/2011-05/msg00007.html
# http://www.simplicidade.org/notes/archives/2009/05/fixing_the_pod.html
# http://lists.freebsd.org/pipermail/freebsd-questions/2011-July/232239.html
	$text =~ s/(\P{ASCII})/
		sprintf '\\[u%04X]', ord $1
	     /eg;

	# should we encode?
	${ $self->{_text_ref} } = $text;
	}

1;

__END__

=head1 NAME

Pod::Perldoc::ToMan - let Perldoc render Pod as man pages

=head1 SYNOPSIS

  perldoc -o man Some::Modulename

=head1 DESCRIPTION

This is a "plug-in" class that allows Perldoc to use
Pod::Man and C<groff> for reading Pod pages.

The following options are supported:  center, date, fixed, fixedbold,
fixeditalic, fixedbolditalic, quotes, release, section

(Those options are explained in L<Pod::Man>.)

For example:

  perldoc -o man -w center:Pod Some::Modulename

=head1 CAVEAT

This module may change to use a different pod-to-nroff formatter class
in the future, and this may change what options are supported.

=head1 SEE ALSO

L<Pod::Man>, L<Pod::Perldoc>, L<Pod::Perldoc::ToNroff>

=head1 COPYRIGHT AND DISCLAIMERS

Copyright (c) 2011 brian d foy. All rights reserved.

Copyright (c) 2002,3,4 Sean M. Burke.  All rights reserved.

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

